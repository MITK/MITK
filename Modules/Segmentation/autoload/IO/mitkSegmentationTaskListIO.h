/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskListIO_h
#define mitkSegmentationTaskListIO_h

#include <mitkAbstractFileIO.h>
#include <mitkFileSystem.h>

namespace mitk
{
  class SegmentationTaskList;

  class SegmentationTaskListIO : public AbstractFileIO
  {
  public:
    SegmentationTaskListIO();

    using AbstractFileReader::Read;
    void Write() override;

  protected:
    std::vector<BaseData::Pointer> DoRead() override;

    int GetMinimumRequiredVersion(const SegmentationTaskList* segmentationTaskList) const;
    fs::path ResolvePath(const fs::path& path) const;

  private:
    SegmentationTaskListIO* IOClone() const override;
  };
}

#endif
