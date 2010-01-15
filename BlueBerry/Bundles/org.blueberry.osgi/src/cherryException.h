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


#ifndef CHERRYEXCEPTION_H_
#define CHERRYEXCEPTION_H_

#include "cherryOSGiDll.h"
#include <Poco/Exception.h>

namespace cherry {

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BadWeakPointerException, Poco::Exception)

}

#endif /* CHERRYEXCEPTION_H_ */
