/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryGuiTkEvent.h"

#include <sstream>

namespace berry {

namespace GuiTk {

Event::Event() :
item(0), detail(0), x(0), y(0),
width(0), height(0), button(0),
character(0), keyCode(0), stateMask(0),
text(""), doit(true)
{

}

std::string Event::ToString() const
{
  std::stringstream stream;
  stream << "GUI SelectionEvent: " << " item=" << item << " detail=" << detail
      << " x=" << x << " y=" << y << " width=" << width << " height=" << height
      << " stateMask=" << stateMask << " text=" << text << " doit=" << doit
      << std::endl;

  return stream.str();
}

}

}
