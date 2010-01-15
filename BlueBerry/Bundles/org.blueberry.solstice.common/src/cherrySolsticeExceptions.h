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


#ifndef CHERRYSOLSTICEEXCEPTIONS_H_
#define CHERRYSOLSTICEEXCEPTIONS_H_

#include "cherryCommonRuntimeDll.h"
#include <Poco/Exception.h>

namespace cherry {

/**
 * This exception is thrown to blow out of a long-running method
 * when the user cancels it.
 * <p>
 * This class can be used without OSGi running.
 * </p><p>
 * This class is not intended to be subclassed by clients but
 * may be instantiated.
 * </p>
 */
POCO_DECLARE_EXCEPTION(CHERRY_COMMON_RUNTIME, OperationCanceledException, Poco::RuntimeException);

}

#endif /* CHERRYSOLSTICEEXCEPTIONS_H_ */
