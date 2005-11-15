
#include <mitkPropertyObserver.h>
#include <itkCommand.h>
#include <exception>

namespace mitk {

//---- PropertyView ---------------------------------------------------------------

PropertyView::PropertyView( const mitk::BaseProperty* property )
:m_Property(property)
{
  if ( !property ) throw std::invalid_argument("NULL pointer makes no sense in PropertyView()"); // no NULL property allowed

  {
    itk::ReceptorMemberCommand<PropertyView>::Pointer command = itk::ReceptorMemberCommand<PropertyView>::New();
    command->SetCallbackFunction(this, &PropertyView::OnModified);
    m_ModifiedTag = property->AddObserver( itk::ModifiedEvent(), command );
  }
    {
    itk::ReceptorMemberCommand<PropertyView>::Pointer command = itk::ReceptorMemberCommand<PropertyView>::New();
    command->SetCallbackFunction(this, &PropertyView::OnDelete);
    m_DeleteTag = property->AddObserver( itk::DeleteEvent(), command );
  }
}


PropertyView::~PropertyView()
{
  if (m_Property)
  {
    mitk::BaseProperty* prop = const_cast<mitk::BaseProperty*>(m_Property);
    prop->RemoveObserver(m_ModifiedTag);
    prop->RemoveObserver(m_DeleteTag);
    const_cast<mitk::BaseProperty*>(m_Property) = NULL;
  }
}


void PropertyView::OnModified(const itk::EventObject& e)
{
  PropertyChanged();
}


void PropertyView::OnDelete(const itk::EventObject& e)
{
  PropertyRemoved();
  if (m_Property)
  {
    mitk::BaseProperty* prop = const_cast<mitk::BaseProperty*>(m_Property);
    prop->RemoveObserver(m_ModifiedTag);
    prop->RemoveObserver(m_DeleteTag);
    const_cast<mitk::BaseProperty*>(m_Property) = NULL;
  }
}
    
void PropertyView::BeginModifyProperty()
{
  if (m_Property)
  {
    mitk::BaseProperty* prop = const_cast<mitk::BaseProperty*>(m_Property);
    prop->RemoveObserver(m_ModifiedTag);
    prop->RemoveObserver(m_DeleteTag);
  }
}

void PropertyView::EndModifyProperty()
{
  if (m_Property)
  {
    {
      itk::ReceptorMemberCommand<PropertyView>::Pointer command = itk::ReceptorMemberCommand<PropertyView>::New();
      command->SetCallbackFunction(this, &PropertyView::OnModified);
      m_ModifiedTag = m_Property->AddObserver( itk::ModifiedEvent(), command );
    }
    {
      itk::ReceptorMemberCommand<PropertyView>::Pointer command = itk::ReceptorMemberCommand<PropertyView>::New();
      command->SetCallbackFunction(this, &PropertyView::OnDelete);
      m_DeleteTag = m_Property->AddObserver( itk::DeleteEvent(), command );
    }
  }
}

//---- PropertyEditor -------------------------------------------------------------

PropertyEditor::PropertyEditor( mitk::BaseProperty* property )
:m_Property(property)
{
  if ( !property ) throw std::invalid_argument("NULL pointer makes no sense in PropertyEditor()"); // no NULL property allowed

  {
    itk::ReceptorMemberCommand<PropertyEditor>::Pointer command = itk::ReceptorMemberCommand<PropertyEditor>::New();
    command->SetCallbackFunction(this, &PropertyEditor::OnModified);
    m_ModifiedTag = property->AddObserver( itk::ModifiedEvent(), command );
  }
  {
    itk::ReceptorMemberCommand<PropertyEditor>::Pointer command = itk::ReceptorMemberCommand<PropertyEditor>::New();
    command->SetCallbackFunction(this, &PropertyEditor::OnDelete);
    m_DeleteTag = property->AddObserver( itk::DeleteEvent(), command );
  }
}


PropertyEditor::~PropertyEditor()
{
  if (m_Property)
  {
    m_Property->RemoveObserver(m_ModifiedTag);
    m_Property->RemoveObserver(m_DeleteTag);
    m_Property = NULL;
  }
}


void PropertyEditor::OnModified(const itk::EventObject& e)
{
  PropertyChanged();
}


void PropertyEditor::OnDelete(const itk::EventObject& e)
{
  PropertyRemoved();
  if (m_Property)
  {
    m_Property->RemoveObserver(m_ModifiedTag);
    m_Property->RemoveObserver(m_DeleteTag);
    m_Property = NULL;
  }
}

void PropertyEditor::BeginModifyProperty()
{
  if (m_Property)
  {
    m_Property->RemoveObserver(m_ModifiedTag);
    m_Property->RemoveObserver(m_DeleteTag);
  }
}

void PropertyEditor::EndModifyProperty()
{
  if (m_Property)
  {
    {
        itk::ReceptorMemberCommand<PropertyEditor>::Pointer command = itk::ReceptorMemberCommand<PropertyEditor>::New();
      command->SetCallbackFunction(this, &PropertyEditor::OnModified);
      m_ModifiedTag = m_Property->AddObserver( itk::ModifiedEvent(), command );
    }
    {
      itk::ReceptorMemberCommand<PropertyEditor>::Pointer command = itk::ReceptorMemberCommand<PropertyEditor>::New();
      command->SetCallbackFunction(this, &PropertyEditor::OnDelete);
      m_DeleteTag = m_Property->AddObserver( itk::DeleteEvent(), command );
    }
  }
}

} // namespace

