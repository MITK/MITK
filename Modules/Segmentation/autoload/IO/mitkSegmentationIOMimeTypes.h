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
  /**
   * \brief MIME type definitions for segmentation I/O operations.
   */
  namespace MitkSegmentationIOMimeTypes
  {
    /**
     * \brief Custom MIME type for segmentation task list files (.json).
     *
     * Identifies JSON files that conform to the segmentation task list format.
     *
     * \sa SegmentationTaskList, SegmentationTaskListIO
     */
    class MITKSEGMENTATIONIO_EXPORT MitkSegmentationTaskListMimeType : public CustomMimeType
    {
    public:
      MitkSegmentationTaskListMimeType();

      bool AppliesTo(const std::string& path) const override;
      MitkSegmentationTaskListMimeType* Clone() const override;
    };

    /** \brief Returns the segmentation task list MIME type instance. */
    MITKSEGMENTATIONIO_EXPORT MitkSegmentationTaskListMimeType SEGMENTATIONTASKLIST_MIMETYPE();

    /** \brief Returns the name string for the segmentation task list MIME type. */
    MITKSEGMENTATIONIO_EXPORT std::string SEGMENTATIONTASKLIST_MIMETYPE_NAME();

    /** \brief Returns all registered segmentation MIME types. */
    MITKSEGMENTATIONIO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
