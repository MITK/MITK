/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPerspectiveItem_h
#define QmitkPerspectiveItem_h

#include <QStandardItem>
#include <berryPlatformUI.h>

class QmitkPerspectiveItem : public QStandardItem
{
public:

  QmitkPerspectiveItem(const QString& string)
    : QStandardItem(string)
  {
  }

  QmitkPerspectiveItem(const QIcon& icon, const QString& string)
    : QStandardItem(icon, string)
  {
  }

  berry::IPerspectiveDescriptor::Pointer m_ItemDescriptor;
  QStringList m_Tags;
  QString m_Description;
};

#endif
