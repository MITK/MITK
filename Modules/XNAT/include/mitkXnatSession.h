/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKXNATSESSION_H
#define MITKXNATSESSION_H

#include <ctkXnatSession.h>

#include <usServiceInterface.h>

namespace mitk
{
  typedef ctkXnatSession XnatSession;
}
US_DECLARE_SERVICE_INTERFACE(mitk::XnatSession, "org.mitk.services.XnatSession")

#endif // MITKCTKXNATSESSION_H
