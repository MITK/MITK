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
  /**
   * \brief Stores a compressed copy of a mitk::Image using LZ4 compression.
   *
   * This class compresses an MITK image slice-by-slice using the LZ4 algorithm
   * and can reconstruct the original image from the compressed data. It is
   * useful for keeping undo/redo snapshots of image data without consuming
   * excessive memory.
   *
   * The class is non-copyable; each instance owns its compressed buffer.
   *
   * \sa mitk::Image
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT CompressedImageContainer
  {
  public:
    /** \brief Construct an empty container. */
    CompressedImageContainer();

    /** \brief Destructor. Frees all compressed data buffers. */
    ~CompressedImageContainer();

    CompressedImageContainer(const CompressedImageContainer&) = delete;
    CompressedImageContainer& operator=(const CompressedImageContainer&) = delete;

    /**
     * \brief Compress the given image and store the result internally.
     *
     * Any previously stored compressed data is cleared before compression.
     * The image is compressed slice-by-slice using the LZ4 algorithm.
     *
     * \param[in] image The image to compress. If nullptr, the container is cleared.
     */
    void CompressImage(const Image* image);

    /**
     * \brief Decompress the stored data and return a new Image.
     *
     * Reconstructs the full image from the internally stored compressed slices.
     *
     * \return A newly created Image with the same pixel type, dimensions, and
     *         geometry as the originally compressed image, or nullptr if no
     *         data is stored.
     */
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
