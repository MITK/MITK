/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVerboseLimitedLinearUndo_h
#define mitkVerboseLimitedLinearUndo_h

// MITK header
#include <mitkLimitedLinearUndo.h>
#include <MitkCoreExports.h>
// STL header
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace mitk
{
  class UndoStackItem;

  /**
   * \brief A limited linear undo model providing GUI elements with stack status information.
   *
   * Extends LimitedLinearUndo by allowing retrieval of human-readable description
   * lists for the undo and redo stacks. These descriptions can be displayed
   * in GUI elements such as undo/redo menus.
   *
   * \ingroup Undo
   */
  class MITKCORE_EXPORT VerboseLimitedLinearUndo : public LimitedLinearUndo
  {
  public:
    mitkClassMacro(VerboseLimitedLinearUndo, LimitedLinearUndo);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief A pair of ObjectEventId and its human-readable description. */
    typedef std::pair<int, std::string> StackDescriptionItem;

    /** \brief A list of (ObjectEventId, description) pairs representing a stack. */
    typedef std::vector<StackDescriptionItem> StackDescription;

    /**
     * \brief Store an operation event on the undo stack.
     *
     * Clears the redo list when a new operation is stored.
     * Respects the undo limit by removing the oldest entry if necessary.
     *
     * \param[in] undoStackItem The undo stack item to store.
     * \return True if the item was stored successfully, false if the item is nullptr.
     */
    bool SetOperationEvent(UndoStackItem *undoStackItem) override;

    /**
     * \brief Return human-readable descriptions of the undo stack.
     *
     * Groups operations by ObjectEventId and generates a description for each group.
     *
     * \return A StackDescription vector with entries ordered from most recent to oldest.
     */
    virtual StackDescription GetUndoDescriptions();

    /**
     * \brief Return human-readable descriptions of the redo stack.
     *
     * Groups operations by ObjectEventId and generates a description for each group.
     *
     * \return A StackDescription vector with entries ordered from most recent to oldest.
     */
    virtual StackDescription GetRedoDescriptions();

  protected:
    VerboseLimitedLinearUndo();
    ~VerboseLimitedLinearUndo() override;
  };

} // namespace mitk

#endif
