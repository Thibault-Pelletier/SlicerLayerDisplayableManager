#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include <vtkObject.h>
#include <vtkWeakPointer.h>
#include <vtkSmartPointer.h>

#include <functional>
#include <map>
#include <vtkCommand.h>

class vtkCamera;
class vtkMRMLAbstractViewNode;
class vtkMRMLInteractionEventData;
class vtkMRMLLayerDMCameraSynchronizer;
class vtkMRMLLayerDMInteractionLogic;
class vtkMRMLLayerDMLayerManager;
class vtkMRMLLayerDMPipelineCreatorI;
class vtkMRMLLayerDMPipelineFactory;
class vtkMRMLLayerDMPipelineI;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkObjectEventObserver;
class vtkRenderWindow;
class vtkRenderer;

/// @brief Class responsible for handling adding / updating / removing pipelines depending on nodes added / removed /
/// updated in the 3D Slicer Scene.
///
/// The display manager handles connections to the scene's events.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineManager : public vtkObject
{
public:
  static vtkMRMLLayerDMPipelineManager* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineManager, vtkObject);

  /// Add a new node to the pipeline manager.
  /// If no pipeline exist for the input display node and the \sa vtkMRMLLayerDMPipelineFactory can create
  /// a pipeline, creates and stores the pipeline in the manager.
  bool AddNode(vtkMRMLNode* node);

  /// Delegates can process interaction event to \sa vtkMRMLLayerDMInteractionLogic
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) const;

  /// Create a new pipeline associated with the input display node.
  /// Delegates to \sa vtkMRMLLayerDMPipelineFactory::CreatePipeline.
  bool CreatePipelineForNode(vtkMRMLNode* displayNode);

  /// Returns the default camera for the pipeline.
  /// The camera synchronization is handled by \sa vtkMRMLLayerDMCameraSynchronizer.
  vtkCamera* GetDefaultCamera() const;

  /// Clear all pipelines from the pipeline manager.
  /// Should be called at delete.
  void ClearDisplayableNodes();

  /// Returns the mouse cursor from the latest pipeline having handled the latest interaction.
  int GetMouseCursor() const;

  /// Returns the pipeline associated with the input display node if any.
  vtkSmartPointer<vtkMRMLLayerDMPipelineI> GetNodePipeline(vtkMRMLNode* node) const;

  /// @{
  /// Makes the latest pipeline lose focus
  void LoseFocus() const;
  void LoseFocus(vtkMRMLInteractionEventData* eventData) const;
  /// @}

  /// Delegates process interaction event to \sa vtkMRMLLayerDMInteractionLogic
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) const;

  /// @{
  /// Remove node from the pipeline manager if any pipeline is associated to the input node.
  bool RemoveNode(vtkMRMLNode* node);
  bool RemovePipeline(vtkMRMLNode* displayNode);
  ///@}

  /// Reset camera clipping range and call display manager request render.
  void RequestRender();

  /// Delegate to \sa vtkMRMLLayerDMLayerManager::ResetCameraClippingRange
  void ResetCameraClippingRange();

  /// Set the Pipeline factory to use by the pipeline manager (initialization).
  /// On factory-modified event, will trigger a \sa UpdateFromScene.
  void SetFactory(const vtkSmartPointer<vtkMRMLLayerDMPipelineFactory>& factory);

  /// Set the render window on which the pipeline manager is attached (initialization).
  void SetRenderWindow(vtkRenderWindow* renderWindow) const;

  /// Set the default renderer used by the display manager (initialization).
  void SetRenderer(vtkRenderer* renderer) const;

  /// Set the request render callback used during \sa RequestRender.
  void SetRequestRender(const std::function<void()>& requestRender);

  /// Set the scene (initialization).
  void SetScene(vtkMRMLScene* scene);

  /// Set the view node (initialization).
  void SetViewNode(vtkMRMLAbstractViewNode* viewNode);

  /// Update all pipelines managed by the pipeline manager.
  void UpdateAllPipelines() const;

  /// Update the pipeline manager from the current MRML scene state.
  /// Will automatically remove or create pipelines depending on the scene state.
  void UpdateFromScene();

protected:
  vtkMRMLLayerDMPipelineManager();
  ~vtkMRMLLayerDMPipelineManager() override = default;

private:
  /// Notify pipelines that the default camera has changed.
  void OnDefaultCameraModified() const;

  /// Update the input pipeline and reset its display.
  void UpdatePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline) const;

  /// Remove pipelines with nodes not present in the scene anymore.
  void RemoveOutdatedPipelines();

  /// Add pipelines for nodes not currently handled by the pipeline manager.
  void AddMissingPipelines();

  vtkSmartPointer<vtkMRMLLayerDMPipelineFactory> m_factory;
  vtkSmartPointer<vtkMRMLLayerDMLayerManager> m_layerManager;
  vtkSmartPointer<vtkMRMLLayerDMCameraSynchronizer> m_cameraSync;
  vtkSmartPointer<vtkMRMLLayerDMInteractionLogic> m_interactionLogic;
  vtkSmartPointer<vtkObjectEventObserver> m_eventObs;
  vtkSmartPointer<vtkCamera> m_defaultCamera;

  vtkWeakPointer<vtkMRMLAbstractViewNode> m_viewNode;
  vtkWeakPointer<vtkMRMLScene> m_scene;

  std::map<vtkWeakPointer<vtkMRMLNode>, vtkSmartPointer<vtkMRMLLayerDMPipelineI>> m_pipelineMap;
  std::function<void()> m_requestRender;

  bool m_isResettingClippingRange;
};
