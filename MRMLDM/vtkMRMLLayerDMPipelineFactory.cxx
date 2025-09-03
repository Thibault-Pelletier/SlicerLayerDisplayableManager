#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkMRMLLayerDMPipelineCreatorI.h"
#include "vtkMRMLLayerDMPipelineCallbackCreator.h"
#include "vtkMRMLLayerDMPipelineI.h"

#include <vtkCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineFactory);

vtkSmartPointer<vtkMRMLLayerDMPipelineFactory> vtkMRMLLayerDMPipelineFactory::GetInstance()
{
  static vtkSmartPointer<vtkMRMLLayerDMPipelineFactory> instance = vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>::New();
  return instance;
}

void vtkMRMLLayerDMPipelineFactory::AddPipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& creator)
{
  if (ContainsPipelineCreator(creator))
  {
    return;
  }

  m_pipelineCreators.emplace_back(creator);
  InvokeEvent(vtkCommand::ModifiedEvent);
}

vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI> vtkMRMLLayerDMPipelineFactory::AddPipelineCreator(
  const std::function<vtkSmartPointer<vtkMRMLLayerDMPipelineI>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& creatorCallBack)
{
  auto creator = vtkSmartPointer<vtkMRMLLayerDMPipelineCallbackCreator>::New();
  creator->SetCallback(creatorCallBack);
  AddPipelineCreator(creator);
  return creator;
}

void vtkMRMLLayerDMPipelineFactory::RemovePipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& creator)
{
  size_t prevSize = m_pipelineCreators.size();
  m_pipelineCreators.erase(
    std::remove_if(m_pipelineCreators.begin(), m_pipelineCreators.end(), [creator](const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& value) { return value == creator; }),
    m_pipelineCreators.end());
  if (m_pipelineCreators.size() != prevSize)
  {
    InvokeEvent(vtkCommand::ModifiedEvent);
  }
}

bool vtkMRMLLayerDMPipelineFactory::ContainsPipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& creator) const
{
  return std::find(m_pipelineCreators.begin(), m_pipelineCreators.end(), creator) != m_pipelineCreators.end();
}

vtkSmartPointer<vtkMRMLLayerDMPipelineI> vtkMRMLLayerDMPipelineFactory::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
{
  for (const auto& ctor : m_pipelineCreators)
  {
    if (auto created = ctor->CreatePipeline(viewNode, node))
    {
      m_lastView = viewNode;
      m_lastNode = node;
      m_lastPipeline = created;
      InvokeEvent(PipelineAboutToBeCreatedEvent);
      return created;
    }
  }

  return {};
}

vtkMRMLAbstractViewNode* vtkMRMLLayerDMPipelineFactory::GetLastViewNode() const
{
  return m_lastView;
}

vtkMRMLNode* vtkMRMLLayerDMPipelineFactory::GetLastNode() const
{
  {
    return m_lastNode;
  }
}

vtkMRMLLayerDMPipelineI* vtkMRMLLayerDMPipelineFactory::GetLastPipeline() const
{
  {
    return m_lastPipeline;
  }
}

vtkMRMLLayerDMPipelineFactory::vtkMRMLLayerDMPipelineFactory()
  : m_lastView(nullptr)
  , m_lastNode(nullptr)
  , m_lastPipeline(nullptr)
{
}