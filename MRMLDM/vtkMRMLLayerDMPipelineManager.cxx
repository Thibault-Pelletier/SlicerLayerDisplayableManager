#include "vtkMRMLLayerDMPipelineManager.h"

#include "vtkMRMLLayerDMLayerManager.h"
#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkObjectEventObserver.h"
#include "vtkMRMLLayerDMPipelineI.h"
#include "vtkMRMLLayerDMCameraSynchronizer.h"
#include "vtkMRMLLayerDMInteractionLogic.h"

#include <vtkCallbackCommand.h>
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLScene.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineManager);

bool vtkMRMLLayerDMPipelineManager::CreatePipelineForNode(vtkMRMLNode* displayNode)
{
  // Early return if manager is not yet created
  if (!m_factory || !m_viewNode)
  {
    return false;
  }

  auto pipeline = m_factory->CreatePipeline(m_viewNode, displayNode);
  if (!pipeline)
  {
    return false;
  }

  pipeline->SetPipelineManager(this);
  pipeline->SetScene(m_scene);
  pipeline->SetViewNode(m_viewNode);
  pipeline->SetDisplayNode(displayNode);
  m_pipelineMap[displayNode] = pipeline;
  m_layerManager->AddPipeline(pipeline);
  m_interactionLogic->AddPipeline(pipeline);
  UpdatePipeline(pipeline);
  InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::ClearDisplayableNodes()
{
  m_pipelineMap.clear();
}

bool vtkMRMLLayerDMPipelineManager::AddNode(vtkMRMLNode* node)
{
  if (auto pipeline = GetNodePipeline(node))
  {
    return false;
  }

  return CreatePipelineForNode(node);
}

void vtkMRMLLayerDMPipelineManager::UpdateAllPipelines() const
{
  for (const auto& pipeline : m_pipelineMap)
  {
    UpdatePipeline(pipeline.second);
  }
}

bool vtkMRMLLayerDMPipelineManager::RemovePipeline(vtkMRMLNode* displayNode)
{
  auto pipeline = GetNodePipeline(displayNode);
  if (!pipeline)
  {
    return false;
  }

  m_layerManager->RemovePipeline(pipeline);
  m_interactionLogic->RemovePipeline(pipeline);
  m_pipelineMap.erase(displayNode);
  InvokeEvent(vtkCommand::ModifiedEvent);
  return true;
}

void vtkMRMLLayerDMPipelineManager::SetRenderWindow(vtkRenderWindow* renderWindow) const
{
  m_layerManager->SetRenderWindow(renderWindow);
}

void vtkMRMLLayerDMPipelineManager::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (m_viewNode == viewNode)
  {
    return;
  }

  m_viewNode = viewNode;
  m_cameraSync->SetViewNode(viewNode);
  m_interactionLogic->SetViewNode(viewNode);
  UpdateAllPipelines();
}

void vtkMRMLLayerDMPipelineManager::SetFactory(const vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>& factory)
{
  if (m_factory == factory)
  {
    return;
  }

  m_eventObs->UpdateObserver(m_factory, factory);
  m_factory = factory;
  UpdateFromScene();
}

int vtkMRMLLayerDMPipelineManager::GetMouseCursor() const
{
  auto lastFocused = m_interactionLogic->GetLastFocusedPipeline();
  return lastFocused ? lastFocused->GetMouseCursor() : VTK_CURSOR_DEFAULT;
}

bool vtkMRMLLayerDMPipelineManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) const
{
  return m_interactionLogic->CanProcessInteractionEvent(eventData, distance2);
}

void vtkMRMLLayerDMPipelineManager::LoseFocus(vtkMRMLInteractionEventData* eventData) const
{
  m_interactionLogic->LoseFocus(eventData);
}

void vtkMRMLLayerDMPipelineManager::LoseFocus() const
{
  m_interactionLogic->LoseFocus();
}

bool vtkMRMLLayerDMPipelineManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) const
{
  return m_interactionLogic->ProcessInteractionEvent(eventData);
}

bool vtkMRMLLayerDMPipelineManager::RemoveNode(vtkMRMLNode* node)
{
  return RemovePipeline(node);
}

void vtkMRMLLayerDMPipelineManager::ResetCameraClippingRange()
{
  if (m_isResettingClippingRange)
  {
    return;
  }

  m_isResettingClippingRange = true;
  m_layerManager->ResetCameraClippingRange();
  m_isResettingClippingRange = false;
}

void vtkMRMLLayerDMPipelineManager::RequestRender()
{
  ResetCameraClippingRange();
  m_requestRender();
}

void vtkMRMLLayerDMPipelineManager::OnDefaultCameraModified() const
{
  for (const auto& pipeline : m_pipelineMap)
  {
    pipeline.second->OnDefaultCameraModified(m_defaultCamera);
  }
}

vtkMRMLLayerDMPipelineManager::vtkMRMLLayerDMPipelineManager()
  : m_factory{ nullptr }
  , m_layerManager(vtkSmartPointer<vtkMRMLLayerDMLayerManager>::New())
  , m_cameraSync(vtkSmartPointer<vtkMRMLLayerDMCameraSynchronizer>::New())
  , m_interactionLogic(vtkSmartPointer<vtkMRMLLayerDMInteractionLogic>::New())
  , m_eventObs(vtkSmartPointer<vtkObjectEventObserver>::New())
  , m_defaultCamera(vtkSmartPointer<vtkCamera>::New())
  , m_viewNode{ nullptr }
  , m_scene{ nullptr }
  , m_pipelineMap{}
  , m_requestRender{ [] {} }
  , m_isResettingClippingRange(false)
{
  m_layerManager->SetDefaultCamera(m_defaultCamera);
  m_cameraSync->SetDefaultCamera(m_defaultCamera);

  m_eventObs->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (obj == m_factory)
      {
        UpdateFromScene();
      }

      if (obj == m_defaultCamera && !m_isResettingClippingRange)
      {
        ResetCameraClippingRange();
        OnDefaultCameraModified();
      }
    });

  // Monitor camera updates
  m_eventObs->UpdateObserver(nullptr, m_defaultCamera);
}

void vtkMRMLLayerDMPipelineManager::UpdatePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline) const
{
  if (!pipeline)
  {
    return;
  }

  auto prev = pipeline->BlockResetDisplay(true);
  pipeline->SetViewNode(m_viewNode);
  pipeline->BlockResetDisplay(prev);
  pipeline->ResetDisplay();
}

vtkSmartPointer<vtkMRMLLayerDMPipelineI> vtkMRMLLayerDMPipelineManager::GetNodePipeline(vtkMRMLNode* node) const
{
  const auto found = m_pipelineMap.find(node);
  if (found == std::end(m_pipelineMap))
  {
    return {};
  }
  return found->second;
}

void vtkMRMLLayerDMPipelineManager::SetRenderer(vtkRenderer* renderer) const
{
  m_cameraSync->SetRenderer(renderer);
}

void vtkMRMLLayerDMPipelineManager::SetRequestRender(const std::function<void()>& requestRender)
{
  m_requestRender = requestRender;
  UpdateAllPipelines();
}

vtkCamera* vtkMRMLLayerDMPipelineManager::GetDefaultCamera() const
{
  return m_defaultCamera;
}

void vtkMRMLLayerDMPipelineManager::RemoveOutdatedPipelines()
{
  if (!m_scene)
  {
    return;
  }

  for (const auto& pipe : m_pipelineMap)
  {
    if (!pipe.first || !m_scene->GetNodeByID(pipe.first->GetID()))
    {
      RemovePipeline(pipe.first);
    }
  }
}

void vtkMRMLLayerDMPipelineManager::AddMissingPipelines()
{
  if (!m_scene)
  {
    return;
  }

  int nNodes = m_scene->GetNumberOfNodes();
  for (int iNode = 0; iNode < nNodes; iNode++)
  {
    if (auto node = vtkMRMLNode::SafeDownCast(m_scene->GetNodes()->GetItemAsObject(iNode)))
    {
      AddNode(node);
    }
  }
}

void vtkMRMLLayerDMPipelineManager::UpdateFromScene()
{
  if (!m_scene)
  {
    return;
  }

  RemoveOutdatedPipelines();
  AddMissingPipelines();
}

void vtkMRMLLayerDMPipelineManager::SetScene(vtkMRMLScene* scene)
{
  if (m_scene == scene)
  {
    return;
  }

  m_scene = scene;
  for (const auto& [node, pipeline] : m_pipelineMap)
  {
    pipeline->SetScene(scene);
  }
}
