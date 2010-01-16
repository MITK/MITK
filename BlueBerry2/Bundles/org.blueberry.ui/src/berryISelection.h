/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYISELECTION_H_
#define BERRYISELECTION_H_

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>

#include "berryUiDll.h"

namespace berry {

using namespace osgi::framework;

/**
 * \ingroup org_blueberry_ui
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
  osgiInterfaceMacro(berry::ISelection);

  /**
   * Returns whether this selection is empty.
   *
   * @return <code>true</code> if this selection is empty,
   *   and <code>false</code> otherwise
   */
  virtual bool IsEmpty() const = 0;

};

}

#endif /*BERRYISELECTION_H_*/
