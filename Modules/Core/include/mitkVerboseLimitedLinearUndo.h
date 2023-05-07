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
#include "mitkLimitedLinearUndo.h"
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
  * @brief A limited linear undo model providing GUI elements with stack status information.
  *
  * Basically does the same, as LimitedLinearUndo class, but it allows you to retrieve a string list, which describes
  * the undo stack or the redo stack. This can be used for display by GUI elements.
  */
  class MITKCORE_EXPORT VerboseLimitedLinearUndo : public LimitedLinearUndo
  {
  public:
    mitkClassMacro(VerboseLimitedLinearUndo, LimitedLinearUndo);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef std::pair<int, std::string> StackDescriptionItem;
    typedef std::vector<StackDescriptionItem>
      StackDescription; /// a list of pairs (int,string), representing a stack with ObjectEventIDs and descriptions

    bool SetOperationEvent(UndoStackItem *undoStackItem) override;

    virtual StackDescription GetUndoDescriptions();
    virtual StackDescription GetRedoDescriptions();

  protected:
    VerboseLimitedLinearUndo();
    ~VerboseLimitedLinearUndo() override;
  };

} // namespace mitk

#endif
