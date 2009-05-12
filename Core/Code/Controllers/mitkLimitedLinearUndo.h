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


#ifndef LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96
#define LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96

// MITK header
#include "mitkCommon.h"
#include "mitkOperationEvent.h"
#include "mitkUndoModel.h"
// STL header
#include <vector>
// ITK header
#include<itkEventObject.h>

namespace mitk {

//##Documentation
//## @brief A linear undo model with one undo and one redo stack.
//##
//## Derived from UndoModel AND itk::Object. Invokes ITK-events to signal listening
//## GUI elements, whether each of the stacks is empty or not (to enable/disable button, ...)
class MITK_CORE_EXPORT LimitedLinearUndo : public UndoModel
{
public:
  typedef std::vector<UndoStackItem*> UndoContainer;
  typedef std::vector<UndoStackItem*>::reverse_iterator UndoContainerRevIter;
  
  mitkClassMacro(LimitedLinearUndo, UndoModel);
  itkNewMacro(Self);

  virtual bool SetOperationEvent(UndoStackItem* stackItem);

  //##Documentation
  //## @brief Undoes the last changes
  //##
  //##  Reads the top element of the Undo-Stack,  
  //##  executes the operation,
  //##  swaps the OperationEvent-Undo with the Operation
  //##  and sets it to Redo-Stack
  virtual bool Undo();
  virtual bool Undo(bool);

  //##Documentation
  //## @brief Undoes all changes until ObjectEventID oeid
  virtual bool Undo(int oeid);

  //##Documentation
  //## @brief Undoes the last changes
  //##
  //## Reads the top element of the Redo-Stack, 
  //## executes the operation,
  //## swaps the OperationEvent-Operation with the Undo-Operation
  //## and sets it to Undo-Stack
  virtual bool Redo();
  virtual bool Redo(bool);

  //##Documentation
  //## @brief Redoes all changes until ObjectEventID oeid
  virtual bool Redo(int oeid);

  //##Documentation
  //## @brief Clears UndoList and RedoList
  virtual void Clear();

  //##Documentation
  //## @brief Clears the RedoList
  virtual void ClearRedoList();

  //##Documentation
  //## @brief True, if RedoList is empty
  virtual bool RedoListEmpty();

  //##Documentation
  //## @brief Returns the ObjectEventId of the 
  //## top element in the OperationHistory
  virtual int GetLastObjectEventIdInList();

  //##Documentation
  //## @brief Returns the GroupEventId of the 
  //## top element in the OperationHistory
  virtual int GetLastGroupEventIdInList();
  
  //##Documentation
  //## @brief Returns the last specified OperationEvent in Undo-list
  //## corresponding to the given values; if nothing found, then returns NULL
  virtual OperationEvent* GetLastOfType(OperationActor* destination, OperationType opType);

protected:
  //##Documentation
  //## Constructor
  LimitedLinearUndo();

  //##Documentation
  //## Destructor
  virtual ~LimitedLinearUndo();

  UndoContainer m_UndoList;

  UndoContainer m_RedoList;

private:
  int FirstObjectEventIdOfCurrentGroup(UndoContainer& stack);

};

/// Some itk events to notify listening GUI elements, when the undo or redo stack is empty (diable undo button)
/// or when there are items in the stack (enable button)
itkEventMacro( UndoStackEvent,     itk::ModifiedEvent );
itkEventMacro( UndoEmptyEvent,     UndoStackEvent );
itkEventMacro( RedoEmptyEvent,     UndoStackEvent );
itkEventMacro( UndoNotEmptyEvent,  UndoStackEvent );
itkEventMacro( RedoNotEmptyEvent,  UndoStackEvent );
/// Additional unused events, if anybody wants to put an artificial limit to the possible number of items in the stack
itkEventMacro( UndoFullEvent,      UndoStackEvent );
itkEventMacro( RedoFullEvent,      UndoStackEvent );

} //namespace mitk

#endif /* LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96 */
