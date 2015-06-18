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


#ifndef VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96
#define VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96

// MITK header
#include <MitkCoreExports.h>
#include "mitkLimitedLinearUndo.h"
// STL header
#include <vector>
#include <list>
#include <utility>
#include <string>

namespace mitk {

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
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef std::pair<int,std::string> StackDescriptionItem;
  typedef std::vector<StackDescriptionItem> StackDescription; /// a list of pairs (int,string), representing a stack with ObjectEventIDs and descriptions

  virtual bool SetOperationEvent(UndoStackItem* undoStackItem) override;

  virtual StackDescription GetUndoDescriptions();
  virtual StackDescription GetRedoDescriptions();

protected:
  VerboseLimitedLinearUndo();
  virtual ~VerboseLimitedLinearUndo();

};

} // namespace mitk

#endif /* VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96 */
