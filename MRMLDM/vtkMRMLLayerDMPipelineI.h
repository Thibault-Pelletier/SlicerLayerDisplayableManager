#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLAbstractViewNode.h"
#include "vtkObjectEventObserver.h"

#include <vtkObject.h>
#include <functional>
#include <vtkMRMLLayerDMPipelineManager.h>

class vtkCamera;
class vtkMRMLAbstractViewNode;
class vtkMRMLInteractionEventData;
class vtkMRMLLayerDMPipelineI;
class vtkMRMLLayerDMPipelineManager;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkObjectEventObserver;
class vtkRenderer;

/// \brief Interface for the layered displayable manager pipelines.
///
/// Contains empty implementation and default behavior for the different API calls.
/// Implementation can be limited to \sa UpdatePipeline and reactivity on node changes for pure
/// display pipelines.
///
/// Widget pipelines should also implement the \sa CanProcessInteractionEvent and \sa ProcessInteractionEvent
/// methods.
///
/// A python main class is available from \sa vtkMRMLLayerDMScriptedPipeline.py
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineI : public vtkObject
{
public:
  static vtkMRMLLayerDMPipelineI* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineI, vtkObject);

  /// true if the pipeline can process the input event data
  /// \param eventData: The MRML event needing to be processed
  /// \param distance2: Return value for the distance to the interaction (preferably actual RAS distance)
  /// \return true if the pipeline can process the input event data. Default = false;
  virtual bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2);

  /// Custom pipeline camera.
  /// If the returned value is not nullptr, then the pipeline (or dedicated logic) is expected to handle its own camera.
  /// Otherwise, the pipeline will be moved in a renderer with a default camera synchronized on its view default camera.
  /// \sa vtkMRMLLayerDMCameraSynchronizer
  /// \return nullptr by default.
  virtual vtkCamera* GetCamera() const;

  /// Custom mouse cursor from VTK mouse cursor enum.
  /// This value is only used if the pipeline actually processes an event and is ignore otherwise.
  virtual int GetMouseCursor() const;

  /// Arbitrary render layer number where the pipeline wants to be displayed.
  /// Return 0 to be at the default renderer (main 3D Slicer renderer)
  /// Return larger values to be rendered in overlay.
  /// \return default = 0
  virtual unsigned int GetRenderLayer() const;

  /// Current widget state of the pipeline.
  /// \return default = WidgetStateIdle
  virtual int GetWidgetState() const;

  /// Triggered when the pipeline had focus (processed an interaction) and loses the focus (other pipeline
  /// handled the new interaction or window leave event).
  /// default behavior: does nothing.
  virtual void LoseFocus(vtkMRMLInteractionEventData* eventData);

  /// Triggered when the default camera is modified.
  /// default behavior: does nothing.
  virtual void OnDefaultCameraModified(vtkCamera* camera);

  /// Triggered when the pipeline is displayed on a new renderer.
  /// default behavior: does nothing.
  virtual void OnRendererAdded(vtkRenderer* renderer);

  /// Triggered when the pipeline is removed from its previous renderer.
  /// default behavior: does nothing.
  virtual void OnRendererRemoved(vtkRenderer* renderer);

  /// Triggered when the pipeline can process the interaction and is at the top of the piority list.
  /// default behavior: does nothing.
  virtual bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData);

  /// Set the display node for the pipeline has changed (initialization).
  /// default behavior: Stored and display node is observed for vtkCommand::ModifiedEvent.
  /// \sa UpdateObserver
  /// \sa OnUpdate
  virtual void SetDisplayNode(vtkMRMLNode* displayNode);

  /// Set the pipeline manager (initialization).
  /// default behavior: Stores the pipeline manager to delegate request render calls (no active observer).
  virtual void SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager);

  /// Set the pipeline scene (initialization).
  /// default behavior: Stores the scene for access (no active observer).
  virtual void SetScene(vtkMRMLScene* scene);

  /// Set the pipeline view node  (initialization).
  /// default behavior: Stored and view node is observed for vtkCommand::ModifiedEvent.
  virtual void SetViewNode(vtkMRMLAbstractViewNode* viewNode);

  /// Triggered on \sa ResetDisplay calls
  /// default behavior: does nothing.
  virtual void UpdatePipeline();

  /// If \param isBlocked is true, \sa UpdatePipeline is not called during \sa ResetDisplay.
  bool BlockResetDisplay(bool isBlocked);

  /// Returns the current display node.
  vtkMRMLNode* GetDisplayNode() const;

  /// Returns the pipeline associated with the input node.
  /// Delegates to \sa vtkMRMLLayerDMPipelineManager::GetNodePipeline.
  /// nullptr if not found or pipelineManager instance is nullptr.
  vtkMRMLLayerDMPipelineI* GetNodePipeline(vtkMRMLNode* node) const;

  /// Returns the current renderer attached to the pipeline.
  /// \sa OnRendererAdded
  /// \sa OnRendererRemoved
  vtkRenderer* GetRenderer() const;

  /// Returns the current scene.
  vtkMRMLScene* GetScene() const;

  /// Returns the current view node.
  vtkMRMLAbstractViewNode* GetViewNode() const;

  /// @{
  /// Remove previous monitored events from \param prevObj and observe events from the \param obj.
  /// If both obj are the same, does nothing.
  /// On event triggerred, calls \sa OnUpdate
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events) const;
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent) const;
  /// @}

  /// Resets the pipeline display.
  /// Delegates actual work to \sa UpdatePipeline.
  /// Called the first time after pipeline initialization.
  void ResetDisplay();

  /// Request rendering and camera clipping reset.
  /// Calls are delegated to \sa vtkMRMLLayerDMPipelineManager::RequestRender.
  void RequestRender() const;

  /// Set the new renderer.
  /// Triggers \sa OnRendererAdded and \sa OnRendererRemoved if renderer has changed.
  void SetRenderer(vtkRenderer* renderer);

protected:
  vtkMRMLLayerDMPipelineI();
  ~vtkMRMLLayerDMPipelineI() override = default;

  /// Observer update callback.
  /// Triggered when any object & events observed using UpdateObserver is triggered.
  virtual void OnUpdate(vtkObject* obj, unsigned long eventId, void* callData);

private:
  vtkWeakPointer<vtkMRMLAbstractViewNode> m_viewNode;
  vtkWeakPointer<vtkMRMLNode> m_displayNode;
  vtkWeakPointer<vtkRenderer> m_renderer;
  bool m_isResetDisplayBlocked;
  vtkSmartPointer<vtkObjectEventObserver> m_obs;
  vtkWeakPointer<vtkMRMLLayerDMPipelineManager> m_pipelineManager;
  vtkWeakPointer<vtkMRMLScene> m_scene;
};
