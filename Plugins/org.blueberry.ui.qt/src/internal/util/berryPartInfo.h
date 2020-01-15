/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
     QIcon image;
     bool dirty;

     PartInfo();

     PartInfo(IPresentablePart::Pointer part);

     void Set(IPresentablePart::Pointer part);
};

}


#endif /* BERRYPARTINFO_H_ */
