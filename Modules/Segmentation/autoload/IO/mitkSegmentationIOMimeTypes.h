/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationIOMimeTypes_h
#define mitkSegmentationIOMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkSegmentationIOExports.h>

namespace mitk
{
  namespace MitkSegmentationIOMimeTypes
  {
    class MITKSEGMENTATIONIO_EXPORT MitkSegmentationTaskListMimeType : public CustomMimeType
    {
    public:
      MitkSegmentationTaskListMimeType();

      bool AppliesTo(const std::string& path) const override;
      MitkSegmentationTaskListMimeType* Clone() const override;
    };

    MITKSEGMENTATIONIO_EXPORT MitkSegmentationTaskListMimeType SEGMENTATIONTASKLIST_MIMETYPE();
    MITKSEGMENTATIONIO_EXPORT std::string SEGMENTATIONTASKLIST_MIMETYPE_NAME();

    MITKSEGMENTATIONIO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
