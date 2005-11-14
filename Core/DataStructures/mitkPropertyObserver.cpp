
#include <mitkPropertyObserver.h>
#include <itkCommand.h>
#include <exception>

namespace mitk {


PropertyObserver::PropertyObserver( mitk::BaseProperty* property )
{
  if ( !property ) throw std::invalid_argument("NULL pointer makes no sense in PropertyObserver()"); // no NULL property allowed
  {
    itk::ReceptorMemberCommand<PropertyObserver>::Pointer command = itk::ReceptorMemberCommand<PropertyObserver>::New();
    command->SetCallbackFunction(this, &PropertyObserver::OnModified);
    m_ModifiedObserverTag = property->AddObserver( itk::ModifiedEvent(), command );
  }
  {
    itk::ReceptorMemberCommand<PropertyObserver>::Pointer command = itk::ReceptorMemberCommand<PropertyObserver>::New();
    command->SetCallbackFunction(this, &PropertyObserver::OnDelete);
    m_DeleteObserverTag = property->AddObserver( itk::DeleteEvent(), command );
  }
}


PropertyObserver::~PropertyObserver()
{
  if (m_Property)
  {
    m_Property->RemoveObserver(m_ModifiedObserverTag);
    m_Property->RemoveObserver(m_DeleteObserverTag);
    m_Property = NULL;
  }
}


void PropertyObserver::OnModified(const itk::EventObject& e)
{
  PropertyChanged();
}


void PropertyObserver::OnDelete(const itk::EventObject& e)
{
  PropertyRemoved();
  if (m_Property)
  {
    m_Property->RemoveObserver(m_ModifiedObserverTag);
    m_Property->RemoveObserver(m_DeleteObserverTag);
    m_Property = NULL;
  }
}

  
}

