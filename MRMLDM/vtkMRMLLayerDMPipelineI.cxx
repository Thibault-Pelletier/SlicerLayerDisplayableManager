#include "vtkMRMLLayerDMPipelineI.h"

#include "vtkMRMLLayerDMPipelineManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLAbstractWidget.h"

#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineI);

void vtkMRMLLayerDMPipelineI::UpdatePipeline() {}

void vtkMRMLLayerDMPipelineI::OnRendererRemoved(vtkRenderer* renderer) {}

void vtkMRMLLayerDMPipelineI::OnRendererAdded(vtkRenderer* renderer) {}

void vtkMRMLLayerDMPipelineI::SetDisplayNode(vtkMRMLNode* displayNode)
{
  UpdateObserver(m_displayNode, displayNode);
  m_displayNode = displayNode;
}

void vtkMRMLLayerDMPipelineI::ResetDisplay()
{
  if (m_isResetDisplayBlocked || !m_viewNode)
  {
    return;
  }

  UpdatePipeline();
}

void vtkMRMLLayerDMPipelineI::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  UpdateObserver(m_viewNode, viewNode);
  m_viewNode = viewNode;
}

void vtkMRMLLayerDMPipelineI::SetRenderer(vtkRenderer* renderer)
{
  if (m_renderer == renderer)
  {
    return;
  }

  OnRendererRemoved(m_renderer);
  m_renderer = renderer;
  OnRendererAdded(m_renderer);
  UpdatePipeline();
}

bool vtkMRMLLayerDMPipelineI::BlockResetDisplay(bool isBlocked)
{
  bool prev = m_isResetDisplayBlocked;
  m_isResetDisplayBlocked = isBlocked;
  return prev;
}

bool vtkMRMLLayerDMPipelineI::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  return false;
}

bool vtkMRMLLayerDMPipelineI::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  return false;
}

int vtkMRMLLayerDMPipelineI::GetMouseCursor() const
{
  return 0;
}

int vtkMRMLLayerDMPipelineI::GetWidgetState() const
{
  return vtkMRMLAbstractWidget::WidgetStateIdle;
}

void vtkMRMLLayerDMPipelineI::LoseFocus(vtkMRMLInteractionEventData* eventData) {}

void vtkMRMLLayerDMPipelineI::OnDefaultCameraModified(vtkCamera* camera) {}

bool vtkMRMLLayerDMPipelineI::UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event) const
{
  return m_obs->UpdateObserver(prevObj, obj, event);
}

bool vtkMRMLLayerDMPipelineI::UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events) const
{
  return m_obs->UpdateObserver(prevObj, obj, events);
}

unsigned int vtkMRMLLayerDMPipelineI::GetRenderLayer() const
{
  return 0;
}

vtkCamera* vtkMRMLLayerDMPipelineI::GetCamera() const
{
  return nullptr;
}

void vtkMRMLLayerDMPipelineI::SetScene(vtkMRMLScene* scene)
{
  m_scene = scene;
}

vtkMRMLLayerDMPipelineI* vtkMRMLLayerDMPipelineI::GetNodePipeline(vtkMRMLNode* node) const
{
  if (!m_pipelineManager)
  {
    return nullptr;
  }
  return m_pipelineManager->GetNodePipeline(node);
}

vtkMRMLAbstractViewNode* vtkMRMLLayerDMPipelineI::GetViewNode() const
{
  return m_viewNode;
}

vtkMRMLNode* vtkMRMLLayerDMPipelineI::GetDisplayNode() const
{
  return m_displayNode;
}

vtkRenderer* vtkMRMLLayerDMPipelineI::GetRenderer() const
{
  return m_renderer;
}

vtkMRMLScene* vtkMRMLLayerDMPipelineI::GetScene() const
{
  return m_scene;
}

void vtkMRMLLayerDMPipelineI::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData) {}

void vtkMRMLLayerDMPipelineI::RequestRender() const
{
  if (m_pipelineManager)
  {
    m_pipelineManager->RequestRender();
  }
}

void vtkMRMLLayerDMPipelineI::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  m_pipelineManager = pipelineManager;
}

vtkMRMLLayerDMPipelineI::vtkMRMLLayerDMPipelineI()
  : m_viewNode{ nullptr }
  , m_displayNode{ nullptr }
  , m_renderer{ nullptr }
  , m_isResetDisplayBlocked{ false }
  , m_obs(vtkSmartPointer<vtkObjectEventObserver>::New())
  , m_pipelineManager(nullptr)
{
  m_obs->SetUpdateCallback([this](vtkObject* obj, unsigned long eventId, void* callData) { OnUpdate(obj, eventId, callData); });
};
