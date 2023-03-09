/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkViewItem_h
#define QmitkViewItem_h

#include <QStandardItem>
#include <berryPlatformUI.h>

class QmitkViewItem : public QStandardItem
{
public:

  QmitkViewItem(const QString& string)
    : QStandardItem(string)
  {
  }

  QmitkViewItem(const QIcon& icon, const QString& string)
    : QStandardItem(icon, string)
  {
  }

  berry::IViewDescriptor::Pointer m_ItemDescriptor;
  QStringList m_Tags;
  QString m_Description;
};

#endif
