from typing import Any

from slicer import (
    vtkMRMLAbstractViewNode,
    vtkMRMLAbstractWidget,
    vtkMRMLInteractionEventData,
    vtkMRMLLayerDMPipelineManager,
    vtkMRMLLayerDMScriptedPipelineBridge,
    vtkMRMLNode,
    vtkMRMLScene,
)
from vtk import vtkCamera, vtkRenderer, vtkObject


class vtkMRMLLayerDMScriptedPipeline(vtkMRMLLayerDMScriptedPipelineBridge):
    def __init__(self):
        self.SetPythonObject(self)

    @property
    def viewNode(self) -> vtkMRMLAbstractViewNode:
        return self.GetViewNode()

    @property
    def displayNode(self) -> vtkMRMLNode:
        return self.GetDisplayNode()

    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        import sys

        return False, sys.float_info.max

    def GetCamera(self) -> vtkCamera | None:
        return None

    def GetMouseCursor(self) -> int:
        return 0

    def GetRenderLayer(self) -> int:
        return 0

    def GetWidgetState(self) -> int:
        return vtkMRMLAbstractWidget.WidgetStateIdle

    def LoseFocus(self, eventData: vtkMRMLInteractionEventData) -> None:
        pass

    def OnDefaultCameraModified(self, camera: vtkCamera) -> None:
        pass

    def OnRendererAdded(self, renderer: vtkRenderer) -> None:
        pass

    def OnRendererRemoved(self, renderer: vtkRenderer) -> None:
        pass

    def OnUpdate(self, obj: vtkObject, eventId: int, callData: Any) -> None:
        pass

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        return False

    def SetDisplayNode(self, displayNode: vtkMRMLNode) -> None:
        pass

    def SetViewNode(self, viewNode: vtkMRMLAbstractViewNode) -> None:
        pass

    def SetScene(self, scene: vtkMRMLScene) -> None:
        pass

    def SetPipelineManager(self, pipelineManager: vtkMRMLLayerDMPipelineManager) -> None:
        pass

    def UpdatePipeline(self) -> None:
        pass
