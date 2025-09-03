#include "vtkMRMLLayerDMCameraSynchronizer.h"

#include "vtkObjectEventObserver.h"
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLSliceNode.h"

#include <vtkObjectFactory.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkMatrix4x4.h>
#include <array>

class CameraSynchronizeStrategy
{
public:
  explicit CameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera)
    : m_camera(camera)
  {
  }
  virtual ~CameraSynchronizeStrategy() = default;
  virtual void UpdateCamera() = 0;

protected:
  vtkSmartPointer<vtkCamera> m_camera;
  vtkNew<vtkObjectEventObserver> m_eventObserver;
};

class DefaultCameraSynchronizeStrategy : public CameraSynchronizeStrategy
{
public:
  explicit DefaultCameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera, vtkRenderer* renderer)
    : CameraSynchronizeStrategy(camera)
    , m_renderer(renderer)
  {
    m_eventObserver->SetUpdateCallback(
      [this](vtkObject* object)
      {
        if (object == m_observedCamera)
        {
          UpdateCamera();
        }
        if (object == m_renderer)
        {
          ObserveActiveCamera();
        }
      });

    m_eventObserver->UpdateObserver(nullptr, m_renderer, vtkCommand::ActiveCameraEvent);
    ObserveActiveCamera();
  }

  void UpdateCamera() override
  {
    if (m_observedCamera)
    {
      m_camera->DeepCopy(m_observedCamera);
      m_camera->Modified();
    }
  }

private:
  void ObserveActiveCamera() { SetObservedCamera(m_renderer ? m_renderer->GetActiveCamera() : nullptr); }

  void SetObservedCamera(vtkCamera* camera)
  {
    if (m_observedCamera == camera)
    {
      return;
    }

    m_eventObserver->UpdateObserver(m_observedCamera, camera);
    m_observedCamera = camera;
    UpdateCamera();
  }

  vtkWeakPointer<vtkRenderer> m_renderer;
  vtkWeakPointer<vtkCamera> m_observedCamera;
};

class SliceViewCameraSynchronizeStrategy : public CameraSynchronizeStrategy
{
public:
  explicit SliceViewCameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera, vtkMRMLSliceNode* sliceNode)
    : CameraSynchronizeStrategy(camera)
    , m_sliceNode{ sliceNode }
  {
    m_eventObserver->SetUpdateCallback(
      [this](vtkObject* object)
      {
        if (object == m_sliceNode)
        {
          UpdateCamera();
        }
      });
    m_eventObserver->UpdateObserver(nullptr, m_sliceNode);
  }

  void UpdateCamera() override
  {
    if (!m_sliceNode)
    {
      return;
    }

    // Compute view center
    vtkMatrix4x4* xyToRas = m_sliceNode->GetXYToRAS();
    std::array<double, 4> viewCenterXY = { 0.5 * m_sliceNode->GetDimensions()[0], 0.5 * m_sliceNode->GetDimensions()[1], 0.0, 1.0 };
    std::array<double, 4> viewCenterRAS = {};
    xyToRas->MultiplyPoint(viewCenterXY.data(), viewCenterRAS.data());

    // Current slice RAS coordinate is invalid (Slice was probably just created and not already displayed).
    // Avoid propagating NaN.
    if (std::isnan(viewCenterRAS[0]))
    {
      return;
    }

    // Parallel projection and scale
    m_camera->ParallelProjectionOn();
    m_camera->SetParallelScale(0.5 * m_sliceNode->GetFieldOfView()[1]);

    // Set focal point
    m_camera->SetFocalPoint(viewCenterRAS.data());

    // View directions
    vtkMatrix4x4* sliceToRAS = m_sliceNode->GetSliceToRAS();

    std::array<double, 3> vRight = { sliceToRAS->GetElement(0, 0), sliceToRAS->GetElement(1, 0), sliceToRAS->GetElement(2, 0) };

    std::array<double, 3> vUp = { sliceToRAS->GetElement(0, 1), sliceToRAS->GetElement(1, 1), sliceToRAS->GetElement(2, 1) };
    m_camera->SetViewUp(vUp.data());

    // Position
    double d = m_camera->GetDistance();
    std::array<double, 3> normal{};
    vtkMath::Cross(vRight.data(), vUp.data(), normal.data());
    double position[3] = { viewCenterRAS[0] + normal[0] * d, viewCenterRAS[1] + normal[1] * d, viewCenterRAS[2] + normal[2] * d };
    m_camera->SetPosition(position);
  }

private:
  vtkWeakPointer<vtkMRMLSliceNode> m_sliceNode;
};

vtkStandardNewMacro(vtkMRMLLayerDMCameraSynchronizer);

void vtkMRMLLayerDMCameraSynchronizer::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (m_viewNode == viewNode)
  {
    return;
  }

  m_viewNode = viewNode;
  UpdateStrategy();
}

void vtkMRMLLayerDMCameraSynchronizer::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (m_defaultCamera == camera)
  {
    return;
  }
  m_defaultCamera = camera;
  UpdateStrategy();
}

void vtkMRMLLayerDMCameraSynchronizer::SetRenderer(vtkRenderer* renderer)
{
  if (m_renderer == renderer)
  {
    return;
  }
  m_renderer = renderer;
  UpdateStrategy();
}

vtkMRMLLayerDMCameraSynchronizer::vtkMRMLLayerDMCameraSynchronizer()
  : m_defaultCamera{ nullptr }
  , m_renderer{ nullptr }
  , m_viewNode{ nullptr }
  , m_syncStrategy{ nullptr }
{
}

vtkMRMLLayerDMCameraSynchronizer::~vtkMRMLLayerDMCameraSynchronizer() = default;

void vtkMRMLLayerDMCameraSynchronizer::UpdateStrategy()
{
  if (!m_defaultCamera || !m_renderer)
  {
    m_syncStrategy = nullptr;
    return;
  }

  if (auto sliceNode = vtkMRMLSliceNode::SafeDownCast(m_viewNode))
  {
    m_syncStrategy = std::make_unique<SliceViewCameraSynchronizeStrategy>(m_defaultCamera, sliceNode);
  }
  else
  {
    m_syncStrategy = std::make_unique<DefaultCameraSynchronizeStrategy>(m_defaultCamera, m_renderer);
  }
  m_syncStrategy->UpdateCamera();
}
