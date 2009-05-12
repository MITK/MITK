/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#include <mitkPropertyObserver.h>
#include <mitkBaseProperty.h>
#include <itkCommand.h>
#include <stdexcept>

namespace mitk {

//---- PropertyObserver -----------------------------------------------------------
PropertyObserver::PropertyObserver()
:m_SelfCall(false)
{
}

PropertyObserver::~PropertyObserver()
{
}

void PropertyObserver::BeginModifyProperty()
{
  m_SelfCall = true;
}

void PropertyObserver::EndModifyProperty()
{
  m_SelfCall = false;
}

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
    m_Property = NULL;
  }
}


void PropertyView::OnModified(const itk::EventObject& /*e*/)
{
  if (m_SelfCall) return;

  PropertyChanged();
}


void PropertyView::OnDelete(const itk::EventObject& /*e*/)
{
  if (m_SelfCall) return;

  PropertyRemoved();
  if (m_Property)
  {
    mitk::BaseProperty* prop = const_cast<mitk::BaseProperty*>(m_Property);
    prop->RemoveObserver(m_ModifiedTag);
    prop->RemoveObserver(m_DeleteTag);
    m_Property = NULL;
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


void PropertyEditor::OnModified(const itk::EventObject& /*e*/)
{
  if (m_SelfCall) return;

  PropertyChanged();
}


void PropertyEditor::OnDelete(const itk::EventObject& /*e*/)
{
  if (m_SelfCall) return; // does this make any sense?
  
  PropertyRemoved();
  if (m_Property)
  {
    m_Property->RemoveObserver(m_ModifiedTag);
    m_Property->RemoveObserver(m_DeleteTag);
    m_Property = NULL;
  }
}

} // namespace

