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

#include "berryEmptyTabFolder.h"
#include "berryEmptyTabItem.h"

#include <QHBoxLayout>
#include <QRect>
#include <QWidget>
#include <QFrame>

namespace berry
{

EmptyTabFolder::EmptyTabFolder(QWidget* parent, bool  /*showborder*/) :
  control(0), childControl(0)
{
  control = new QFrame(parent);
  control->setObjectName("StandaloneViewForm");
  QHBoxLayout* layout = new QHBoxLayout(control);
  layout->setContentsMargins(0,0,0,0);
  control->setLayout(layout);

  //borderColor = parent.getDisplay().getSystemColor(SWT.COLOR_WIDGET_NORMAL_SHADOW);
  //        if (showborder) {
  //            layout.xmargin = 1;
  //            layout.ymargin = 1;
  //          control.addPaintListener(new PaintListener() {
  //            public void paintControl(PaintEvent e) {
  //              e.gc.setForeground(borderColor);
  //              Rectangle rect = control.getClientArea();
  //              rect.width--;
  //              rect.height--;
  //              e.gc.drawRectangle(rect);
  //            }
  //          });
  //        }
}

QSize EmptyTabFolder::ComputeSize(int  /*widthHint*/, int  /*heightHint*/)
{
  QRect rect = control->layout()->contentsRect();
  return QSize(rect.x(), rect.y());
}

AbstractTabItem* EmptyTabFolder::Add(int  /*index*/, int  /*flags*/)
{
  return new EmptyTabItem();
}

void EmptyTabFolder::Move(int /*from*/, int /*to*/)
{

}

QWidget* EmptyTabFolder::GetContentParent()
{
  return control;
}

void EmptyTabFolder::SetContent(QWidget* newContent)
{
  if (childControl)
  {
    childControl->setParent(0);
  }

  childControl = newContent;
  control->layout()->addWidget(childControl);
}

QList<AbstractTabItem*> EmptyTabFolder::GetItems()
{
  return QList<AbstractTabItem*>();
}

AbstractTabItem* EmptyTabFolder::GetSelection()
{
  return 0;
}

void EmptyTabFolder::SetSelection(AbstractTabItem*  /*toSelect*/)
{

}

void EmptyTabFolder::Layout(bool flushCache)
{
  AbstractTabFolder::Layout(flushCache);

  //control->layout->update();
}

void EmptyTabFolder::SetSelectedInfo(const PartInfo&  /*info*/)
{

}

void EmptyTabFolder::EnablePaneMenu(bool  /*enabled*/)
{

}

QWidget* EmptyTabFolder::GetControl()
{
  return control;
}

QRect EmptyTabFolder::GetTabArea()
{
  return QRect(0, 0, 0, 0);
}

}
