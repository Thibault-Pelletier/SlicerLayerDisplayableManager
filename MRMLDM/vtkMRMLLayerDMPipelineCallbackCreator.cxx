#include "vtkMRMLLayerDMPipelineCallbackCreator.h"
#include "vtkMRMLLayerDMPipelineI.h"

#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineCallbackCreator);

vtkSmartPointer<vtkMRMLLayerDMPipelineI> vtkMRMLLayerDMPipelineCallbackCreator::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const
{
  return m_callback(viewNode, node);
}

void vtkMRMLLayerDMPipelineCallbackCreator::SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipelineI>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback)
{
  m_callback = callback;
}

vtkMRMLLayerDMPipelineCallbackCreator::vtkMRMLLayerDMPipelineCallbackCreator()
  : m_callback([](vtkMRMLAbstractViewNode*, vtkMRMLNode*) { return nullptr; }) {};
