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


#ifndef VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96
#define VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96

// MITK header
#include "mitkCommon.h"
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
class MITK_CORE_EXPORT VerboseLimitedLinearUndo : public LimitedLinearUndo
{
public:
  mitkClassMacro(VerboseLimitedLinearUndo, LimitedLinearUndo);
  itkNewMacro(Self);

  typedef std::pair<int,std::string> StackDescriptionItem; 
  typedef std::vector<StackDescriptionItem> StackDescription; /// a list of pairs (int,string), representing a stack with ObjectEventIDs and descriptions

  virtual bool SetOperationEvent(UndoStackItem* undoStackItem);

  virtual StackDescription GetUndoDescriptions();
  virtual StackDescription GetRedoDescriptions();

protected:
  VerboseLimitedLinearUndo();
  virtual ~VerboseLimitedLinearUndo();

};

} // namespace mitk

#endif /* VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96 */
