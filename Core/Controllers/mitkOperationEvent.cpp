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


#include "mitkOperationEvent.h"
#include <itkCommand.h>

int mitk::UndoStackItem::m_CurrObjectEventId = 0;
int mitk::UndoStackItem::m_CurrGroupEventId = 0;

bool mitk::UndoStackItem::m_IncrObjectEventId = false;
bool mitk::UndoStackItem::m_IncrGroupEventId = false;


mitk::UndoStackItem::UndoStackItem(std::string description)
: m_Reversed(false),
  m_Description(description)
{
  m_ObjectEventId = GetCurrObjectEventId();
  m_GroupEventId = GetCurrGroupEventId();
}

mitk::UndoStackItem::~UndoStackItem()
{
  // nothing to do
}

void mitk::UndoStackItem::ExecuteIncrement()
{
  if (m_IncrObjectEventId)
  {
    ++m_CurrObjectEventId;
    m_IncrObjectEventId = false;
  }
  
  if (m_IncrGroupEventId)
  {
    ++m_CurrGroupEventId;
    m_IncrGroupEventId = false;
  }
}

int mitk::UndoStackItem::GetCurrObjectEventId()
{
  return m_CurrObjectEventId;
}

int mitk::UndoStackItem::GetCurrGroupEventId()
{
  return m_CurrGroupEventId;
}

void mitk::UndoStackItem::IncCurrObjectEventId()
{
  m_IncrObjectEventId = true;
}

void mitk::UndoStackItem::IncCurrGroupEventId()
{
  m_IncrGroupEventId = true;
}

int mitk::UndoStackItem::GetObjectEventId()
{
  return m_ObjectEventId;
}

int mitk::UndoStackItem::GetGroupEventId()
{
  return m_GroupEventId;
}

std::string mitk::UndoStackItem::GetDescription()
{
  return m_Description;
}

void mitk::UndoStackItem::ReverseOperations()
{
  m_Reversed = !m_Reversed;
}

void mitk::UndoStackItem::ReverseAndExecute()
{
  ReverseOperations();
}

// ******************** mitk::OperationEvent ********************

mitk::Operation* mitk::OperationEvent::GetOperation()
{
  return m_Operation;
}

mitk::OperationEvent::OperationEvent(OperationActor* destination, 
                                     Operation* operation, Operation* undoOperation,
                                     std::string description)
: UndoStackItem(description), 
  m_Destination(destination),
  m_Operation(operation), 
  m_UndoOperation(undoOperation),
  m_Invalid(false)
{
  // nothing to do
  if (itk::Object* object = dynamic_cast<itk::Object*>( m_Destination ))
  {
    itk::SimpleMemberCommand< OperationEvent >::Pointer command = itk::SimpleMemberCommand< OperationEvent >::New();
    command->SetCallbackFunction( this, &OperationEvent::OnObjectDeleted );
    m_DeleteTag = object->AddObserver( itk::DeleteEvent(), command );
  }
}

mitk::OperationEvent::~OperationEvent()
{
  if (itk::Object* object = dynamic_cast<itk::Object*>( m_Destination ))
  {
    if (!m_Invalid)
    {
      object->RemoveObserver( m_DeleteTag );
    }
  }

  delete m_Operation;
  delete m_UndoOperation;
}

//##Documentation
//##  swaps the Undo and Redo- operation and changes m_Reversed
void mitk::OperationEvent::ReverseOperations()
{
  if (m_Operation == NULL) 
    return;

  Operation *tempOperation = m_Operation;
  m_Operation = m_UndoOperation;
  m_UndoOperation = tempOperation;

  UndoStackItem::ReverseOperations();
}

void mitk::OperationEvent::ReverseAndExecute()
{
  ReverseOperations();
  if (m_Destination && m_Operation)
    m_Destination->ExecuteOperation( m_Operation );
}

mitk::OperationActor* mitk::OperationEvent::GetDestination() 
{
  return m_Destination;
}

void mitk::OperationEvent::OnObjectDeleted()
{
  m_Invalid = true;
}

bool mitk::OperationEvent::IsValid()
{
  return !m_Invalid;
}
