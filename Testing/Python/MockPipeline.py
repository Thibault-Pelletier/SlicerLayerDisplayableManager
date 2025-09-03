from slicer import vtkMRMLInteractionEventData

from unittest.mock import MagicMock
import sys

from LayerDMManagerLib import vtkMRMLLayerDMScriptedPipeline


class MockPipeline(vtkMRMLLayerDMScriptedPipeline):
    def __init__(self, layer=0, widgetState=0, canProcess=False, processDistance=sys.float_info.max, didProcess=False):
        super().__init__()
        self.layer = layer
        self.widgetState = widgetState
        self.mockCanProcess = MagicMock(return_value=(canProcess, processDistance))
        self.mockProcess = MagicMock(return_value=didProcess)
        self.mockLoseFocus = MagicMock()

    def GetRenderLayer(self) -> int:
        return self.layer

    def GetWidgetState(self) -> int:
        return self.widgetState

    def CanProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> tuple[bool, float]:
        return self.mockCanProcess(eventData)

    def ProcessInteractionEvent(self, eventData: vtkMRMLInteractionEventData) -> bool:
        return self.mockProcess(eventData)

    def LoseFocus(self, eventData: vtkMRMLInteractionEventData) -> None:
        self.mockLoseFocus(eventData)
