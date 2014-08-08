/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

    std::vector<QString> m_Tags;
    QString m_Description;
private:

};

}

#endif
