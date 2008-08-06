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
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk {

class UndoStackItem; 
class OperationEvent;
class OperationActor;
  
//##Documentation
//## @brief superclass for all UndoModels
//##
//## all necessary operations, that all UndoModels share.
//## @ingroup Undo
class MITK_CORE_EXPORT UndoModel : public itk::Object
{
  public:
  mitkClassMacro(UndoModel, itk::Object);
  
  // no New Macro because this is an abstract class!

  virtual bool SetOperationEvent(UndoStackItem* stackItem) = 0;

  virtual bool Undo() = 0;
  virtual bool Undo(bool fine) = 0;

  virtual bool Redo() = 0;
  virtual bool Redo(bool fine) = 0;

  //##Documentation
  //## @brief clears undo and Redolist
  virtual void Clear() = 0;

  //##Documentation
  //## @brief clears the RedoList
  virtual void ClearRedoList() = 0;

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

protected:
  UndoModel(){};
  virtual ~UndoModel(){};

};

}// namespace mitk
#endif /* UNDOMODEL_H_HEADER_INCLUDED_C16ED098 */


