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


#ifndef OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC
#define OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC

#include <MitkCoreExports.h>
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkUndoModel.h"
#include <string>
#include <list>

namespace mitk {

//##Documentation
//## @brief Represents an entry of the undo or redo stack.
//##
//## This basic entry includes a textual description of the item and a pair of IDs. Static
//## member functions handle creation and incrementing of these IDs.
//##
//## The ObjectEventID is increased by the global EventMapper for most of the events (see
//## code for details). Incrementation of the IDs is done in two steps. First the
//## EventMapper sets a flag via (possibly multiple calls of) IncCurrObjectEventID(), then
//## ExecuteIncrement() does the actual incementation.
//##
//## The GroupEventID is intended for logical grouping of several related Operations.
//## Currently this is used only by PointSetDataInteractor. How this is done and when to use
//## GroupEventIDs is still undocumented.
//## @ingroup Undo
class MITK_CORE_EXPORT UndoStackItem
{
  public:
    UndoStackItem(std::string description = "");

    virtual ~UndoStackItem();

    //##Documentation
    //## @brief For combining operations in groups
    //##
    //## This ID is used in the undo mechanism.
    //## For separation of the seperate operations
    //## If the GroupEventId of two OperationEvents is equal,
    //## then they share one group and will be undone in case of Undo(fine==false)
    static int GetCurrGroupEventId();

    //##Documentation
    //## @brief For combining operations in Objects
    //##
    //## This ID is used in the Undo-Mechanism.
    //## For separation of the seperate operations
    //## If the ObjectEventId of two OperationEvents is equal,
    //## then they share one Object and will be undone in all cases of Undo(true and false).
    //## they shal not be seperated, because they were produced to realize one object-change.
    //## for example: OE_statechange and OE_addlastpoint
    static int GetCurrObjectEventId();

    //##Documentation
    //## @brief Returns the GroupEventId for this object
    int GetGroupEventId();

    //##Documentation
    //## @brief Returns the ObjectEventId for this object
    int GetObjectEventId();

    //##Documentation
    //## @brief Returns the textual description of this object
    std::string GetDescription();

    virtual void ReverseOperations();
    virtual void ReverseAndExecute();

    //##Documentation
    //## @brief Sets the current ObjectEventId to be incremended when ExecuteIncrement is called
    //## For example if a button click generates operations the ObjectEventId has to be incremented to be able to undo the operations.
    //## Difference between ObjectEventId and GroupEventId: The ObjectEventId capsulates all operations caused by one event.
    //## A GroupEventId capsulates several ObjectEventIds so that several operations caused by several events can be undone with one Undo call.
    static void IncCurrObjectEventId();

    //##Documentation
    //## @brief Sets the current GroupEventId to be incremended when ExecuteIncrement is called
    //## For example if a button click generates operations the GroupEventId has to be incremented to be able to undo the operations.
    //## Difference between ObjectEventId and GroupEventId: The ObjectEventId capsulates all operations caused by one event.
    //## A GroupEventId capsulates several ObjectEventIds so that several operations caused by several events can be undone with one Undo call.
    static void IncCurrGroupEventId();

    //##Documentation
    //## @brief Executes the incrementation of objectEventId and groupEventId if they are set to be incremented
    static void ExecuteIncrement();

  protected:
    //##Documentation
    //## @brief true, if operation and undooperation have been swaped/changed
    bool m_Reversed;

  private:
    static int m_CurrObjectEventId;

    static int m_CurrGroupEventId;

    static bool m_IncrObjectEventId;

    static bool m_IncrGroupEventId;

    int m_ObjectEventId;

    int m_GroupEventId;

    std::string m_Description;

    UndoStackItem(UndoStackItem&);              // hide copy constructor
    void operator=(const UndoStackItem&);       // hide operator=

};

//##Documentation
//## @brief Represents a pair of operations: undo and the according redo.
//##
//## Additionally to the base class UndoStackItem, which only provides a description of an
//## item, OperationEvent does the actual accounting of the undo/redo stack. This class
//## holds two Operation objects (operation and its inverse operation) and the corresponding
//## OperationActor. The operations may be swapped by the
//## undo models, when an OperationEvent is moved from their undo to their redo
//## stack or vice versa.
//##
//## Note, that memory management of operation and undooperation is done by this class.
//## Memory of both objects is freed in the destructor. For this, the method IsValid() is needed which holds
//## information of the state of m_Destination. In case the object referenced by m_Destination is already deleted,
//## isValid() returns false.
//## In more detail if the destination happens to be an itk::Object (often the case), OperationEvent is informed as soon
//## as the object is deleted - from this moment on the OperationEvent gets invalid. You should
//## check this flag before you call anything on destination
//##
//## @ingroup Undo
class MITK_CORE_EXPORT OperationEvent : public UndoStackItem
{
public:
  //## @brief default constructor
  OperationEvent(OperationActor* destination, Operation* operation, Operation* undoOperation, std::string description = "" );

  //## @brief default destructor
  //##
  //## removes observers if destination is valid
  //## and frees memory referenced by m_Operation and m_UndoOperation
  virtual ~OperationEvent();

  //## @brief Returns the operation
  Operation* GetOperation();

  //## @brief Returns the destination of the operations
  OperationActor* GetDestination();

  friend class UndoModel;

  //## @brief Swaps the two operations and sets a flag,
  //## that it has been swapped and doOp is undoOp and undoOp is doOp
  virtual void ReverseOperations();

  //##reverses and executes both operations (used, when moved from undo to redo stack)
  virtual void ReverseAndExecute();

  //## @brief returns true if the destination still is present
  //## and false if it already has been deleted
  virtual bool IsValid();

protected:

  void OnObjectDeleted();

private:

  // Has to be observed for itk::DeleteEvents.
  // When destination is deleted, this stack item is invalid!
  OperationActor* m_Destination;

  //## reference to the operation
  Operation* m_Operation;

  //## reference to the undo operation
  Operation* m_UndoOperation;

  //## hide copy constructor
  OperationEvent(OperationEvent&);
  //## hide operator=
  void operator=(const OperationEvent&);

  //observertag used to listen to m_Destination
  unsigned long m_DeleteTag;

  //## stores if destination is valid or already has been freed
  bool m_Invalid;
};

} //namespace mitk

#endif /* OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC */
