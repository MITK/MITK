/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYNATIVETABITEM_H_
#define BERRYNATIVETABITEM_H_

#include "internal/util/berryAbstractTabItem.h"

#include <QObject>

class QToolButton;

namespace berry
{

class NativeTabFolder;

class NativeTabItem: public QObject, public AbstractTabItem
{

  Q_OBJECT

private:

  Object::Pointer data;
  NativeTabFolder* parent;
  const int style;
  bool showClose;

  QToolButton* closeButton;

private slots:

  void CloseButtonClicked();

public:

  NativeTabItem(NativeTabFolder* parent, int index, int style);

  /* (non-Javadoc)
   * @see AbstractTabItem#getBounds()
   */
  QRect GetBounds() override;

  void SetInfo(const PartInfo& info) override;

  bool GetShowClose() const;

  void SetShowClose(bool close);

  QWidget* GetCloseButton();

  void Dispose() override;

  /* (non-Javadoc)
   * @see AbstractTabItem#getData()
   */
  Object::Pointer GetData() override;

  /* (non-Javadoc)
   * @see AbstractTabItem#setData(java.lang.Object)
   */
  void SetData(Object::Pointer d) override;

};

}

#endif /* BERRYNATIVETABITEM_H_ */
