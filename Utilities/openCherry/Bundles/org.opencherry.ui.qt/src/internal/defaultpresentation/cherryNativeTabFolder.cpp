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

#include "cherryNativeTabFolder.h"
#include "cherryNativeTabItem.h"

#include "../cherryQtControlWidget.h"

#include <cherryShell.h>
#include <cherryConstants.h>

#include <QVBoxLayout>

namespace cherry
{

AbstractTabItem* NativeTabFolder::GetTab(int index)
{
  return tabControl->getTab(index);
}

void NativeTabFolder::TabSelectionChanged(int index)
{
  this->FireEvent(TabFolderEvent::EVENT_TAB_SELECTED, tabControl->getTab(index));
}

void NativeTabFolder::DragStarted(const QPoint& location)
{
  Point point(location.x(), location.y());
  this->HandleDragStarted(location);
}

void NativeTabFolder::CloseButtonClicked(bool /*checked*/)
{
  this->FireEvent(TabFolderEvent::EVENT_CLOSE, this->GetSelection());
}

NativeTabFolder::NativeTabFolder(QWidget* parent)
{
  content = 0;
  viewForm = new QtControlWidget(parent, Shell::Pointer(0));
  qobject_cast<QFrame*>(viewForm)->setFrameStyle(QFrame::Box | QFrame::Plain);
  viewForm->setObjectName("ViewForm");
  QVBoxLayout* layout = new QVBoxLayout(viewForm);
  layout->setContentsMargins(0,0,0,0);
  viewForm->setLayout(layout);

  tabControl = new QCTabBar(viewForm);
  layout->addWidget(tabControl);

  this->connect(tabControl, SIGNAL(currentChanged(int)), this,
      SLOT(TabSelectionChanged(int)));

  this->connect(tabControl, SIGNAL(dragStarted(const QPoint&)), this,
      SLOT(DragStarted(const QPoint&)));


  std::cout << "Created: viewForm <-- " << qPrintable(parent->objectName());
  for (parent = parent->parentWidget(); parent != 0; parent = parent->parentWidget())
    std::cout << " <-- " << qPrintable(parent->objectName());
  std::cout << std::endl;

  parent = viewForm;
  std::cout << "Created control: QCTabBar <-- " << qPrintable(parent->objectName());
  for (parent = parent->parentWidget(); parent != 0; parent = parent->parentWidget())
    std::cout << " <-- " << qPrintable(parent->objectName());
  std::cout << std::endl;

  //attachListeners(control, false);

  //        viewForm = new ViewForm(control, SWT.FLAT);
  //        attachListeners(viewForm, false);
  //        systemToolbar = new StandardSystemToolbar(viewForm, true, false, true, true, true);
  //        systemToolbar.addListener(systemToolbarListener);
  //        viewForm.setTopRight(systemToolbar.getControl());
  //
  //        topCenter = new ProxyControl(viewForm);
  //        topCenterCache = new SizeCache();
  //
  //        title = new CLabel(viewForm, SWT.LEFT);
  //        attachListeners(title, false);
  //        viewForm.setTopLeft(title);
}

QSize NativeTabFolder::ComputeSize(int widthHint, int heightHint)
{
  return QSize(50,50);
}

AbstractTabItem* NativeTabFolder::Add(int index, int flags)
{
  NativeTabItem* item = new NativeTabItem(this, index, flags);

  if (flags & Constants::CLOSE)
  {
    this->connect(item->GetCloseButton(), SIGNAL(clicked(bool)), this, SLOT(CloseButtonClicked(bool)));
  }

  return item;
}

void NativeTabFolder::Layout(bool flushCache)
{
  AbstractTabFolder::Layout(flushCache);

//  QRect rect1 = tabControl->geometry();
//  QRect rect2 = viewForm->geometry();
//  std::cout << "QCTabBar geometry is: x=" << rect1.x() << ", y=" << rect1.y() << ", width=" << rect1.width() << ", height=" << rect1.height() << std::endl;
//  std::cout << "ViewForm geometry is: " << rect2.x() << ", y=" << rect2.y() << ", width=" << rect2.width() << ", height=" << rect2.height() << std::endl;

//  Rectangle oldBounds = viewForm.getBounds();
//  Rectangle newBounds = control.getClientArea();
//
//  viewForm.setBounds(newBounds);
//
//  if (Util.equals(oldBounds, newBounds))
//  {
//    viewForm.layout(flushCache);
//  }
}

QPoint NativeTabFolder::GetPaneMenuLocation()
{
  return AbstractTabFolder::GetPaneMenuLocation();
  //return systemToolbar.getPaneMenuLocation();
}

void NativeTabFolder::SetState(int state)
{
  AbstractTabFolder::SetState(state);

  //systemToolbar.setState(state);
}

QRect NativeTabFolder::GetClientArea()
{
  if (content == 0)
  {
    return QRect();
  }

  return content->geometry();
}

std::vector<AbstractTabItem*> NativeTabFolder::GetItems()
{
  return tabControl->getTabs();
}

void NativeTabFolder::SetSelection(AbstractTabItem* toSelect)
{
  if (toSelect == 0)
  {
    return;
  }

  tabControl->setCurrentTab(toSelect);
}

void NativeTabFolder::SetSelectedInfo(const PartInfo& info)
{
//  if (!Util.equals(title.getText(), info.title))
//  {
//    title.setText(info.title);
//  }
//  if (title.getImage() != info.image)
//  {
//    title.setImage(info.image);
//  }
}

QRect NativeTabFolder::GetTabArea()
{

  return tabControl->geometry();

//  Rectangle bounds = control.getBounds();
//
//  Rectangle clientArea = control.getClientArea();
//
//  bounds.x = 0;
//  bounds.y = 0;
//  Geometry.expand(bounds, 0, 0, -(clientArea.height + clientArea.y), 0);
//
//  return Geometry.toDisplay(control.getParent(), bounds);
}

QWidget* NativeTabFolder::GetControl()
{
  return viewForm;
}

bool NativeTabFolder::IsOnBorder(const QPoint& globalPos)
{
//  Point localPos = getControl().toControl(globalPos);
//
//  Rectangle clientArea = getClientArea();
//  return localPos.y > clientArea.y && localPos.y < clientArea.y
//      + clientArea.height;
  return false;
}

AbstractTabItem* NativeTabFolder::GetSelection()
{
  return tabControl->getCurrentTab();
}

QWidget* NativeTabFolder::GetContentParent()
{
  return viewForm;
}

void NativeTabFolder::SetContent(QWidget* newContent)
{
  //viewForm.setContent(newContent);
  if (content != 0) viewForm->layout()->removeWidget(content);
  content = newContent;
  ((QBoxLayout*)viewForm->layout())->addWidget(newContent, 1);
}

QCTabBar* NativeTabFolder::GetTabFolder()
{
  return tabControl;
}

void NativeTabFolder::SetSelectedTitle(const QString& newTitle)
{
  //title.setText(newTitle);
}

void NativeTabFolder::SetSelectedImage(const QPixmap* image)
{
  //title.setImage(image);
}

AbstractTabItem* NativeTabFolder::GetItem(const QPoint& toFind)
{
  QPoint localPoint = tabControl->mapFromGlobal(toFind);
  int index = tabControl->tabAt(localPoint);
  if (index < 0)
    return 0;
  return tabControl->getTab(index);
}

void NativeTabFolder::EnablePaneMenu(bool enabled)
{
  //systemToolbar.enablePaneMenu(enabled);
}

}
