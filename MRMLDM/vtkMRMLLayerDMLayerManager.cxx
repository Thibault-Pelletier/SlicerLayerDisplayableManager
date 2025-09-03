#include "vtkMRMLLayerDMLayerManager.h"

#include "vtkMRMLLayerDMPipelineI.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkBoundingBox.h>

vtkStandardNewMacro(vtkMRMLLayerDMLayerManager);

void vtkMRMLLayerDMLayerManager::AddPipeline(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  auto key = GetPipelineLayerKey(pipeline);
  if (!ContainsLayerKey(key))
  {
    m_pipelineLayers[key] = {};
  }
  m_pipelineLayers[key].emplace(pipeline);
  UpdateLayers();
}

vtkMRMLLayerDMLayerManager::LayerKey vtkMRMLLayerDMLayerManager::GetPipelineLayerKey(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (!pipeline)
  {
    return {};
  }
  return { pipeline->GetRenderLayer(), GetCameraId(pipeline->GetCamera()) };
}

int vtkMRMLLayerDMLayerManager::GetNumberOfDistinctLayers() const
{
  return static_cast<int>(m_pipelineLayers.size());
}

int vtkMRMLLayerDMLayerManager::GetNumberOfManagedLayers() const
{
  return GetNumberOfDistinctLayers() - 1;
}

int vtkMRMLLayerDMLayerManager::GetNumberOfRenderers() const
{
  return static_cast<int>(m_renderers.size());
}

void vtkMRMLLayerDMLayerManager::RemovePipeline(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  auto key = GetPipelineLayerKey(pipeline);
  if (!ContainsLayerKey(key))
  {
    return;
  }

  m_pipelineLayers[key].erase(pipeline);
  UpdateLayers();
}

void vtkMRMLLayerDMLayerManager::ResetCameraClippingRange() const
{
  // Reset first renderer clipping range
  if (auto defaultRenderer = GetDefaultRenderer())
  {
    defaultRenderer->ResetCameraClippingRange();
  }

  // Reset the managed renderers grouped by common cameras
  for (const auto& pair : m_cameraRendererMap)
  {
    ResetRenderersCameraClippingRange(pair.second, ComputeRenderersVisibleBounds(pair.second));
  }
}

void vtkMRMLLayerDMLayerManager::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  if (m_renderWindow == renderWindow)
  {
    return;
  }

  RemoveAllLayers();
  m_renderWindow = renderWindow;
  UpdateLayers();
}

void vtkMRMLLayerDMLayerManager::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (m_defaultCamera == camera)
  {
    return;
  }
  m_defaultCamera = camera;
  UpdateLayers();
}

vtkMRMLLayerDMLayerManager::vtkMRMLLayerDMLayerManager()
  : m_emptyPipeline(vtkSmartPointer<vtkMRMLLayerDMPipelineI>::New())
{
  AddPipeline(m_emptyPipeline);
}

vtkRenderer* vtkMRMLLayerDMLayerManager::GetRendererMatchingKey(const LayerKey& key)
{
  // If key index matches the default layer, return the render window's first renderer
  int keyIndex = GetKeyIndex(key);
  if (keyIndex == 0)
  {
    return GetDefaultRenderer();
  }

  // Otherwise, convert key index to matching managed renderer index and return the associated renderer
  int rendererIndex = keyIndex - 1;
  if (rendererIndex < 0 || rendererIndex >= GetNumberOfRenderers())
  {
    return nullptr;
  }
  return m_renderers[rendererIndex];
}

vtkRenderer* vtkMRMLLayerDMLayerManager::GetDefaultRenderer() const
{
  if (!m_renderWindow)
  {
    return nullptr;
  }
  return m_renderWindow->GetRenderers()->GetFirstRenderer();
}

void vtkMRMLLayerDMLayerManager::AddMissingLayers()
{
  while (GetNumberOfRenderers() < GetNumberOfManagedLayers())
  {
    // Managed renderers are displayed as overlays and should not catch any events.
    // Events handling is done using the DM mechanism.
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->InteractiveOff();
    m_renderWindow->AddRenderer(renderer);
    m_renderers.emplace_back(renderer);
  }
}

std::array<double, 6> vtkMRMLLayerDMLayerManager::ComputeRenderersVisibleBounds(const std::set<vtkWeakPointer<vtkRenderer>>& renderers)
{
  vtkBoundingBox bbox;

  for (const auto& renderer : renderers)
  {
    if (!renderer)
    {
      continue;
    }
    bbox.AddBounds(renderer->ComputeVisiblePropBounds());
  }

  std::array<double, 6> bounds{};
  bbox.GetBounds(bounds.data());
  return bounds;
}

bool vtkMRMLLayerDMLayerManager::ContainsLayerKey(const LayerKey& key)
{
  return m_pipelineLayers.find(key) != m_pipelineLayers.end();
}

std::uintptr_t vtkMRMLLayerDMLayerManager::GetCameraId(vtkCamera* camera)
{
  if (!camera)
  {
    return 0;
  }
  return reinterpret_cast<std::uintptr_t>(camera);
}

vtkCamera* vtkMRMLLayerDMLayerManager::GetCameraForLayer(const LayerKey& key, const std::set<vtkWeakPointer<vtkMRMLLayerDMPipelineI>>& pipelines) const
{
  auto cameraId = std::get<1>(key);
  if (cameraId == 0)
  {
    return m_defaultCamera;
  }

  for (const auto& pipeline : pipelines)
  {
    if (pipeline)
    {
      return pipeline->GetCamera();
    }
  }

  return nullptr;
}

int vtkMRMLLayerDMLayerManager::GetKeyIndex(const LayerKey& key) const
{
  int index = 0;
  for (const auto& pair : m_pipelineLayers)
  {
    if (pair.first == key)
    {
      return index;
    }
    ++index;
  }
  return -1;
}

void vtkMRMLLayerDMLayerManager::RemoveAllLayers()
{
  for (const auto& renderer : m_renderers)
  {
    RemoveRenderer(renderer);
  }
  UpdateRenderWindowNumberOfLayers();
  m_renderers.clear();
}

void vtkMRMLLayerDMLayerManager::RemoveAllPipelineRenderers()
{
  // if the render window is null, notify pipelines
  for (const auto& [key, pipelines] : m_pipelineLayers)
  {
    for (const auto& pipeline : pipelines)
    {
      if (pipeline)
      {
        pipeline->SetRenderer(nullptr);
      }
    }
  }
}

void vtkMRMLLayerDMLayerManager::RemoveOutdatedLayers()
{
  while (GetNumberOfRenderers() && (GetNumberOfRenderers() > GetNumberOfManagedLayers()))
  {
    RemoveRenderer(m_renderers[GetNumberOfRenderers() - 1]);
  }
}

void vtkMRMLLayerDMLayerManager::RemoveOutdatedPipelines()
{
  // Remove pipelines which have been garbage collected
  for (auto& [key, pipelines] : m_pipelineLayers)
  {
    for (const auto& pipeline : pipelines)
    {
      if (!pipeline)
      {
        pipelines.erase(pipeline);
      }
    }

    if (pipelines.empty())
    {
      m_pipelineLayers.erase(key);
    }
  }
}

void vtkMRMLLayerDMLayerManager::RemoveRenderer(const vtkSmartPointer<vtkRenderer>& renderer)
{
  if (m_renderWindow && m_renderWindow->HasRenderer(renderer))
  {
    m_renderWindow->RemoveRenderer(renderer);
  }

  m_renderers.erase(std::find(m_renderers.begin(), m_renderers.end(), renderer));
}

void vtkMRMLLayerDMLayerManager::ResetRenderersCameraClippingRange(const std::set<vtkWeakPointer<vtkRenderer>>& renderers, const std::array<double, 6>& bounds)
{
  for (const auto& renderer : renderers)
  {
    if (!renderer)
    {
      continue;
    }
    renderer->ResetCameraClippingRange(bounds.data());
  }
}

void vtkMRMLLayerDMLayerManager::SynchronizePipelineRenderers()
{
  for (const auto& pair : m_pipelineLayers)
  {
    auto renderer = GetRendererMatchingKey(pair.first);
    for (const auto& pipeline : pair.second)
    {
      if (pipeline)
      {
        pipeline->SetRenderer(renderer);
      }
    }
  }
}

void vtkMRMLLayerDMLayerManager::UpdateRenderWindowNumberOfLayers() const
{
  if (!m_renderWindow)
  {
    return;
  }

  // Synchronize the render window number of layers with its actual number of renderers
  int numberOfRenderers = m_renderWindow->GetRenderers()->GetNumberOfItems();
  int iMax = 0;
  for (int iRenderer = 0; iRenderer < numberOfRenderers; iRenderer++)
  {
    if (auto renderer = vtkRenderer::SafeDownCast(m_renderWindow->GetRenderers()->GetItemAsObject(iRenderer)))
    {
      iMax = std::max(iMax, renderer->GetLayer());
    }
  }

  m_renderWindow->SetNumberOfLayers(iMax + 1);
}

void vtkMRMLLayerDMLayerManager::UpdateLayers()
{
  if (!m_renderWindow)
  {
    RemoveAllPipelineRenderers();
    return;
  }

  RemoveOutdatedPipelines();
  RemoveOutdatedLayers();
  AddMissingLayers();
  UpdateRenderWindowLayerOrdering();
  UpdateRendererCamera();
  SynchronizePipelineRenderers();
}

void vtkMRMLLayerDMLayerManager::UpdateRenderWindowLayerOrdering() const
{
  // Managed layers are always ordered from layer 1 to the number of managed renderers
  for (int iRenderer = 0; iRenderer < GetNumberOfRenderers(); iRenderer++)
  {
    m_renderers[iRenderer]->SetLayer(iRenderer + 1);
  }
  UpdateRenderWindowNumberOfLayers();
}

void vtkMRMLLayerDMLayerManager::UpdateRendererCamera()
{
  // Set the camera for the managed renderers
  // Layer 0 is unmanaged and its camera is left unchanged by the layer manager
  // Pipelines with no explicit camera map to the default camera
  // Pipelines with custom camera are grouped and use their cameras
  m_cameraRendererMap.clear();

  int iRenderer = -1;
  for (const auto& pair : m_pipelineLayers)
  {
    if (iRenderer >= 0 && iRenderer < GetNumberOfRenderers())
    {
      auto camera = GetCameraForLayer(pair.first, pair.second);
      m_renderers[iRenderer]->SetActiveCamera(camera);
      m_cameraRendererMap[camera].emplace(m_renderers[iRenderer]);
    }

    iRenderer++;
  }
}