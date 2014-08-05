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

    using QStandardItem::QStandardItem;
    berry::IPerspectiveDescriptor::Pointer m_Perspective;

private:

};

}

#endif
