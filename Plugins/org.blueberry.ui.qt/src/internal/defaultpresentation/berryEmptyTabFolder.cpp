/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEmptyTabFolder.h"
#include "berryEmptyTabItem.h"

#include <QHBoxLayout>
#include <QRect>
#include <QWidget>
#include <QFrame>

namespace berry
{

EmptyTabFolder::EmptyTabFolder(QWidget* parent, bool  /*showborder*/) :
  control(nullptr), childControl(nullptr)
{
  control = new QFrame(parent);
  control->setObjectName("StandaloneViewForm");
  auto   layout = new QHBoxLayout(control);
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
    childControl->setParent(nullptr);
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
  return nullptr;
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
