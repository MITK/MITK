/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultilabelIOMimeTypes_h
#define mitkMultilabelIOMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkMultilabelIOExports.h>

namespace mitk
{
  namespace MitkMultilabelIOMimeTypes
  {
    class MITKMULTILABELIO_EXPORT MitkSegmentationTaskMimeType : public CustomMimeType
    {
    public:
      MitkSegmentationTaskMimeType();

      bool AppliesTo(const std::string& path) const override;
      MitkSegmentationTaskMimeType* Clone() const override;
    };

    MITKMULTILABELIO_EXPORT MitkSegmentationTaskMimeType SEGMENTATIONTASK_MIMETYPE();
    MITKMULTILABELIO_EXPORT std::string SEGMENTATIONTASK_MIMETYPE_NAME();

    MITKMULTILABELIO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
