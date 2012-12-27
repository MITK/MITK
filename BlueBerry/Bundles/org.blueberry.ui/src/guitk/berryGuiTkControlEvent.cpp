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

#include "berryGuiTkControlEvent.h"

namespace berry
{

namespace GuiTk
{

ControlEvent::ControlEvent() :
  Event()
{
}

ControlEvent::ControlEvent(void* item, int x, int y, int width, int height) :
  Event()
{
  this->item = item;
  this->x = x;
  this->y = y;
  this->width = width;
  this->height = height;
}

}

}
