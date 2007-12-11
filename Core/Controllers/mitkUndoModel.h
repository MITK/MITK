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


#ifndef UNDOMODEL_H_HEADER_INCLUDED_C16ED098
#define UNDOMODEL_H_HEADER_INCLUDED_C16ED098

#include "mitkOperation.h"

namespace mitk {

class UndoStackItem; 
class OperationEvent;
class OperationActor;
  
//##ModelId=3E5F564C03D4
//##Documentation
//## @brief superclass for all UndoModels
//##
//## all necessary operations, that all UndoModels share.
//## @ingroup Undo
class UndoModel
{
  public:
  //##ModelId=3E95950F02B1
  UndoModel();

  virtual ~UndoModel();

  //##ModelId=3E5F5C6C00DF
  virtual bool SetOperationEvent(UndoStackItem* stackItem) = 0;

  //##ModelId=3E5F5C6C00F3
  virtual bool Undo() = 0;
  //##ModelId=3F045197000E
  virtual bool Undo(bool fine) = 0;

  //##ModelId=3E5F5C6C00FE
  virtual bool Redo() = 0;
  //##ModelId=3F045197002D
  virtual bool Redo(bool fine) = 0;

  //##ModelId=3F045197004D
  //##Documentation
  //## @brief clears undo and Redolist
  virtual void Clear() = 0;

  //##ModelId=3F045197005D
  //##Documentation
  //## @brief clears the RedoList
  virtual void ClearRedoList() = 0;

  //##ModelId=3F045197006D
  //##Documentation
  //## @brief true if RedoList is empty
  virtual bool RedoListEmpty() = 0;

  //##Documentation
  //## @brief returns the ObjectEventId of the 
  //## top Element in the OperationHistory of the selected 
  //## UndoModel
  virtual int GetLastObjectEventIdInList() = 0;

  //##Documentation
  //## @brief returns the GroupEventId of the 
  //## top Element in the OperationHistory of the selected 
  //## UndoModel
  virtual int GetLastGroupEventIdInList() = 0;

  //##Documentation
  //## @brief returns the last specified OperationEvent in Undo-list
  //## corresponding to the given values; if nothing found, then returns NULL
  //## 
  //## needed to get the old Position of an Element for declaring an UndoOperation
  virtual OperationEvent* GetLastOfType(OperationActor* destination, OperationType opType) = 0;
  
};

}// namespace mitk
#endif /* UNDOMODEL_H_HEADER_INCLUDED_C16ED098 */

