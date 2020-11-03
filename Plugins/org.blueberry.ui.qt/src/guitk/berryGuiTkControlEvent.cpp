/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryGuiTkControlEvent.h"

namespace berry
{

namespace GuiTk
{

ControlEvent::ControlEvent() :
  Event()
{
}

ControlEvent::ControlEvent(QWidget* item, int x, int y, int width, int height) :
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
