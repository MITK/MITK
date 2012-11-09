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


#ifndef BERRYPARTINFO_H_
#define BERRYPARTINFO_H_

#include <QIcon>
#include <berryIPresentablePart.h>

namespace berry {

struct PartInfo {
     QString name;
     QString title;
     QString contentDescription;
     QString toolTip;
     QIcon* image;
     bool dirty;

     PartInfo();

     PartInfo(IPresentablePart::Pointer part);

     void Set(IPresentablePart::Pointer part);
};

}


#endif /* BERRYPARTINFO_H_ */
