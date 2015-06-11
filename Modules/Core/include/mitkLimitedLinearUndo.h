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


#ifndef LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96
#define LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96

// MITK header
#include <MitkCoreExports.h>
#include "mitkOperationEvent.h"
#include "mitkUndoModel.h"
// STL header
#include <vector>
#include <deque>
// ITK header
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk {

const unsigned int MIN_DEQUE_SIZE = 10;
const unsigned int MAX_DEQUE_SIZE = 10000;

//##Documentation
//## @brief A linear undo model with one undo and one redo stack.
//##
//## Derived from UndoModel AND itk::Object. Invokes ITK-events to signal listening
//## GUI elements, whether each of the stacks is empty or not (to enable/disable button, ...)
class MITKCORE_EXPORT LimitedLinearUndo : public UndoModel
{
public:
  typedef std::deque<UndoStackItem*> UndoContainer;
  typedef std::deque<UndoStackItem*>::reverse_iterator UndoContainerRevIter;

  static void setDequeSize(unsigned int size);
  static unsigned int getDequeSize();

  mitkClassMacro(LimitedLinearUndo, UndoModel);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual bool SetOperationEvent(UndoStackItem* stackItem) override;

  //##Documentation
  //## @brief Undoes the last changes
  //##
  //##  Reads the top element of the Undo-Stack,
  //##  executes the operation,
  //##  swaps the OperationEvent-Undo with the Operation
  //##  and sets it to Redo-Stack
  virtual bool Undo() override;
  virtual bool Undo(bool) override;

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
  virtual bool Redo() override;
  virtual bool Redo(bool) override;

  //##Documentation
  //## @brief Redoes all changes until ObjectEventID oeid
  virtual bool Redo(int oeid);

  //##Documentation
  //## @brief Clears UndoList and RedoList
  virtual void Clear() override;

  //##Documentation
  //## @brief Clears the RedoList
  virtual void ClearRedoList() override;

  //##Documentation
  //## @brief True, if RedoList is empty
  virtual bool RedoListEmpty() override;

  //##Documentation
  //## @brief Returns the ObjectEventId of the
  //## top element in the OperationHistory
  virtual int GetLastObjectEventIdInList() override;

  //##Documentation
  //## @brief Returns the GroupEventId of the
  //## top element in the OperationHistory
  virtual int GetLastGroupEventIdInList() override;

  //##Documentation
  //## @brief Returns the last specified OperationEvent in Undo-list
  //## corresponding to the given values; if nothing found, then returns NULL
  virtual OperationEvent* GetLastOfType(OperationActor* destination, OperationType opType) override;

protected:
  //##Documentation
  //## Constructor
  LimitedLinearUndo();

  //##Documentation
  //## Destructor
  virtual ~LimitedLinearUndo();

  //## @brief Convenience method to free the memory of
  //## elements in the list and to clear the list
  void ClearList(UndoContainer* list);

  UndoContainer m_UndoList;

  UndoContainer m_RedoList;

private:
  int FirstObjectEventIdOfCurrentGroup(UndoContainer& stack);

  static unsigned int m_dequeSize;

};

#pragma GCC visibility push(default)

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

#pragma GCC visibility pop

} //namespace mitk

#endif /* LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96 */
