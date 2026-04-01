/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOperationEvent_h
#define mitkOperationEvent_h

#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkUndoModel.h>

#include <mitkITKEventObserverGuard.h>

#include <list>
#include <string>


#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Represents an entry on the undo or redo stack.
   *
   * This base class stores a textual description and a pair of IDs
   * (ObjectEventId and GroupEventId) for grouping related operations.
   * Static member functions manage creation and incrementing of these IDs.
   *
   * The GroupEventId is intended for logical grouping of several related
   * operations so they can be undone together with a single Undo(false) call.
   *
   * \sa OperationEvent, UndoController, UndoModel
   * \ingroup Undo
   */
  class MITKCORE_EXPORT UndoStackItem
  {
  public:
    /**
     * \brief Construct an UndoStackItem with an optional description.
     * \param[in] description Human-readable description of this undo entry.
     */
    UndoStackItem(std::string description = "");

    /** \brief Virtual destructor. */
    virtual ~UndoStackItem();

    /**
     * \brief Get the current global GroupEventId.
     *
     * Operations sharing the same GroupEventId are undone together when
     * Undo(false) is called.
     *
     * \return The current GroupEventId counter value.
     */
    static int GetCurrGroupEventId();

    /**
     * \brief Get the current global ObjectEventId.
     *
     * Operations sharing the same ObjectEventId belong to the same logical
     * object change and are always undone together (both Undo(true) and
     * Undo(false)).
     *
     * \return The current ObjectEventId counter value.
     */
    static int GetCurrObjectEventId();

    /**
     * \brief Get this item's GroupEventId.
     * \return The GroupEventId assigned at construction time.
     */
    int GetGroupEventId();

    /**
     * \brief Get this item's ObjectEventId.
     * \return The ObjectEventId assigned at construction time.
     */
    int GetObjectEventId();

    /**
     * \brief Get the textual description of this undo entry.
     * \return The description string.
     */
    std::string GetDescription();

    /** \brief Swap the do/undo operations (toggle reversed state). */
    virtual void ReverseOperations();

    /** \brief Reverse the operations and execute them. */
    virtual void ReverseAndExecute();

    /**
     * \brief Check whether this entry is still valid.
     *
     * Returns false if the destination object has been deleted or the
     * operations are no longer valid.
     *
     * \return True if the entry can still be undone/redone.
     */
    virtual bool IsValid() const = 0;

    /**
     * \brief Increment the global ObjectEventId counter.
     *
     * Call this when a new user interaction produces operations that should
     * be grouped as a single object change. The ObjectEventId groups all
     * operations from one event; the GroupEventId groups multiple
     * ObjectEventIds for coarser undo granularity.
     */
    static void IncCurrObjectEventId();

    /**
     * \brief Increment the global GroupEventId counter.
     *
     * Call this to start a new logical group of operations. Multiple
     * ObjectEventIds within the same GroupEventId are undone together
     * by Undo(false).
     */
    static void IncCurrGroupEventId();

  protected:
    //##Documentation
    //## @brief true, if operation and undooperation have been swapped/changed
    bool m_Reversed;

  private:
    static int m_CurrObjectEventId;

    static int m_CurrGroupEventId;

    int m_ObjectEventId;

    int m_GroupEventId;

    std::string m_Description;

    UndoStackItem(UndoStackItem &);        // hide copy constructor
    void operator=(const UndoStackItem &); // hide operator=
  };

  /**
   * \brief Pairs a do-operation with its inverse undo-operation.
   *
   * Extends UndoStackItem with the actual operation accounting for the undo/redo
   * framework. Holds two Operation objects and the OperationActor (destination)
   * on which they are executed. The operations are swapped when the event moves
   * between undo and redo stacks.
   *
   * This class owns the memory of both operations and deletes them in its
   * destructor. If the destination (an OperationActor, often an itk::Object)
   * is deleted, this event becomes invalid; always check IsValid() before
   * executing operations.
   *
   * \sa UndoStackItem, Operation, OperationActor, UndoController
   * \ingroup Undo
   */
  class MITKCORE_EXPORT OperationEvent : public UndoStackItem
  {
  public:
    /**
     * \brief Construct an OperationEvent.
     *
     * If the destination is an itk::Object, a delete observer is registered
     * to detect when it is destroyed.
     *
     * \param[in] destination    The OperationActor that executes the operations.
     * \param[in] operation      The do-operation (ownership transferred).
     * \param[in] undoOperation  The inverse undo-operation (ownership transferred).
     * \param[in] description    Optional human-readable description.
     */
    OperationEvent(OperationActor *destination,
                   Operation *operation,
                   Operation *undoOperation,
                   std::string description = "");

    /**
     * \brief Destructor.
     *
     * Removes the delete observer (if destination is still valid) and
     * frees both Operation objects.
     */
    ~OperationEvent() override;

    /**
     * \brief Get the current do-operation.
     * \return Pointer to the Operation (may have been swapped with the undo op).
     */
    Operation *GetOperation();

    /**
     * \brief Get the destination actor for the operations.
     * \return Pointer to the OperationActor.
     */
    OperationActor *GetDestination();

    friend class UndoModel;

    /**
     * \brief Swap the do-operation and undo-operation.
     *
     * Called by the undo model when this event is moved between the undo
     * and redo stacks.
     */
    void ReverseOperations() override;

    /**
     * \brief Reverse the operations and execute the (now current) do-operation.
     *
     * Used when an event is moved from the undo stack to the redo stack.
     */
    void ReverseAndExecute() override;

    /**
     * \brief Check whether this event is still valid.
     *
     * Returns false if the destination has been deleted or either operation
     * reports itself as invalid.
     *
     * \return True if the event can be executed.
     */
    bool IsValid() const override;

  protected:
    void OnObjectDeleted();

  private:
    // Has to be observed for itk::DeleteEvents.
    // When destination is deleted, this stack item is invalid!
    OperationActor* m_Destination;

    //## reference to the operation
    Operation *m_Operation;

    //## reference to the undo operation
    Operation *m_UndoOperation;

    //## hide copy constructor
    OperationEvent(OperationEvent &);
    //## hide operator=
    void operator=(const OperationEvent &);

    ITKEventObserverGuard m_DelObserver;

    //## stores if destination is valid or already has been freed
    bool m_Invalid;
  };

} // namespace mitk

#endif
