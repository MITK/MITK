/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKVIEWITEM_H
#define QMITKVIEWITEM_H

#include <QStandardItem>
#include <berryPlatformUI.h>

class QmitkViewItem : public QStandardItem
{
public:

  QmitkViewItem(QString string)
    : QStandardItem(string)
  {
  }

  QmitkViewItem(const QIcon& icon, QString string)
    : QStandardItem(icon, string)
  {
  }

  berry::IViewDescriptor::Pointer m_View;
  QStringList m_Tags;
  QString m_Description;
};

#endif // QMITKVIEWITEM_H
