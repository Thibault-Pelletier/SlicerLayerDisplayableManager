#include "vtkMRMLLayerDMPipelineScriptedCreator.h"
#include "vtkMRMLLayerDMPipelineI.h"

#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLNode.h"

#include <vtkObjectFactory.h>
#include <vtkSmartPyObject.h>
#include <vtkPythonUtil.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineScriptedCreator);

vtkMRMLLayerDMPipelineScriptedCreator::vtkMRMLLayerDMPipelineScriptedCreator()
  : m_object(nullptr)
{
  SetCallback(
    [this](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) -> vtkSmartPointer<vtkMRMLLayerDMPipelineI>
    {
      if (!Py_IsInitialized() || !m_object || !PyCallable_Check(m_object))
      {
        return nullptr;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;

      PyObject* pyViewNode = vtkPythonUtil::GetObjectFromPointer(viewNode);
      PyObject* pyNode = vtkPythonUtil::GetObjectFromPointer(node);
      vtkSmartPyObject pyArgs(PyTuple_Pack(2, pyViewNode, pyNode));

      Py_XDECREF(pyViewNode);
      Py_XDECREF(pyNode);
      PyObject* result = PyObject_CallObject(m_object, pyArgs);
      if (!result)
      {
        PyErr_Print();
        return nullptr;
      }
      return vtkMRMLLayerDMPipelineI::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkMRMLLayerDMPipelineI"));
    });
}

vtkMRMLLayerDMPipelineScriptedCreator::~vtkMRMLLayerDMPipelineScriptedCreator()
{
  if (Py_IsInitialized())
  {
    vtkPythonScopeGilEnsurer gilEnsurer;
    Py_XDECREF(m_object);
  }
}

void vtkMRMLLayerDMPipelineScriptedCreator::SetPythonCallback(PyObject* object)
{
  if (!Py_IsInitialized())
  {
    vtkErrorMacro("" << __func__ << "Python environment is not initialized correctly. Failed to set callback : " << object);
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (m_object == object)
  {
    return;
  }

  // Set the new python lambda
  Py_XDECREF(m_object);
  m_object = object;
  Py_INCREF(m_object);
}
