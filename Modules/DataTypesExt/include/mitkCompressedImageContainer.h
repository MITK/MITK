/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCompressedImageContainer_h
#define mitkCompressedImageContainer_h

#include <MitkDataTypesExtExports.h>
#include <mitkImage.h>
#include <array>
#include <memory>
#include <utility>

namespace mitk
{
  class MITKDATATYPESEXT_EXPORT CompressedImageContainer
  {
  public:
    CompressedImageContainer();
    ~CompressedImageContainer();

    CompressedImageContainer(const CompressedImageContainer&) = delete;
    CompressedImageContainer& operator=(const CompressedImageContainer&) = delete;

    void CompressImage(const Image* image);
    Image::Pointer DecompressImage() const;

  private:
    using CompressedSliceData = std::pair<int, char*>;
    using CompressedTimeStepData = std::vector<CompressedSliceData>;
    using CompressedImageData = std::vector<CompressedTimeStepData>;

    void ClearCompressedImageData();

    CompressedImageData m_CompressedImageData;

    std::unique_ptr<PixelType> m_PixelType;
    TimeGeometry::Pointer m_TimeGeometry;
    std::array<unsigned int, 2> m_SliceDimensions;
    unsigned int m_Dimension;
  };
}

#endif
