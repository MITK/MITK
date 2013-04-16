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


#ifndef BERRYEXCEPTION_H_
#define BERRYEXCEPTION_H_

#include <org_blueberry_osgi_Export.h>
#include <Poco/Exception.h>

namespace berry {

POCO_DECLARE_EXCEPTION(BERRY_OSGI, BadWeakPointerException, Poco::Exception)

}

#endif /* BERRYEXCEPTION_H_ */
