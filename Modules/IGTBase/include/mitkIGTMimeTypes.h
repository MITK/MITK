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
  };
}


#endif // MITKIOMIMETYPE_H_HEADER_INCLUDED_