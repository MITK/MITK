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

#ifndef CHERRYITOOLITEM_H_
#define CHERRYITOOLITEM_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../cherryUiDll.h"

#include "cherryUIElement.h"

namespace cherry
{

struct IToolItemListener;

/**
 * Instances of this class represent a selectable user interface object
 * that represents a button in a tool bar.
 * <dl>
 * <dt><b>Styles:</b></dt>
 * <dd>PUSH, CHECK, RADIO, SEPARATOR, DROP_DOWN</dd>
 * <dt><b>Events:</b></dt>
 * <dd>Selection</dd>
 * </dl>
 * <p>
 * Note: Only one of the styles CHECK, PUSH, RADIO, SEPARATOR and DROP_DOWN
 * may be specified.
 * </p><p>
 * IMPORTANT: This class is <em>not</em> intended to be subclassed.
 * </p>
 */
struct CHERRY_UI IToolItem: public UIElement
{

protected:

  IToolItem(SmartPointer<IServiceLocator> locator) :
    UIElement(locator)
  {
  }

public:

  osgiObjectMacro(cherry::IToolItem)
  osgiManifestMacro(IToolItem)

};

}

#endif /* CHERRYITOOLITEM_H_ */
