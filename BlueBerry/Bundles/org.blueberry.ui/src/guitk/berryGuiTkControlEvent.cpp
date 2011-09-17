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
