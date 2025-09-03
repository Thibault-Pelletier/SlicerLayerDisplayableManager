#include "vtkMRMLLayerDMScriptedPipelineBridge.h"

#include "vtkMRMLInteractionEventData.h"

#include <vtkObjectFactory.h>
#include <vtkSmartPyObject.h>
#include <vtkPythonUtil.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMScriptedPipelineBridge);

inline PyObject* ToPyObject(vtkObjectBase* obj)
{
  return vtkPythonUtil::GetObjectFromPointer(obj);
}

inline PyObject* ToPyObject(unsigned long value)
{
  return PyLong_FromUnsignedLong(value);
}

inline PyObject* ToPyObject(void* ptr)
{
  if (ptr)
  {
    return PyCapsule_New(ptr, nullptr, nullptr);
  }

  // Return borrowed reference to Py_None
  Py_INCREF(Py_None);
  return Py_None;
}

template <typename... Args>
vtkSmartPyObject ToPyArgs(Args... args)
{
  // Convert each argument to PyObject*
  PyObject* pyObjs[] = { ToPyObject(args)... };

  // Pack into a Python tuple and transfer ownership
  PyObject* pyTuple = PyTuple_New(sizeof...(args));
  for (size_t i = 0; i < sizeof...(args); ++i)
  {
    PyTuple_SET_ITEM(pyTuple, i, pyObjs[i]);
  }

  return { pyTuple };
}

void vtkMRMLLayerDMScriptedPipelineBridge::UpdatePipeline()
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod({}, __func__);
}

vtkMRMLLayerDMScriptedPipelineBridge::vtkMRMLLayerDMScriptedPipelineBridge()
  : m_object{ nullptr }
{
}

vtkMRMLLayerDMScriptedPipelineBridge::~vtkMRMLLayerDMScriptedPipelineBridge()
{
  if (Py_IsInitialized())
  {
    vtkPythonScopeGilEnsurer gilEnsurer;
    Py_XDECREF(m_object);
  }
}

bool vtkMRMLLayerDMScriptedPipelineBridge::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  if (!Py_IsInitialized())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = CallPythonMethod(ToPyArgs(eventData), __func__))
  {
    int canProcess;
    if (PyTuple_Check(result) && PyArg_ParseTuple(result, "pd", &canProcess, &distance2))
    {
      return canProcess;
    }

    // Unpack error or unexpected return type
    PyErr_SetString(PyExc_TypeError, "Expected a tuple[bool, float] return type");
    PyErr_Print();
  }

  return false;
}

vtkCamera* vtkMRMLLayerDMScriptedPipelineBridge::GetCamera() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetCamera();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  auto result = CallPythonMethod({}, __func__);
  if (result && (result != Py_None))
  {
    return vtkCamera::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkCamera"));
  }
  return Superclass::GetCamera();
}

int vtkMRMLLayerDMScriptedPipelineBridge::GetMouseCursor() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetMouseCursor();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = CallPythonMethod({}, __func__))
  {
    return PyLong_AsLong(result);
  }
  return Superclass::GetMouseCursor();
}

unsigned int vtkMRMLLayerDMScriptedPipelineBridge::GetRenderLayer() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetRenderLayer();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = CallPythonMethod({}, __func__))
  {
    return PyLong_AsLong(result);
  }
  return Superclass::GetRenderLayer();
}

int vtkMRMLLayerDMScriptedPipelineBridge::GetWidgetState() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetWidgetState();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = CallPythonMethod({}, __func__))
  {
    return PyLong_AsLong(result);
  }
  return Superclass::GetWidgetState();
}

void vtkMRMLLayerDMScriptedPipelineBridge::LoseFocus(vtkMRMLInteractionEventData* eventData)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(eventData), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnDefaultCameraModified(vtkCamera* camera)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(camera), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnRendererAdded(vtkRenderer* renderer)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(renderer), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnRendererRemoved(vtkRenderer* renderer)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(renderer), __func__);
}

bool vtkMRMLLayerDMScriptedPipelineBridge::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!Py_IsInitialized())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = CallPythonMethod(ToPyArgs(eventData), __func__))
  {
    return result == Py_True;
  }
  return false;
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetDisplayNode(vtkMRMLNode* displayNode)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  Superclass::SetDisplayNode(displayNode);
  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(displayNode), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  Superclass::SetViewNode(viewNode);
  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(viewNode), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetScene(vtkMRMLScene* scene)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  Superclass::SetScene(scene);
  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(scene), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  Superclass::SetPipelineManager(pipelineManager);
  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(pipelineManager), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetPythonObject(PyObject* object)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!object || object == m_object)
  {
    return;
  }

  // Set the new python lambda
  Py_XDECREF(m_object);
  m_object = object;
  Py_INCREF(m_object);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  CallPythonMethod(ToPyArgs(obj, eventId, callData), __func__);
}

PyObject* vtkMRMLLayerDMScriptedPipelineBridge::CallPythonMethod(const vtkSmartPyObject& pyArgs, const std::string& fName) const
{
  if (!m_object)
  {
    vtkErrorMacro("" << __func__ << ": Python object is invalid.");
    return nullptr;
  }

  PyObject* method = PyObject_GetAttrString(m_object, fName.c_str());
  if (!method || !PyCallable_Check(method))
  {
    vtkErrorMacro("" << __func__ << ": Invalid method : " << fName);
    return nullptr;
  }

  PyObject* result = PyObject_CallObject(method, pyArgs);
  if (!result)
  {
    PyErr_Print();
    return nullptr;
  }
  return result;
}
