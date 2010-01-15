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


#ifndef CHERRYOBJECTS_H_
#define CHERRYOBJECTS_H_

#include "cherryObjectGeneric.h"

namespace cherry {

cherrySpecializeGenericObject(ObjectBool,bool,false);
cherrySpecializeGenericObject(ObjectInt,int,0);
cherrySpecializeGenericObject(ObjectFloat,float,0.0f);
cherrySpecializeGenericObject(ObjectDouble,double,0.0);

}

#endif /* CHERRYOBJECTS_H_ */
