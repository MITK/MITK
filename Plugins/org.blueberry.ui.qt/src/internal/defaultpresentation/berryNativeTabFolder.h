/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYNATIVETABFOLDER_H_
#define BERRYNATIVETABFOLDER_H_

#include <internal/util/berryAbstractTabFolder.h>

#include <QObject>

class QFrame;
class QWidget;

namespace berry
{

struct IQtStyleManager;

class QCTabBar;

class NativeTabFolder: public QObject, public AbstractTabFolder
{

Q_OBJECT

private:

  QCTabBar* tabControl;
  QFrame* contentFrame;
  QWidget* viewForm;
  QWidget* content;

  IQtStyleManager* skinManager;

  /**
   * @param item
   * @return
   */
  AbstractTabItem* GetTab(int index);

private slots:

  void TabSelectionChanged(int index);
  void DragStarted(const QPoint& location);
  void ViewFormDestroyed(QObject*);

public:

  NativeTabFolder(QWidget* parent);

  ~NativeTabFolder() override;

  void UpdateColors();

  void SetActive(int activeState) override;

  bool eventFilter(QObject* watched, QEvent* event) override;

  void CloseButtonClicked(AbstractTabItem* item);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#computeSize(int, int)
   */
  QSize ComputeSize(int widthHint, int heightHint) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#add(int)
   */
  AbstractTabItem* Add(int index, int flags) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#move(int, int)
   */
  void Move(int from, int to) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#layout(boolean)
   */
  void Layout(bool flushCache) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getPaneMenuLocation()
   */
  QPoint GetPaneMenuLocation() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setState(int)
   */
  void SetState(int state) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getClientArea()
   */
  QRect GetClientArea();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getItems()
   */
  QList<AbstractTabItem*> GetItems() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setSelection(org.blueberry.ui.internal.presentations.util.Widget)
   */
  void SetSelection(AbstractTabItem* toSelect) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setSelectedInfo(org.blueberry.ui.internal.presentations.util.PartInfo)
   */
  void SetSelectedInfo(const PartInfo& info) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getTabArea()
   */
  QRect GetTabArea() override;

  QWidget* GetControl() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#isOnBorder(org.blueberry.swt.graphics.Point)
   */
  bool IsOnBorder(const QPoint& globalPos) override;

  AbstractTabItem* GetSelection() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getContentParent()
   */
  QWidget* GetContentParent() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#setContent(org.blueberry.swt.widgets.Control)
   */
  void SetContent(QWidget* newContent) override;

  /**
   * @return
   * @since 3.1
   */
  QCTabBar* GetTabFolder();

  /**
   * @param item
   * @param newTitle
   * @since 3.1
   */
  /* protected */void SetSelectedTitle(const QString& newTitle);

  /**
   * @param image
   * @since 3.1
   */
  /* protected */void SetSelectedImage(const QPixmap* image);

  using AbstractTabFolder::GetItem;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.presentations.util.AbstractTabFolder#getItem(org.blueberry.swt.graphics.Point)
   */
  AbstractTabItem* GetItem(const QPoint& toFind) override;

  /**
   * @param enabled
   * @since 3.1
   */
  void EnablePaneMenu(bool enabled) override;

};

}

#endif /* BERRYNATIVETABFOLDER_H_ */
