/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRawImageFileReader_h
#define mitkRawImageFileReader_h
#include <itkVector.h>
#include <mitkFileReader.h>
#include <mitkImageSource.h>
#include <MitkLegacyIOExports.h>

namespace mitk
{
  /**
   * \brief Reader for raw (headerless) image files.
   *
   * Reads raw binary image data without a header. The user must explicitly configure
   * the dimensionality (2D or 3D), the size of each dimension, the pixel type, and the
   * byte order (endianity) before reading. If any of these parameters are set
   * incorrectly, the image will either fail to load or be displayed incorrectly.
   *
   * Internally uses itk::RawImageIO for the actual file reading.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   * \sa mitk::ImageSource, mitk::FileReader
   */
  class DEPRECATED() MITKLEGACYIO_EXPORT RawImageFileReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(RawImageFileReader, FileReader);

    /** \brief Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the file name of the raw image to read (std::string). */
    itkSetMacro(FileName, std::string);
    /** \brief Set the file name of the raw image to read (C-string). */
    itkSetStringMacro(FileName);
    /** \brief Get the file name of the raw image. */
    itkGetMacro(FileName, std::string);
    /** \brief Get the file name of the raw image (C-string). */
    itkGetStringMacro(FileName);

    /** \brief Set the file prefix. */
    itkSetMacro(FilePrefix, std::string);
    /** \brief Set the file prefix (C-string). */
    itkSetStringMacro(FilePrefix);
    /** \brief Get the file prefix. */
    itkGetMacro(FilePrefix, std::string);
    /** \brief Get the file prefix (C-string). */
    itkGetStringMacro(FilePrefix);

    /** \brief Set the file pattern. */
    itkSetMacro(FilePattern, std::string);
    /** \brief Set the file pattern (C-string). */
    itkSetStringMacro(FilePattern);
    /** \brief Get the file pattern. */
    itkGetMacro(FilePattern, std::string);
    /** \brief Get the file pattern (C-string). */
    itkGetStringMacro(FilePattern);

    /**
     * \brief Enumeration of supported pixel types for raw image reading.
     */
    typedef enum {
      UCHAR,  ///< \brief Unsigned 8-bit integer.
      SCHAR,  ///< \brief Signed 8-bit integer.
      USHORT, ///< \brief Unsigned 16-bit integer.
      SSHORT, ///< \brief Signed 16-bit integer.
      UINT,   ///< \brief Unsigned 32-bit integer.
      SINT,   ///< \brief Signed 32-bit integer.
      FLOAT,  ///< \brief 32-bit floating point.
      DOUBLE  ///< \brief 64-bit floating point.
    } IOPixelType;

    /**
     * \brief Set the pixel type of the raw image data.
     * \param[in] _arg The pixel type to use for reading.
     */
    itkSetMacro(PixelType, IOPixelType);

    /**
     * \brief Enumeration of supported byte orders.
     */
    typedef enum {
      LITTLE, ///< \brief Little-endian byte order.
      BIG     ///< \brief Big-endian byte order (default).
    } EndianityType;

    /**
     * \brief Set the byte order of the raw image data.
     * \param[in] _arg The endianity to use for reading.
     */
    itkSetMacro(Endianity, EndianityType);

    /**
     * \brief Set the number of dimensions (2 or 3).
     * \param[in] _arg The dimensionality of the raw image.
     */
    itkSetMacro(Dimensionality, int);

    /**
     * \brief Get the number of dimensions.
     * \return The dimensionality (2 or 3).
     */
    itkGetMacro(Dimensionality, int);

    /**
     * \brief Set the size of a specific image dimension.
     *
     * Dimensions must be set one by one, starting from dimension 0.
     * Valid indices are 0, 1, and 2.
     *
     * \param[in] i The dimension index (0, 1, or 2). Values greater than 2 are ignored.
     * \param[in] dim The size of the dimension in pixels.
     */
    void SetDimensions(unsigned int i, unsigned int dim);

    /**
     * \brief Get the size of a specific image dimension.
     * \param[in] i The dimension index (0, 1, or 2).
     * \return The size of the dimension in pixels, or 0 if \p i > 2.
     */
    unsigned int GetDimensions(unsigned int i) const;

    /**
     * \brief Check whether the given file can be read as a raw image.
     *
     * Returns \c true if the filename is non-empty and the file is not part of a series.
     *
     * \param[in] filename The file path to check.
     * \param[in] filePrefix The file prefix.
     * \param[in] filePattern The file pattern.
     * \return \c true if the file can potentially be read; \c false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    RawImageFileReader();

    ~RawImageFileReader() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimensions>
    void TypedGenerateData();

    /** Name of file to be read.*/
    std::string m_FileName;

    /** File prefix.  */
    std::string m_FilePrefix;

    /** File pattern. */
    std::string m_FilePattern;

    /** Pixel type of image to be read. Must be of type IOPixelType. */
    IOPixelType m_PixelType;

    /** Dimensionality of file to be read. Can be 2 or 3. */
    int m_Dimensionality;

    /** Endianity. Must be set to LITTLE or BIG. Default is BIG. */
    EndianityType m_Endianity;

    /** Vector containing dimensions of image to be read. */
    itk::Vector<int, 3> m_Dimensions;
  };

} // namespace mitk

#endif
