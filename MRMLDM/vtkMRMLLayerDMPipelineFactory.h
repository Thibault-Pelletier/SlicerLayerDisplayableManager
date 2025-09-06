#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include <functional>
#include <vector>

class vtkMRMLAbstractViewNode;
class vtkMRMLLayerDMPipelineCreatorI;
class vtkMRMLLayerDMPipelineI;
class vtkMRMLNode;

/// \brief Class responsible for creating new pipelines given input viewNode and Node pairs.
///
/// Delegates creation to its list of \sa vtkMRMLLayerDMPipelineCreatorI.
/// Early returns when a first creator capable of handling the input is found.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineFactory : public vtkObject
{
public:
  enum Events
  {
    // Triggered when the factory creates a non-empty pipeline
    PipelineAboutToBeCreatedEvent = vtkCommand::UserEvent + 1
  };

  static vtkMRMLLayerDMPipelineFactory* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineFactory, vtkObject);

  /// \brief Singleton instance of the factory used by the displayable manager
  static vtkSmartPointer<vtkMRMLLayerDMPipelineFactory> GetInstance();

  /// \brief Add the input creator to the list of creators.
  /// If the factory already contains the creator, does nothing.
  /// Invokes vtkCommand::ModifiedEvent if the factory is modified.
  void AddPipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& creator);

  /// Convenience method to add creator callback
  /// Delegates to \sa vtkMRMLLayerDMPipelineCallbackCreator and returns the creator instance.
  vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI> AddPipelineCreator(
    const std::function<vtkSmartPointer<vtkMRMLLayerDMPipelineI>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& creatorCallBack);

  /// \brief Remove the input creator from the list of creators.
  /// If the factory doesn't contain the creator, does nothing.
  /// Invokes vtkCommand::ModifiedEvent if the factory is modified.
  void RemovePipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& creator);

  /// true if the given creator is contained in the factory, false otherwise.
  bool ContainsPipelineCreator(const vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>& creator) const;

  /// Tries to create a new pipeline given input viewNode and node by iterating on its creators.
  /// Returns nullptr if no creator was able to create a pipeline.
  /// Invokes PipelineAboutToBeCreatedEvent before returning the newly created pipeline instance.
  /// \sa GetLastViewNode
  /// \sa GetLastNode
  /// \sa GetLastPipeline
  vtkSmartPointer<vtkMRMLLayerDMPipelineI> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node);

  /// @{
  /// Get the last pipeline created by the factory.
  /// Values are valid when the PipelineAboutToBeCreatedEvent event is triggered.
  vtkMRMLAbstractViewNode* GetLastViewNode() const;
  vtkMRMLNode* GetLastNode() const;
  vtkMRMLLayerDMPipelineI* GetLastPipeline() const;
  /// @}

protected:
  vtkMRMLLayerDMPipelineFactory();
  ~vtkMRMLLayerDMPipelineFactory() override = default;

private:
  std::vector<vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI>> m_pipelineCreators;
  vtkMRMLAbstractViewNode* m_lastView;
  vtkMRMLNode* m_lastNode;
  vtkMRMLLayerDMPipelineI* m_lastPipeline;
};