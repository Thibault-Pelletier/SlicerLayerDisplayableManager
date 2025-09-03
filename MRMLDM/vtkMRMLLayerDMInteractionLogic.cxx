#include "vtkMRMLLayerDMInteractionLogic.h"

#include "vtkMRMLLayerDMPipelineI.h"
#include "vtkMRMLInteractionEventData.h"

#include <vtkMRMLAbstractWidget.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMInteractionLogic);

vtkMRMLLayerDMPipelineI* vtkMRMLLayerDMInteractionLogic::GetLastFocusedPipeline() const
{
  return m_prevFocusedPipeline;
}

vtkMRMLLayerDMInteractionLogic::vtkMRMLLayerDMInteractionLogic()
  : m_prevFocusedPipeline{ nullptr }
  , m_canProcess{}
  , m_viewNode{ nullptr }
{
}

int vtkMRMLLayerDMInteractionLogic::MinWidgetState()
{
  return vtkMRMLAbstractWidget::WidgetStateOnWidget;
}

void vtkMRMLLayerDMInteractionLogic::LoseFocus(vtkMRMLInteractionEventData* eventData)
{
  if (m_prevFocusedPipeline)
  {
    m_prevFocusedPipeline->LoseFocus(eventData);
    m_prevFocusedPipeline = nullptr;
  }
}

void vtkMRMLLayerDMInteractionLogic::LoseFocus()
{
  vtkNew<vtkMRMLInteractionEventData> leaveEvent;
  leaveEvent->SetType(vtkCommand::LeaveEvent);
  leaveEvent->SetViewNode(m_viewNode);
  LoseFocus(leaveEvent);
}

void vtkMRMLLayerDMInteractionLogic::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  m_viewNode = viewNode;
}

std::vector<vtkSmartPointer<vtkMRMLLayerDMPipelineI>> vtkMRMLLayerDMInteractionLogic::GetCanProcessPipelines() const
{
  return m_canProcess;
}

std::tuple<double, int> vtkMRMLLayerDMInteractionLogic::PrioritizeCanProcessPipelines(vtkMRMLInteractionEventData* eventData)
{
  // For each pipeline, if pipeline can process, store its state value, layer and distance to interaction
  std::map<vtkMRMLLayerDMPipelineI*, std::tuple<int, unsigned int, double>> priority;
  double minDistance = std::numeric_limits<double>::max();
  int maxState = MinWidgetState();
  for (const auto& pipeline : m_pipelines)
  {
    double pipelineDistance = std::numeric_limits<double>::max();
    if (pipeline->CanProcessInteractionEvent(eventData, pipelineDistance))
    {
      m_canProcess.emplace_back(pipeline);
      int widgetState = std::max(MinWidgetState(), pipeline->GetWidgetState());
      minDistance = std::min(minDistance, pipelineDistance);
      maxState = std::max(widgetState, maxState);
      priority[pipeline] = std::make_tuple(widgetState, pipeline->GetRenderLayer(), -pipelineDistance);
    }
  }
  // Sort can process by layer order and inverted square distance (larger layer number first and closest to interaction)
  std::sort(m_canProcess.begin(),
            m_canProcess.end(),
            [&priority](const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& a, const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& b) { return priority[a] > priority[b]; });

  return std::make_tuple(minDistance, maxState);
}

void vtkMRMLLayerDMInteractionLogic::LosePreviousFocusInCannotProcess(vtkMRMLInteractionEventData* eventData)
{
  // Lose focus if previous focused pipeline cannot process current interaction
  if (std::find(m_canProcess.begin(), m_canProcess.end(), m_prevFocusedPipeline) == m_canProcess.end())
  {
    LoseFocus(eventData);
  }
}

void vtkMRMLLayerDMInteractionLogic::AddPipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline)
{
  if (std::find(m_pipelines.begin(), m_pipelines.end(), pipeline) != m_pipelines.end())
  {
    return;
  }
  m_pipelines.emplace_back(pipeline);
}

void vtkMRMLLayerDMInteractionLogic::RemovePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline)
{
  m_pipelines.erase(std::find(m_pipelines.begin(), m_pipelines.end(), pipeline));
}

bool vtkMRMLLayerDMInteractionLogic::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  // Clear previous interaction list
  m_canProcess.clear();

  // On leave event lose focus and early return to avoid bad pipeline state
  if (eventData->GetType() == vtkCommand::LeaveEvent)
  {
    LoseFocus(eventData);
    return false;
  }

  // Refresh the can process pipelines and order them by priority
  auto [minDistance, maxState] = PrioritizeCanProcessPipelines(eventData);

  // Lose previous focus if not in can process list
  LosePreviousFocusInCannotProcess(eventData);

  // Return lowest double value if any pipeline can process and is not idle
  // Otherwise, return the min distance returned by the processes.
  distance2 = maxState > MinWidgetState() ? std::numeric_limits<double>::lowest() : minDistance;
  return !m_canProcess.empty();
}

bool vtkMRMLLayerDMInteractionLogic::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  for (const auto& pipeline : m_canProcess)
  {
    // If pipeline can process, store pipeline for further interaction events
    if (pipeline->ProcessInteractionEvent(eventData))
    {
      if (pipeline != m_prevFocusedPipeline)
      {
        LoseFocus(eventData);
      }
      m_prevFocusedPipeline = pipeline;
      return true;
    }
  }

  // If no pipeline was able to process interaction, lose focus
  LoseFocus(eventData);
  return false;
}