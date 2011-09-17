/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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
