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

#include "cherryQtAbstractControlWidget.h"

namespace cherry {

void QtAbstractControlWidget::AddControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.AddListener(listener);
}

void QtAbstractControlWidget::RemoveControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.RemoveListener(listener);
}

void QtAbstractControlWidget::AddShellListener(IShellListener::Pointer listener)
{
  shellEvents.AddListener(listener);
}

void QtAbstractControlWidget::RemoveShellListener(IShellListener::Pointer listener)
{
  shellEvents.RemoveListener(listener);
}

}
