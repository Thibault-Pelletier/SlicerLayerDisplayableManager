import slicer
from LayerDMManagerLib import vtkMRMLLayerDMScriptedPipeline
from slicer import (
    vtkMRMLLayerDisplayableManager,
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineScriptedCreator,
    vtkMRMLModelNode,
    vtkMRMLSliceViewDisplayableManagerFactory,
    vtkMRMLThreeDViewDisplayableManagerFactory,
)
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkRenderer, vtkActor, vtkObjectFactory


class Pipeline(vtkMRMLLayerDMScriptedPipeline):
    def __init__(self):
        super().__init__()
        self.actor = vtkActor()

    def OnRendererAdded(self, renderer: vtkRenderer):
        if renderer:
            renderer.AddViewProp(self.actor)

    def OnRendererRemoved(self, renderer: vtkRenderer):
        if renderer and renderer.HasViewProp(self.actor):
            renderer.RemoveViewProp(self.actor)


class DisplayableManagerTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

        # Verify the DM can be created by the vtk Object Factory
        assert vtkObjectFactory.CreateInstance(vtkMRMLLayerDisplayableManager.__name__) is not None

        # Register the DM to the default views
        vtkMRMLLayerDisplayableManager.RegisterInDefaultViews()

        # Verify registration is correct for both 2D and 3D views
        assert vtkMRMLLayerDisplayableManager.IsRegisteredInFactory(
            vtkMRMLSliceViewDisplayableManagerFactory.GetInstance()
        )
        assert vtkMRMLLayerDisplayableManager.IsRegisteredInFactory(
            vtkMRMLThreeDViewDisplayableManagerFactory.GetInstance()
        )

    def test_registered_pipelines_can_add_their_actors_to_views(self):
        threeDNode = slicer.mrmlScene.GetNodeByID("vtkMRMLViewNode1")

        # Register a pipeline creator with static pipeline
        pipeline = Pipeline()
        factory = vtkMRMLLayerDMPipelineFactory.GetInstance()
        creator = vtkMRMLLayerDMPipelineScriptedCreator()
        creator.SetPythonCallback(
            lambda view, node: pipeline if view == threeDNode and isinstance(node, vtkMRMLModelNode) else None
        )
        factory.AddPipelineCreator(creator)

        # Trigger pipeline creation by adding a new model node to the scene
        slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")

        # Verify the pipeline"s actor is present in the threed view
        renderWindow = slicer.app.layoutManager().viewWidget(threeDNode).viewWidget().renderWindow()
        renderer = renderWindow.GetRenderers().GetItemAsObject(0)
        assert renderer.HasViewProp(pipeline.actor)
