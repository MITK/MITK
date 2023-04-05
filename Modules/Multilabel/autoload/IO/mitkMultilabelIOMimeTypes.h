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
    class MITKMULTILABELIO_EXPORT MitkSegmentationTaskListMimeType : public CustomMimeType
    {
    public:
      MitkSegmentationTaskListMimeType();

      bool AppliesTo(const std::string& path) const override;
      MitkSegmentationTaskListMimeType* Clone() const override;
    };

    MITKMULTILABELIO_EXPORT MitkSegmentationTaskListMimeType SEGMENTATIONTASKLIST_MIMETYPE();
    MITKMULTILABELIO_EXPORT std::string SEGMENTATIONTASKLIST_MIMETYPE_NAME();

    class MITKMULTILABELIO_EXPORT LegacyLabelSetMimeType : public CustomMimeType
    {
    public:
      LegacyLabelSetMimeType();

      bool AppliesTo(const std::string& path) const override;
      LegacyLabelSetMimeType* Clone() const override;
    };

    MITKMULTILABELIO_EXPORT LegacyLabelSetMimeType LEGACYLABELSET_MIMETYPE();
    MITKMULTILABELIO_EXPORT std::string LEGACYLABELSET_MIMETYPE_NAME();

    MITKMULTILABELIO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
