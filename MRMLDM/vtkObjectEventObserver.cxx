#include "vtkObjectEventObserver.h"

#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkObjectEventObserver);

template <class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

vtkObjectEventObserver::vtkObjectEventObserver()
  : m_updateCommand(vtkSmartPointer<vtkCallbackCommand>::New())
{
  m_updateCommand->SetClientData(this);
  m_updateCommand->SetCallback(
    [](vtkObject* caller, unsigned long eid, void* clientData, void* callData)
    {
      auto client = static_cast<vtkObjectEventObserver*>(clientData);
      try
      {
        // Dispatch to callback depending on current std variant content
        std::visit(Overloaded{ [&](const std::function<void(vtkObject * node)>& f) { f(caller); },
                               [&](const std::function<void(vtkObject * node, unsigned long eventId)>& f) { f(caller, eid); },
                               [&](const std::function<void(vtkObject * node, unsigned long eventId, void* callData)>& f) { f(caller, eid, callData); } },
                   client->m_callback);
      }
      catch (const std::bad_function_call&)
      {
        // Ignore unset function callbacks
      }
    });
}

vtkObjectEventObserver::~vtkObjectEventObserver()
{
  for (const auto& obs : m_obsMap)
  {
    if (obs.first)
    {
      for (auto& event : obs.second)
      {
        obs.first->RemoveObserver(event);
      }
    }
  }
}

bool vtkObjectEventObserver::UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event)
{
  return UpdateObserver(prevObj, obj, std::vector<unsigned long>{ event });
}

bool vtkObjectEventObserver::UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events)
{
  if (prevObj == obj)
  {
    return false;
  }

  RemoveObserver(prevObj);
  for (const auto& event : events)
  {
    AddObserver(obj, event);
  }
  return true;
}

void vtkObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node)>& callback)
{
  m_callback = callback;
}

void vtkObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback)
{
  m_callback = callback;
}

void vtkObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback)
{
  m_callback = callback;
}

void vtkObjectEventObserver::AddObserver(vtkObject* node, unsigned long event)
{
  if (!node)
  {
    return;
  }

  if (m_obsMap.find(node) == std::end(m_obsMap))
  {
    m_obsMap[node] = std::set<unsigned long>{};
  }

  if (m_obsMap[node].find(event) != std::end(m_obsMap[node]))
  {
    return;
  }

  m_obsMap[node].insert(node->AddObserver(event, m_updateCommand));
}

void vtkObjectEventObserver::RemoveObserver(vtkObject* node)
{
  if (!node || m_obsMap.find(node) == std::end(m_obsMap))
  {
    return;
  }

  for (auto& event : m_obsMap[node])
  {
    node->RemoveObserver(event);
  }

  m_obsMap.erase(node);
}
