#include "vtkObjectSet.h"
#include "vtkObjectObserver.h"

#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

vtkObjectSet::vtkObjectSet()
{
}

vtkObjectSet::~vtkObjectSet()
{

}

void vtkObjectSet::AddObserver( vtkObjectObserver* observer )
{
  m_vtkObjectObservers.insert(observer);
}

void vtkObjectSet::RemoveObserver( vtkObjectObserver* observer )
{
  m_vtkObjectObservers.erase(observer);
}

void vtkObjectSet::AddObject ( vtkObject * obj )
{
  // if add object, add observer for modified and delete event
  std::pair<std::set<const vtkObject *>::iterator, bool> res = m_Objects.insert(obj);

  if(res.second)
  {
    vtkSmartPointer<vtkCallbackCommand> modifiedCallback =
        vtkSmartPointer<vtkCallbackCommand>::New();
    modifiedCallback->SetCallback ( vtkObjectSet::OnObjectModified );
    modifiedCallback->SetClientData(this);

    m_ObjectModifiedTags[obj] = obj->AddObserver
                                 ( vtkCommand::ModifiedEvent, modifiedCallback );
    m_ObjectDeleteTags[obj] = obj->AddObserver
                                 ( vtkCommand::DeleteEvent, modifiedCallback );
  }
}

void vtkObjectSet::RemoveObject ( vtkObject * obj )
{
  // if object removed, remove observer and tags for modified and delete event
  std::set<const vtkObject *>::size_type deletedObjs = m_Objects.erase(obj);

  if(deletedObjs > 0)
  {
    obj->RemoveObserver(m_ObjectModifiedTags[obj]);
    obj->RemoveObserver(m_ObjectDeleteTags[obj]);

    m_ObjectModifiedTags.erase(obj);
    m_ObjectDeleteTags.erase(obj);
  }
}

void vtkObjectSet::OnObjectModified(vtkObject* caller, unsigned long eid
                                 , void* clientdata, void *calldata)
{
  // if event == modified event, inform objectobservers
  // else: call RemoveObject() or remove objectobserver and inform objectobservers

  vtkObjectSet* instance = static_cast<vtkObjectSet*>( clientdata );

  for(std::set<vtkObjectObserver*>::iterator it = instance->m_vtkObjectObservers.begin();
    it != instance->m_vtkObjectObservers.end(); ++it)
    eid == vtkCommand::ModifiedEvent ? (*it)->OnModified( caller ):
        (*it)->OnDeleted( caller );

  if(eid != vtkCommand::ModifiedEvent)
    instance->RemoveObject( caller );
}
