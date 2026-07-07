/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUndoModel_h
#define mitkUndoModel_h

#include <mitkCommon.h>
#include <mitkOperation.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk
{
  class UndoStackItem;
  class OperationEvent;
  class OperationActor;

  /**
   * \brief Abstract superclass for all undo models.
   *
   * Defines the interface that all UndoModel implementations must provide,
   * including undo/redo operations, stack management, and history queries.
   *
   * \ingroup Undo
   */
  class MITKCORE_EXPORT UndoModel : public itk::Object
  {
  public:
    mitkClassMacroItkParent(UndoModel, itk::Object);

    // no New Macro because this is an abstract class!

    /**
     * \brief Store an operation event on the undo stack.
     *
     * \param[in] stackItem The undo stack item to store.
     * \return True if the item was stored successfully.
     */
    virtual bool SetOperationEvent(UndoStackItem *stackItem) = 0;

    /**
     * \brief Undo the last operation (fine-grained by ObjectEventId).
     * \return True if the undo was performed successfully.
     */
    virtual bool Undo() = 0;

    /**
     * \brief Undo the last operation with control over granularity.
     *
     * \param[in] fine If true, undo by ObjectEventId; if false, undo by GroupEventId.
     * \return True if the undo was performed successfully.
     */
    virtual bool Undo(bool fine) = 0;

    /**
     * \brief Redo the last undone operation (fine-grained by ObjectEventId).
     * \return True if the redo was performed successfully.
     */
    virtual bool Redo() = 0;

    /**
     * \brief Redo the last undone operation with control over granularity.
     *
     * \param[in] fine If true, redo by ObjectEventId; if false, redo by GroupEventId.
     * \return True if the redo was performed successfully.
     */
    virtual bool Redo(bool fine) = 0;

    /** \brief Clear both the undo and redo lists. */
    virtual void Clear() = 0;

    /** \brief Clear the redo list only. */
    virtual void ClearRedoList() = 0;

    /**
     * \brief Check whether the redo list is empty.
     * \return True if the redo list contains no entries.
     */
    virtual bool RedoListEmpty() = 0;

    /**
     * \brief Check whether the undo list is empty.
     * \return True if the undo list contains no entries.
     */
    virtual bool UndoListEmpty() = 0;

    /**
     * \brief Get the limit on the size of the undo history.
     *
     * The undo limit determines how many items can be stored
     * in the undo stack. A value of 0 means there is no limit.
     *
     * \return The maximum number of items on the undo stack, or 0 for unlimited.
     */
    virtual std::size_t GetUndoLimit() const = 0;

    /**
     * \brief Set a limit on the size of the undo history.
     *
     * If the limit is reached, the oldest undo items will
     * be dropped from the bottom of the undo stack.
     * A value of 0 means there is no limit.
     *
     * \param[in] limit The maximum number of items on the stack.
     */
    virtual void SetUndoLimit(std::size_t limit) = 0;

    /**
     * \brief Return the ObjectEventId of the top element in the undo history.
     * \return The ObjectEventId of the most recent undo stack entry.
     */
    virtual int GetLastObjectEventIdInList() = 0;

    /**
     * \brief Return the GroupEventId of the top element in the undo history.
     * \return The GroupEventId of the most recent undo stack entry.
     */
    virtual int GetLastGroupEventIdInList() = 0;

    /**
     * \brief Return the last OperationEvent matching the given destination and type.
     *
     * Useful for retrieving the old position of an element when declaring
     * an UndoOperation.
     *
     * \param[in] destination The target OperationActor.
     * \param[in] opType The operation type to search for.
     * \return The matching OperationEvent, or nullptr if not found.
     */
    virtual OperationEvent *GetLastOfType(OperationActor *destination, OperationType opType) = 0;

    /**
     * \brief Remove invalid operations from the undo/redo stack.
     *
     * Iterates through all stored OperationEvents and removes those
     * that are no longer valid.
     *
     * \return The number of invalid operations removed.
     */
    virtual unsigned int RemoveInvalidOperations() = 0;

  protected:
    UndoModel(){};
    ~UndoModel() override{};
  };

} // namespace mitk
#endif
