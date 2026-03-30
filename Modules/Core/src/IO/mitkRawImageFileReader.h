/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRawImageFileReader_h
#define mitkRawImageFileReader_h

#include <mitkAbstractFileReader.h>

namespace mitk
{
  /**
   * \brief Reader for raw (headerless) binary image files.
   *
   * Reads raw image data where the user must specify the dimensionality (2D or 3D),
   * the size along each dimension, the pixel type, and the byte order (endianness)
   * via reader options. If these parameters are incorrect, the image will not open
   * or the visualization will be incorrect.
   *
   * \sa AbstractFileReader
   */
  class RawImageFileReaderService : public AbstractFileReader
  {
  public:
    /** \brief Supported pixel types for raw image data. */
    typedef enum { UCHAR, SCHAR, USHORT, SSHORT, UINT, SINT, FLOAT, DOUBLE } IOPixelType;

    /** \brief Byte order of the raw image data. */
    typedef enum { LITTLE, BIG } EndianityType;

    /**
     * \brief Construct and register the raw image reader service.
     *
     * Sets up default options for pixel type, dimensionality, endianness,
     * and image dimensions.
     */
    RawImageFileReaderService();

  protected:
    /** \brief Copy constructor. */
    RawImageFileReaderService(const RawImageFileReaderService &other);

    /**
     * \brief Read a raw binary image file based on the configured options.
     *
     * Determines pixel type, dimensionality, endianness, and dimensions from
     * the reader options, then delegates to TypedRead for the actual I/O.
     *
     * \return A vector containing the read Image.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    using mitk::AbstractFileReader::Read;

  private:
    /**
     * \brief Templated reading method for a specific pixel type and dimensionality.
     *
     * \tparam TPixel The pixel type.
     * \tparam VImageDimensions The number of image dimensions.
     * \param[in] path The file path to read.
     * \param[in] endianity The byte order of the data.
     * \param[in] size Array of dimension sizes.
     * \return The read Image as a BaseData pointer.
     */
    template <typename TPixel, unsigned int VImageDimensions>
    mitk::BaseData::Pointer TypedRead(const std::string &path, EndianityType endianity, int *size);

    RawImageFileReaderService *Clone() const override;
  };

} // namespace mitk

#endif
