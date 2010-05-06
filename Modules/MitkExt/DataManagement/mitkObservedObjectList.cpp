#include "mitkObservedObjectList.h"

#include "mitkObjectObserver.h"
#include <itkCommand.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

namespace mitk {

ObservedObjectList::ObservedObjectList()
{
}

ObservedObjectList::~ObservedObjectList()
{

}

void ObservedObjectList::AddObserver( ObjectObserver* observer )
{
  m_ObjectObservers.insert(observer);
  // try to add the observer itself to the list
  // thus, ObservedObjectList gets notified when
  // one of its observers gets deleted. useful
  // because the observer must then not call removeobserver()
  // manually
  itk::Object * object = dynamic_cast<itk::Object *> ( observer );
  if(object)
  {
    this->AddObject(object);
  }
}

void ObservedObjectList::RemoveObserver( ObjectObserver* observer )
{
  m_ObjectObservers.erase(observer);
}


void ObservedObjectList::AddObject ( itk::Object * obj )
{
  // if add object, add observer for modified and delete event
  std::pair<std::set<const itk::Object *>::iterator, bool> res = m_ItkObjects.insert(obj);

  if(res.second)
  {
    // subscribe for modified event
    itk::MemberCommand<ObservedObjectList>::Pointer _modifiedCommand =
      itk::MemberCommand<ObservedObjectList>::New();
    _modifiedCommand->SetCallbackFunction(this
      , &ObservedObjectList::OnItkObjectModified);
    m_ItkObjectModifiedTags[obj] =
        obj->AddObserver(itk::ModifiedEvent(), _modifiedCommand);

    // subscribe for delete event
    itk::MemberCommand<ObservedObjectList>::Pointer _DeleteCommand =
      itk::MemberCommand<ObservedObjectList>::New();
    _DeleteCommand->SetCallbackFunction(this
      , &ObservedObjectList::OnItkObjectModified);
    m_ItkObjectDeleteTags[obj] =
        obj->AddObserver(itk::DeleteEvent(), _DeleteCommand);
  }
}

void ObservedObjectList::RemoveObject ( itk::Object * obj )
{
  // if object removed, remove observer for modified and delete event
  // and observer tags
  std::set<const itk::Object *>::size_type deletedObjs = m_ItkObjects.erase(obj);

  if(deletedObjs > 0)
  {
    obj->RemoveObserver(m_ItkObjectModifiedTags[obj]);
    obj->RemoveObserver(m_ItkObjectDeleteTags[obj]);

    m_ItkObjectModifiedTags.erase(obj);
    m_ItkObjectDeleteTags.erase(obj);
  }

}

//const itk::Object * ObservedObjectList::GetItkObject (unsigned int i) const
//{
//  if( m_ItkObjects.size() > 0 && i < m_ItkObjects.size() )
//  {
//    std::set<const itk::Object *>::const_iterator it = m_ItkObjects.begin();
//    std::advance(it, i);
//    return *(it);
//  }
//  else return 0;
//}
//
//unsigned int ObservedObjectList::GetSizeOfItkObjects () const
//{
//  return m_ItkObjects.size();
//}


void ObservedObjectList::AddObject ( vtkObject * obj )
{
  // if add object, add observer for modified and delete event
  std::pair<std::set<const vtkObject *>::iterator, bool> res = m_VtkObjects.insert(obj);

  if(res.second)
  {
    vtkSmartPointer<vtkCallbackCommand> modifiedCallback =
        vtkSmartPointer<vtkCallbackCommand>::New();
    modifiedCallback->SetCallback ( ObservedObjectList::OnVtkObjectModified );
    modifiedCallback->SetClientData(this);

    m_VtkObjectModifiedTags[obj] = obj->AddObserver
                                 ( vtkCommand::ModifiedEvent, modifiedCallback );
    m_VtkObjectDeleteTags[obj] = obj->AddObserver
                                 ( vtkCommand::DeleteEvent, modifiedCallback );
  }
}

void ObservedObjectList::RemoveObject ( vtkObject * obj )
{
  // if object removed, remove observer and tags for modified and delete event
  std::set<const vtkObject *>::size_type deletedObjs = m_VtkObjects.erase(obj);

  if(deletedObjs > 0)
  {
    obj->RemoveObserver(m_VtkObjectModifiedTags[obj]);
    obj->RemoveObserver(m_VtkObjectDeleteTags[obj]);

    m_VtkObjectModifiedTags.erase(obj);
    m_VtkObjectDeleteTags.erase(obj);
  }
}

//const vtkObject * ObservedObjectList::GetVtkObject (unsigned int i) const
//{
//  if( m_VtkObjects.size() > 0 && i < m_VtkObjects.size() )
//  {
//    std::set<const vtkObject *>::const_iterator it = m_VtkObjects.begin();
//    std::advance(it, i);
//    return *(it);
//  }
//  else return 0;
//}

//
//unsigned int ObservedObjectList::GetSizeOfVtkObjects () const
//{
//  return m_VtkObjects.size();
//}


void ObservedObjectList::OnItkObjectModified(const itk::Object* caller
                                        , const itk::EventObject &event)
{
  // if event == modified event, inform objectobservers
  // else: call RemoveObject() and inform objectobservers

  const itk::ModifiedEvent* modifiedEvent
      = dynamic_cast<const itk::ModifiedEvent*>(&event);

  for(std::set<ObjectObserver*>::iterator it = m_ObjectObservers.begin();
    it != m_ObjectObservers.end(); ++it)
    modifiedEvent ? (*it)->OnModified( caller ): (*it)->OnDeleted( caller );

  if(!modifiedEvent)
    this->RemoveObject(const_cast<itk::Object*>(caller));
}

void ObservedObjectList::OnVtkObjectModified(vtkObject* caller, unsigned long eid
                                 , void* clientdata, void *calldata)
{
  // if event == modified event, inform objectobservers
  // else: call RemoveObject() or remove objectobserver and inform objectobservers

  ObservedObjectList* instance = static_cast<ObservedObjectList*>( clientdata );

  for(std::set<ObjectObserver*>::iterator it = instance->m_ObjectObservers.begin();
    it != instance->m_ObjectObservers.end(); ++it)
    eid == vtkCommand::ModifiedEvent ? (*it)->OnModified( caller ):
        (*it)->OnDeleted( caller );

  if(eid != vtkCommand::ModifiedEvent)
    instance->RemoveObject( caller );
}

} // namespace mitk
