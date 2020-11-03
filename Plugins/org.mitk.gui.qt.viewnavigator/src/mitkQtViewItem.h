/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _ViewItem
#define _ViewItem

#include <QStandardItem>
#include <QString>
#include <berryPlatformUI.h>

namespace mitk
{

class QtViewItem : public QStandardItem
{
public:
    QtViewItem(QString string) :
      QStandardItem(string)
    {
    }
    QtViewItem(const QIcon& icon, QString string) :
      QStandardItem(icon, string)
    {
    }

    berry::IViewDescriptor::Pointer m_View;
    QStringList m_Tags;
    QString m_Description;

private:

};

}

#endif
