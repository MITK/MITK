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

//##ModelId=3E9B07B40374
int mitk::UndoStackItem::m_CurrObjectEventId = 0;
//##ModelId=3E9B07B5002B
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

//##ModelId=3E9B07B501A7
int mitk::UndoStackItem::GetCurrObjectEventId()
{
  return m_CurrObjectEventId;
}

//##ModelId=3E9B07B50220
int mitk::UndoStackItem::GetCurrGroupEventId()
{
  return m_CurrGroupEventId;
}

//##ModelId=3EF099E90289
void mitk::UndoStackItem::IncCurrObjectEventId()
{
  m_IncrObjectEventId = true;
}

//##ModelId=3EF099E90269
void mitk::UndoStackItem::IncCurrGroupEventId()
{
  m_IncrGroupEventId = true;
}

//##ModelId=3EF099E90259
int mitk::UndoStackItem::GetObjectEventId()
{
  return m_ObjectEventId;
}

//##ModelId=3EF099E90249
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

//##ModelId=3E5F610D00BB
mitk::Operation* mitk::OperationEvent::GetOperation()
{
  return m_Operation;
}

//##ModelId=3E957AE700E6
mitk::OperationEvent::OperationEvent(OperationActor* destination, 
                                     Operation* operation, Operation* undoOperation,
                                     std::string description)
: UndoStackItem(description), 
  m_Destination(destination),
  m_Operation(operation), m_UndoOperation(undoOperation) 
{
  // nothing to do
}

//##ModelId=3F0451960212
mitk::OperationEvent::~OperationEvent()
{
  // nothing to do
}

//##ModelId=3E957C1102E3
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
  m_Destination->ExecuteOperation( m_Operation );
}

//##ModelId=3E9B07B502AC
mitk::OperationActor* mitk::OperationEvent::GetDestination() 
{
  return m_Destination;
}


