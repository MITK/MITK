/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "QmitkFunctionality.h"

#include <qlcdnumber.h>
#include <qslider.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <stdexcept>
#include <qcursor.h>
#include <itkCommand.h>
#include <QmitkPropertyListView.h>

QmitkFunctionality::QmitkFunctionality(QObject *parent, const char *name, mitk::DataTreeIteratorBase* dataIt) : 
QmitkBaseFunctionalityComponent(parent, name, dataIt), m_Available(false), m_Activated(false), m_DataTreeIterator(NULL), m_TreeChangedWhileInActive(false), m_InTreeChanged(false), m_ObserverTag(0)
{
  std::cout << "Instantiating QmitkFunctionality. QObject::name(): " << this->name() << std::endl;
  SetDataTree(dataIt);
  m_Options = mitk::PropertyList::New();
}

QmitkFunctionality::~QmitkFunctionality()
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
}

QString QmitkFunctionality::GetFunctionalityName()
{
  return name();
}

void QmitkFunctionality::Activated()
{
  m_Activated = true;
  if(m_TreeChangedWhileInActive)
  {
    if(IsInTreeChanged()==false)
    {
      m_InTreeChanged = true;
      TreeChanged();
      m_InTreeChanged = false;
    }
    m_TreeChangedWhileInActive = false;
  }
}

void QmitkFunctionality::Deactivated()
{
  m_Activated = false;
}

void QmitkFunctionality::Reinitialize()
{
}

bool QmitkFunctionality::IsActivated()
{
  return m_Activated;
}

bool QmitkFunctionality::IsAvailable()
{
  return m_Available;
}

bool QmitkFunctionality::IsInTreeChanged() const
{
  return m_InTreeChanged;
}

void QmitkFunctionality::SetAvailability(bool available)
{
  this->m_Available=available;
  emit AvailabilityChanged(this);
  emit AvailabilityChanged();
}

void QmitkFunctionality::SetDataTree(mitk::DataTreeIteratorBase* it)
{
  if(m_DataTreeIterator.IsNotNull() )
  {
    m_DataTreeIterator->GetTree()->RemoveObserver(m_ObserverTag);
  }
  m_DataTreeIterator = it;
  if(m_DataTreeIterator.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkFunctionality>::Pointer command = itk::ReceptorMemberCommand<QmitkFunctionality>::New();
    command->SetCallbackFunction(this, &QmitkFunctionality::TreeChanged);
    m_ObserverTag = m_DataTreeIterator->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  }
}

mitk::DataTreeIteratorBase* QmitkFunctionality::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

void QmitkFunctionality::TreeChanged(const itk::EventObject & /*treeChangedEvent*/)
{
  if(IsActivated())
  {
    m_TreeChangedWhileInActive = false;
    if(IsInTreeChanged()==false)
    {
      m_InTreeChanged = true;
      TreeChanged();
      m_InTreeChanged = false;
    }
  }
  else
    m_TreeChangedWhileInActive = true;
}

void QmitkFunctionality::TreeChanged()
{
}

QWidget * QmitkFunctionality::CreateOptionWidget(QWidget* parent)
{
  QmitkPropertyListView* dialog = new QmitkPropertyListView(parent);
  dialog->SetPropertyList(this->GetFunctionalityOptionsList());
  return dialog;
}

void QmitkFunctionality::OptionsChanged(QWidget* itkNotUsed(optionDialog))
{
  // Read new values from your option dialog and update your functionality accordingly
}

mitk::PropertyList* QmitkFunctionality::GetFunctionalityOptionsList()
{
  return m_Options.GetPointer();
}

void QmitkFunctionality::SetFunctionalityOptionsList(mitk::PropertyList* pl)
{
  if (pl)
  {
    if ( m_Options.IsNull() )
      m_Options = mitk::PropertyList::New();
    m_Options = pl->Clone();
  }
  
  // more meaningful implementation, i.e. reaction to property values, to be done in sub-classes
}



void QmitkFunctionality::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}



void QmitkFunctionality::WaitCursorOff()
{
  QApplication::restoreOverrideCursor();
}

void QmitkFunctionality::HandleException( const char* str, QWidget* parent, bool showDialog ) const
{
  itkGenericOutputMacro( << "Exception caught: " << str );
  if ( showDialog )
  {
    QMessageBox::critical ( parent, "Exception caught!", str );
  }
}

void QmitkFunctionality::HandleException( std::exception& e, QWidget* parent, bool showDialog ) const
{
  HandleException( e.what(), parent, showDialog );
}
  
bool QmitkFunctionality::TestYourself()
{
  std::cout << "NO TEST IMPLEMENTED : ";
  return true;
}
