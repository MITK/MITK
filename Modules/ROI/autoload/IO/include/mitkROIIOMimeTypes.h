/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIIOMimeTypes_h
#define mitkROIIOMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkROIIOExports.h>

namespace mitk
{
  namespace MitkROIIOMimeTypes
  {
    class MITKROIIO_EXPORT MitkROIMimeType : public CustomMimeType
    {
    public:
      MitkROIMimeType();

      bool AppliesTo(const std::string& path) const override;
      MitkROIMimeType* Clone() const override;
    };

    MITKROIIO_EXPORT MitkROIMimeType ROI_MIMETYPE();
    MITKROIIO_EXPORT std::string ROI_MIMETYPE_NAME();
    MITKROIIO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
