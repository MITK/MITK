/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _PerspectiveItem
#define _PerspectiveItem

#include <QStandardItem>
#include <berryPlatformUI.h>

namespace mitk
{

class QtPerspectiveItem : public QStandardItem
{
public:
    QtPerspectiveItem(QString string) :
      QStandardItem(string)
    {
    }
    QtPerspectiveItem(const QIcon& icon, QString string) :
      QStandardItem(icon, string)
    {
    }

    berry::IPerspectiveDescriptor::Pointer m_Perspective;

    QStringList m_Tags;
    QString m_Description;
private:

};

}

#endif
