/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryGuiTkEvent.h"

#include <QTextStream>

namespace berry {

namespace GuiTk {

Event::Event() :
item(nullptr), detail(0), x(0), y(0),
width(0), height(0), button(0),
character(0), keyCode(0), stateMask(0),
text(""), doit(true)
{

}

QString Event::ToString() const
{
  QString str;
  QTextStream stream(&str);
  stream << "GUI SelectionEvent: " << " item=" << item << " detail=" << detail
      << " x=" << x << " y=" << y << " width=" << width << " height=" << height
      << " stateMask=" << stateMask << " text=" << text << " doit=" << doit
      << '\n';

  return str;
}

}

}
