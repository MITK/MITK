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
    std::vector<QString> m_Tags;
    QString m_Description;

private:

};

}

#endif
