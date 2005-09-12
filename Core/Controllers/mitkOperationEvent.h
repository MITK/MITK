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


#ifndef OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC
#define OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC

#include "mitkCommon.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkUndoModel.h"
#include <string>
#include <list>

namespace mitk {

//##Documentation
//## @brief Represents an entry of the undo or redo stack.
//## @ingroup Undo
//## This basic entry includes a textual description of the item and a pair of IDs. Static
//## member functions handle creation and incrementing of these IDs. 
//##
//## The ObjectEventID is increased by the global EventMapper for most of the events (see
//## code for details). Incrementation of the IDs is done in two steps. First the
//## EventMapper sets a flag via (possibly multiple calls of) IncCurrObjectEventID(), then 
//## ExecuteIncrement() does the actual incementation.
//## 
//## The GroupEventID is intended for logical grouping of several related Operations.
//## Currently this is used only by PointSetInteractor. How this is done and when to use
//## GroupEventIDs is still undocumented.
class UndoStackItem
{
  public:
    UndoStackItem(std::string description = "");

    virtual ~UndoStackItem();
  
    //##ModelId=3E9B07B50220
    //##Documentation
    //## @brief For combining operations in groups
    //##
    //## This ID is used in the undo mechanism.
    //## For separation of the seperate operations
    //## If the GroupEventId of two OperationEvents is equal, 
    //## then they share one group and will be undone in case of Undo(fine==false)
    static int GetCurrGroupEventId();

    //##ModelId=3E9B07B501A7
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

    //##ModelId=3EF099E90249
    //##Documentation
    //## @brief Returns the GroupEventId for this object
    int GetGroupEventId();

    //##ModelId=3EF099E90259
    //##Documentation
    //## @brief Returns the ObjectEventId for this object
    int GetObjectEventId();
  
    //##NoModelId
    //##Documentation
    //## @brief Returns the textual description of this object
    std::string GetDescription();

    friend class StateMachine; //for IncCurrGroupEventId
    friend class EventMapper;  //for IncCurrObjectEventId
    
    virtual void ReverseOperations();
    virtual void ReverseAndExecute();

  protected:
    //##ModelId=3E9B07B500D5
    //##Documentation
    //## @brief true, if operation and undooperation have been swaped/changed
    bool m_Reversed;

    //##Documentation
    //## @brief Executes the incrementation of objectEventId and groupEventId if they are set to be incremented
    static void ExecuteIncrement();

    //##ModelId=3EF099E90289
    //##Documentation
    //## @brief Sets the current ObjectEventId to be incremended when ExecuteIncrement is called
    static void IncCurrObjectEventId();

    //##ModelId=3EF099E90269
    //##Documentation
    //## @brief Sets the current GroupEventId to be incremended when ExecuteIncrement is called
    static void IncCurrGroupEventId();
  
  private:
    //##ModelId=3E9B07B40374
    static int m_CurrObjectEventId;

    //##ModelId=3E9B07B5002B
    static int m_CurrGroupEventId;
  
    static bool m_IncrObjectEventId;
  
    static bool m_IncrGroupEventId;
  
    //##ModelId=3E7F488E022B
    int m_ObjectEventId;

    //##ModelId=3E7F48C60335
    int m_GroupEventId;
  
    std::string m_Description;
    
    UndoStackItem(UndoStackItem&);              // hide copy constructor
    void operator=(const UndoStackItem&);       // hide operator=

};

//##ModelId=3E5F60F301A4
//##Documentation
//## @brief Represents a pair of operations: undo and the according redo.
//## @ingroup Undo
//## Additionally to the base class UndoStackItem, which only provides a description of an
//## item, OperationEvent does the actual accounting of the undo/redo stack. This class
//## holds two Operation objects (operation and its inverse operation) and the corresponding 
//## OperationActor. The operations may be swapped, which is done by the
//## LimitedLinearUndo models, when an OperationEvent is moved from the undo to the redo
//## stack or vice versa.
class OperationEvent : public UndoStackItem
{
public:
  //##ModelId=3E957AE700E6
  //OperationEvent(OperationActor* destination, Operation* operation, Operation* undoOperation, int objectEventId, int groupEventId );
  OperationEvent(OperationActor* destination, Operation* operation, Operation* undoOperation, std::string description = "" );

  //##ModelId=3F0451960212
  virtual ~OperationEvent();

  //##ModelId=3E5F610D00BB
  Operation* GetOperation();

  //##ModelId=3E9B07B502AC
  //## @brief Returns the destination of the operations
  OperationActor* GetDestination();

  friend class UndoModel;

  //##ModelId=3E957C1102E3
  //##Documentation
  //## @brief Swaps the two operations and sets a flag, 
  //## that it has been swapped and do is undo and undo is do
  virtual void ReverseOperations();
  virtual void ReverseAndExecute();           /// reverses and executes both operations (used, when moved from undo to redo stack)
protected:

private:
  //##ModelId=3E5F61DB00D6
  OperationActor* m_Destination;

  //##ModelId=3E5F6B1E0107
  Operation* m_Operation;

  //##ModelId=3E5F6B2E0060
  Operation* m_UndoOperation;

  OperationEvent(OperationEvent&);             // hide copy constructor
  void operator=(const OperationEvent&);       // hide operator=
};

} //namespace mitk

#endif /* OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC */

