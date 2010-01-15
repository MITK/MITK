/*=========================================================================

 Program:   openCherry Platform
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


#ifndef OSGI_FRAMEWORK_OBJECTS_H_
#define OSGI_FRAMEWORK_OBJECTS_H_

#include "ObjectGeneric.h"

namespace osgi {

namespace framework {

osgiSpecializeGenericObject(osgi::framework,ObjectBool,bool,false);
osgiSpecializeGenericObject(osgi::framework,ObjectInt,int,0);
osgiSpecializeGenericObject(osgi::framework,ObjectLong,long,0);
osgiSpecializeGenericObject(osgi::framework,ObjectFloat,float,0.0f);
osgiSpecializeGenericObject(osgi::framework,ObjectDouble,double,0.0);
osgiSpecializeGenericObject(osgi::framework,ObjectChar,char,'\0');

}

}

#endif /* OSGI_FRAMEWORK_OBJECTS_H_ */
