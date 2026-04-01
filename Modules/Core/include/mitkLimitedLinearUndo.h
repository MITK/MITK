/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLimitedLinearUndo_h
#define mitkLimitedLinearUndo_h

// MITK header
#include <mitkOperationEvent.h>
#include <mitkUndoModel.h>
#include <MitkCoreExports.h>
// STL header
#include <vector>
// ITK header
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

#include <deque>

namespace mitk
{
  /**
   * \brief A linear undo model with one undo and one redo stack.
   *
   * Derived from UndoModel AND itk::Object. Invokes ITK-events to signal listening
   * GUI elements whether each of the stacks is empty or not (to enable/disable buttons, etc.).
   */
  class MITKCORE_EXPORT LimitedLinearUndo : public UndoModel
  {
  public:
    typedef std::deque<UndoStackItem *> UndoContainer;
    typedef std::deque<UndoStackItem *>::reverse_iterator UndoContainerRevIter;

    mitkClassMacro(LimitedLinearUndo, UndoModel);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Store an operation event on the undo stack.
     *
     * Clears the redo stack when a new operation is added. If the undo limit
     * is reached, the oldest entry is removed.
     *
     * \return True if the stack item was successfully added.
     */
    bool SetOperationEvent(UndoStackItem *stackItem) override;

    /** \brief Undoes the last changes.
     *
     * Reads the top element of the Undo-Stack,
     * executes the operation,
     * swaps the OperationEvent-Undo with the Operation
     * and sets it to the Redo-Stack.
     */
    bool Undo() override;

    /** \brief Undoes the last changes.
     *
     * If fine is true, undoes one object event ID; otherwise undoes
     * one group event ID.
     */
    bool Undo(bool) override;

    /** \brief Undoes all changes until ObjectEventID oeid. */
    virtual bool Undo(int oeid);

    /** \brief Redoes the last undone changes.
     *
     * Reads the top element of the Redo-Stack,
     * executes the operation,
     * swaps the OperationEvent-Operation with the Undo-Operation
     * and sets it to the Undo-Stack.
     */
    bool Redo() override;

    /** \brief Redoes changes (parameter currently unused). */
    bool Redo(bool) override;

    /** \brief Redoes all changes until ObjectEventID oeid. */
    virtual bool Redo(int oeid);

    /** \brief Clears both the UndoList and RedoList. */
    void Clear() override;

    /** \brief Clears the RedoList. */
    void ClearRedoList() override;

    /** \brief Returns true if the RedoList is empty. */
    bool RedoListEmpty() override;

    /** \brief Returns true if the UndoList is empty. */
    bool UndoListEmpty() override;

    /** \brief Gets the limit on the size of the undo history.
     *
     * The undo limit determines how many items can be stored
     * in the undo stack. A value of 0 means there is no limit.
     */
    std::size_t GetUndoLimit() const override;

    /** \brief Sets a limit on the size of the undo history.
     *
     * If the limit is reached, the oldest undo items will
     * be dropped from the bottom of the undo stack.
     * A value of 0 means there is no limit.
     */
    void SetUndoLimit(std::size_t limit) override;

    /** \brief Returns the ObjectEventId of the top element in the OperationHistory. */
    int GetLastObjectEventIdInList() override;

    /** \brief Returns the GroupEventId of the top element in the OperationHistory. */
    int GetLastGroupEventIdInList() override;

    /** \brief Returns the last OperationEvent in the Undo-list matching the given
     *         destination and operation type.
     *
     * \return The matching OperationEvent, or nullptr if nothing was found.
     */
    OperationEvent *GetLastOfType(OperationActor *destination, OperationType opType) override;

    /** \brief Removes invalid operations from the undo/redo stack.
     *
     * Iterates through m_UndoList and m_RedoList and removes invalid OperationEvents.
     *
     * \return Number of invalid operations removed.
     */
    unsigned int RemoveInvalidOperations() override;

  protected:
    /** \brief Constructor. */
    LimitedLinearUndo();

    /** \brief Destructor. Deletes all undo and redo entries. */
    ~LimitedLinearUndo() override;

    /** \brief Convenience method to free the memory of elements in the list and clear it. */
    void ClearList(UndoContainer *list);

    UndoContainer m_UndoList;

    UndoContainer m_RedoList;

  private:
    int FirstObjectEventIdOfCurrentGroup(UndoContainer &stack);

    std::size_t m_UndoLimit;

  };

#pragma GCC visibility push(default)

  /** \brief ITK events to notify listening GUI elements when the undo or redo stack
   *         is empty (disable button) or when there are items in the stack (enable button).
   */
  itkEventMacroDeclaration(UndoStackEvent, itk::ModifiedEvent);
  itkEventMacroDeclaration(UndoEmptyEvent, UndoStackEvent);   ///< \brief Undo stack became empty.
  itkEventMacroDeclaration(RedoEmptyEvent, UndoStackEvent);   ///< \brief Redo stack became empty.
  itkEventMacroDeclaration(UndoNotEmptyEvent, UndoStackEvent); ///< \brief Undo stack is no longer empty.
  itkEventMacroDeclaration(RedoNotEmptyEvent, UndoStackEvent); ///< \brief Redo stack is no longer empty.
  /** \brief Additional events for signaling that a stack has reached its limit. */
  itkEventMacroDeclaration(UndoFullEvent, UndoStackEvent);    ///< \brief Undo stack is full.
  itkEventMacroDeclaration(RedoFullEvent, UndoStackEvent);    ///< \brief Redo stack is full.

#pragma GCC visibility pop

} // namespace mitk

#endif
