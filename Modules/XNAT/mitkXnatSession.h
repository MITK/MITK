/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKXNATSESSION_H
#define MITKXNATSESSION_H

#include <ctkXnatSession.h>

#include <usServiceInterface.h>

namespace mitk {

  typedef ctkXnatSession XnatSession;

}
US_DECLARE_SERVICE_INTERFACE(mitk::XnatSession, "org.mitk.services.XnatSession")

#endif // MITKCTKXNATSESSION_H
