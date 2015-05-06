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
  QRect GetBounds();

  void SetInfo(const PartInfo& info);

  bool GetShowClose() const;

  void SetShowClose(bool close);

  QWidget* GetCloseButton();

  void Dispose();

  /* (non-Javadoc)
   * @see AbstractTabItem#getData()
   */
  Object::Pointer GetData();

  /* (non-Javadoc)
   * @see AbstractTabItem#setData(java.lang.Object)
   */
  void SetData(Object::Pointer d);

};

}

#endif /* BERRYNATIVETABITEM_H_ */
