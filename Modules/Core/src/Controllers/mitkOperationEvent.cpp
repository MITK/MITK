/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkOperationEvent.h"
#include <itkCommand.h>

int mitk::UndoStackItem::m_CurrObjectEventId = 0;
int mitk::UndoStackItem::m_CurrGroupEventId = 0;

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
  ++m_CurrObjectEventId;
}

void mitk::UndoStackItem::IncCurrGroupEventId()
{
  ++m_CurrGroupEventId;
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
                                     Operation* operation,
                                     Operation* undoOperation,
                                     std::string description,
                                     mitk::DataNode* parentNode)
: UndoStackItem(description),
  m_Destination(destination),
  m_Operation(operation),
  m_UndoOperation(undoOperation),
  m_Invalid(false),
  m_ParentNode(parentNode)
{
  //connect to delete event
  if (itk::Object* object = dynamic_cast<itk::Object*>( m_Destination ))
  {
    itk::SimpleMemberCommand< OperationEvent >::Pointer command = itk::SimpleMemberCommand< OperationEvent >::New();
    command->SetCallbackFunction( this, &OperationEvent::OnObjectDeleted );
    m_DeleteTag = object->AddObserver( itk::DeleteEvent(), command );
  }
}

mitk::OperationEvent::~OperationEvent()
{
  //remove the observer if the data m_Destination still is present
  if (!m_Invalid)
  {
    if (itk::Object* object = dynamic_cast<itk::Object*>( m_Destination ))
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
  if (m_Operation == nullptr)
    return;

  Operation *tempOperation = m_Operation;
  m_Operation = m_UndoOperation;
  m_UndoOperation = tempOperation;

  UndoStackItem::ReverseOperations();
}

void mitk::OperationEvent::ReverseAndExecute()
{
  ReverseOperations();
  if (m_Destination && m_Operation && !m_Invalid)
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
