#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"
#include <vtkMRMLAbstractDisplayableManager.h>

#include <vtkSmartPointer.h>

class vtkMRMLLayerDMPipelineManager;
class vtkMRMLDisplayableManagerFactory;

/// \brief Displayable manager responsible for handling multiple displayable pipelines in sublayers.
///
/// This displayable manager is a generic displayable manager with an internal \sa vtkMRMLLayerDMPipelineManager implementation
/// expecting a one-to-one relationship between a pair of view node, node and display pipeline.
///
/// Interactions are delegated to the pipeline manager which then dispatches to the different pipelines in their different renderer layers.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDisplayableManager : public vtkMRMLAbstractDisplayableManager
{
public:
  static vtkMRMLLayerDisplayableManager* New();
  vtkTypeMacro(vtkMRMLLayerDisplayableManager, vtkMRMLAbstractDisplayableManager);

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  /// Register the DM in the 2D and 3D view factories
  static void RegisterInDefaultViews();

  /// Register the Displayable Manager in the given input factory if it's not already registered
  static void RegisterInFactory(vtkMRMLDisplayableManagerFactory* factory);

  /// true if the input factory is defined and displayable manager is present in the input factory
  static bool IsRegisteredInFactory(vtkMRMLDisplayableManagerFactory* factory);

protected:
  vtkMRMLLayerDisplayableManager();
  ~vtkMRMLLayerDisplayableManager() override = default;
  vtkMRMLLayerDisplayableManager(const vtkMRMLLayerDisplayableManager&) = delete;
  void operator=(const vtkMRMLLayerDisplayableManager&) = delete;

  void Create() override;

  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;
  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneEndBatchProcess() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void UnobserveMRMLScene() override;
  void UpdateFromMRML() override;

  int GetMouseCursor() override;
  void SetHasFocus(bool hasFocus, vtkMRMLInteractionEventData* eventData) override;

private:
  vtkSmartPointer<vtkMRMLLayerDMPipelineManager> m_pipelineManager;
};
