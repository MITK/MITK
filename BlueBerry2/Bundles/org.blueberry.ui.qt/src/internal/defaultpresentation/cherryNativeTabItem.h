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

#ifndef CHERRYNATIVETABITEM_H_
#define CHERRYNATIVETABITEM_H_

#include "../util/cherryAbstractTabItem.h"

#include <QObject>

class QToolButton;

namespace cherry
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

#endif /* CHERRYNATIVETABITEM_H_ */
