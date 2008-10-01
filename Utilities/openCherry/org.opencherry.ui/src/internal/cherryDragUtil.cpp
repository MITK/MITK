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

#include "cherryDragUtil.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"

namespace cherry {

Rectangle DragUtil::GetDisplayBounds(void* boundsControl)
{
  void* parent = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(boundsControl);
  if (parent == 0)
  {
      return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(boundsControl);
  }

  Rectangle rect = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(boundsControl);
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToDisplay(parent, rect);
}

}
