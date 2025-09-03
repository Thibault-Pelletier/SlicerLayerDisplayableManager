#include "vtkMRMLLayerDisplayableManager.h"

#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkMRMLLayerDMPipelineManager.h"
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceViewDisplayableManagerFactory.h"
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"

#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDisplayableManager);

vtkMRMLLayerDisplayableManager::vtkMRMLLayerDisplayableManager()
  : m_pipelineManager(nullptr)
{
}

bool vtkMRMLLayerDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  if (!m_pipelineManager)
  {
    return false;
  }

  return m_pipelineManager->CanProcessInteractionEvent(eventData, distance2);
}

bool vtkMRMLLayerDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!m_pipelineManager)
  {
    return false;
  }

  return m_pipelineManager->ProcessInteractionEvent(eventData);
}

void vtkMRMLLayerDisplayableManager::RegisterInDefaultViews()
{
  RegisterInFactory(vtkMRMLSliceViewDisplayableManagerFactory::GetInstance());
  RegisterInFactory(vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance());
}

void vtkMRMLLayerDisplayableManager::RegisterInFactory(vtkMRMLDisplayableManagerFactory* factory)
{
  if (!factory || IsRegisteredInFactory(factory))
  {
    return;
  }

  const vtkNew<vtkMRMLLayerDisplayableManager> dm;
  factory->RegisterDisplayableManager(dm->GetClassName());
}

bool vtkMRMLLayerDisplayableManager::IsRegisteredInFactory(vtkMRMLDisplayableManagerFactory* factory)
{
  if (!factory)
  {
    return false;
  }

  const vtkNew<vtkMRMLLayerDisplayableManager> dm;
  return factory->IsDisplayableManagerRegistered(dm->GetClassName());
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneEndClose()
{
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || !m_pipelineManager)
  {
    return;
  }

  if (this->m_pipelineManager->AddNode(node))
  {
    this->RequestRender();
  }
}

void vtkMRMLLayerDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (this->GetMRMLScene()->IsBatchProcessing() || !m_pipelineManager)
  {
    return;
  }

  if (this->m_pipelineManager->RemoveNode(node))
  {
    this->RequestRender();
  }
}

void vtkMRMLLayerDisplayableManager::UnobserveMRMLScene()
{
  if (!m_pipelineManager)
  {
    return;
  }
  this->m_pipelineManager->ClearDisplayableNodes();
}

void vtkMRMLLayerDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  if (!m_pipelineManager)
  {
    return;
  }
  m_pipelineManager->SetScene(this->GetMRMLScene());
  m_pipelineManager->UpdateFromScene();
}

void vtkMRMLLayerDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{
  auto viewNode = vtkMRMLAbstractViewNode::SafeDownCast(caller);
  if (!viewNode || !m_pipelineManager)
  {
    return;
  }

  m_pipelineManager->SetViewNode(viewNode);
}

int vtkMRMLLayerDisplayableManager::GetMouseCursor()
{
  if (!m_pipelineManager)
  {
    return vtkMRMLAbstractDisplayableManager::GetMouseCursor();
  }
  return m_pipelineManager->GetMouseCursor();
}

void vtkMRMLLayerDisplayableManager::Create()
{
  vtkRenderer* renderer = this->GetRenderer();
  if (!renderer || !renderer->GetRenderWindow())
  {
    vtkErrorMacro("vtkMRMLLayerDisplayableManager::Create() failed: renderer is invalid");
    return;
  }

  if (!m_pipelineManager)
  {
    m_pipelineManager = vtkSmartPointer<vtkMRMLLayerDMPipelineManager>::New();
  }

  m_pipelineManager->SetRenderWindow(renderer->GetRenderWindow());
  m_pipelineManager->SetRenderer(renderer);
  m_pipelineManager->SetFactory(vtkMRMLLayerDMPipelineFactory::GetInstance());
  m_pipelineManager->SetScene(GetMRMLScene());
  m_pipelineManager->SetViewNode(vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode()));
  m_pipelineManager->SetRequestRender([this] { RequestRender(); });

  // Make sure the DM is up to date with the current scene state
  this->UpdateFromMRML();
}

void vtkMRMLLayerDisplayableManager::SetHasFocus(bool hasFocus, vtkMRMLInteractionEventData* eventData)
{
  Superclass ::SetHasFocus(hasFocus, eventData);
  if (m_pipelineManager && !hasFocus)
  {
    m_pipelineManager->LoseFocus(eventData);
  }
}
