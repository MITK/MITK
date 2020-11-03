/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYOBJECTS_H_
#define BERRYOBJECTS_H_

#include "berryObjectGeneric.h"

namespace berry {

berrySpecializeGenericObject(ObjectBool,bool,false);
berrySpecializeGenericObject(ObjectInt,int,0);
berrySpecializeGenericObject(ObjectFloat,float,0.0f);
berrySpecializeGenericObject(ObjectDouble,double,0.0);

}

#endif /* BERRYOBJECTS_H_ */
