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

#ifndef UNDOMODEL_H_HEADER_INCLUDED_C16ED098
#define UNDOMODEL_H_HEADER_INCLUDED_C16ED098

#include "mitkCommon.h"
#include "mitkOperation.h"
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk
{
  class UndoStackItem;
  class OperationEvent;
  class OperationActor;

  //##Documentation
  //## @brief superclass for all UndoModels
  //##
  //## all necessary operations, that all UndoModels share.
  //## @ingroup Undo
  class MITKCORE_EXPORT UndoModel : public itk::Object
  {
  public:
    mitkClassMacroItkParent(UndoModel, itk::Object);

    // no New Macro because this is an abstract class!

    virtual bool SetOperationEvent(UndoStackItem *stackItem) = 0;

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
    //## @brief Gets the limit on the size of the undo history.
    //## The undo limit determines how many items can be stored
    //## in the undo stack. If the value is 0 that means that
    //## there is no limit.
    virtual std::size_t GetUndoLimit() const = 0;

    //##Documentation
    //## @brief Sets a limit on the size of the undo history.
    //## If the limit is reached, the oldest undo items will
    //## be dropped from the bottom of the undo stack.
    //## The 0 value means that there is no limit.
    //## @param limit the maximum number of items on the stack
    virtual void SetUndoLimit(std::size_t limit) = 0;

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
    //## corresponding to the given values; if nothing found, then returns nullptr
    //##
    //## needed to get the old Position of an Element for declaring an UndoOperation
    virtual OperationEvent *GetLastOfType(OperationActor *destination, OperationType opType) = 0;

  protected:
    UndoModel(){};
    ~UndoModel() override{};
  };

} // namespace mitk
#endif /* UNDOMODEL_H_HEADER_INCLUDED_C16ED098 */
