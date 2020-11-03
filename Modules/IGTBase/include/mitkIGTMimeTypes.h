/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIOMIMETYPE_H_HEADER_INCLUDED_
#define MITKIOMIMETYPE_H_HEADER_INCLUDED_

#include <MitkIGTBaseExports.h>
#include <mitkIOMimeTypes.h>
#include <mitkCustomMimeType.h>

namespace mitk {
  class MITKIGTBASE_EXPORT IGTMimeTypes
  {
  public:
    static CustomMimeType NAVIGATIONDATASETXML_MIMETYPE();
    static CustomMimeType NAVIGATIONDATASETCSV_MIMETYPE();
    static CustomMimeType USDEVICEINFORMATIONXML_MIMETYPE();
  };
}


#endif // MITKIOMIMETYPE_H_HEADER_INCLUDED_
