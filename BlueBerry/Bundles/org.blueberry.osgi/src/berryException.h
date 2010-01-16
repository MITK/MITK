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


#ifndef BERRYEXCEPTION_H_
#define BERRYEXCEPTION_H_

#include "berryOSGiDll.h"
#include <Poco/Exception.h>

namespace berry {

POCO_DECLARE_EXCEPTION(BERRY_OSGI, BadWeakPointerException, Poco::Exception)

}

#endif /* BERRYEXCEPTION_H_ */
