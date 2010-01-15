/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYISELECTION_H_
#define CHERRYISELECTION_H_

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>

#include "cherryUiDll.h"

namespace cherry {

using namespace osgi::framework;

/**
 * \ingroup org_opencherry_ui
 *
 * Interface for a selection.
 *
 * @see ISelectionProvider
 * @see ISelectionChangedListener
 * @see SelectionChangedEvent
 *
 **/
struct ISelection : public Object
{
  osgiInterfaceMacro(cherry::ISelection);

  /**
   * Returns whether this selection is empty.
   *
   * @return <code>true</code> if this selection is empty,
   *   and <code>false</code> otherwise
   */
  virtual bool IsEmpty() const = 0;

};

}

#endif /*CHERRYISELECTION_H_*/
