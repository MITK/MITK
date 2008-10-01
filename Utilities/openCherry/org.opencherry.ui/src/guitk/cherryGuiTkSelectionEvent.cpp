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

#include "cherryGuiTkSelectionEvent.h"

#include <sstream>

namespace cherry
{

namespace GuiTk
{

SelectionEvent::SelectionEvent(void* item_) :
  item(item_), detail(0), x(0), y(0), width(0), height(0),
  stateMask(0), text(""), doit(true)
{

}

std::string SelectionEvent::ToString()
{
  std::ostringstream stream;
  stream << "GUI SelectionEvent: " << " item=" << item << " detail=" << detail
      << " x=" << x << " y=" << y << " width=" << width << " height=" << height
      << " stateMask=" << stateMask << " text=" << text << " doit=" << doit
      << std::endl;

  return stream.str();
}

}

}
