/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "mitkOperationEvent.h"
#include "mitkLimitedLinearUndo.h"
// STL header
#include <vector>
#include <list>
#include <string>
// ITK header
#include <itkObject.h>
#include<itkEventObject.h>

namespace mitk {

/**
* @brief A limited linear undo model providing GUI elements with stack status information.
*
* Basically does the same, as LimitedLinearUndo class, but it allows you to retrieve a string list, which describes
* the undo stack or the redo stack. This can be used for display by GUI elements.
*/
class VerboseLimitedLinearUndo : public LimitedLinearUndo
{
public:
  virtual bool SetOperationEvent(UndoStackItem* undoStackItem);

  VerboseLimitedLinearUndo();

  virtual bool Undo();
  virtual bool Undo(bool fine);
  
  virtual bool Redo();
  virtual bool Redo(bool fine);

  virtual ~VerboseLimitedLinearUndo();

  virtual std::list<std::string> GetUndoDescriptions();
  virtual std::list<std::string> GetRedoDescriptions();
};

} // namespace mitk

#endif /* VERBOSELIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E96 */
