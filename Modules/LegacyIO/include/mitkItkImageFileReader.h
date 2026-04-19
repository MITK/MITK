/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkItkImageFileReader_h
#define mitkItkImageFileReader_h

#include <mitkFileReader.h>
#include <mitkImageSource.h>
#include <MitkLegacyIOExports.h>

namespace mitk
{
  /**
   * \brief Reader for image file formats supported by ITK.
   *
   * Reads image files using the ITK image IO factory mechanism. The reader can handle
   * any image format for which an ITK image IO object is registered. The resulting
   * image data is provided as a mitk::Image.
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::ItkImageFileIOFactory, mitk::ImageSource, mitk::FileReader
   */
  class MITKLEGACYIO_EXPORT ItkImageFileReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(ItkImageFileReader, FileReader);

    /** \brief Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the file name of the image file to read. */
    mitkOverrideSetStringMacro(FileName);
    /** \brief Get the file name of the image file to read. */
    mitkOverrideGetStringMacro(FileName);

    /** \brief Set the file prefix (for series reading). */
    mitkOverrideSetStringMacro(FilePrefix);
    /** \brief Get the file prefix. */
    mitkOverrideGetStringMacro(FilePrefix);

    /** \brief Set the file pattern (for series reading). */
    mitkOverrideSetStringMacro(FilePattern);
    /** \brief Get the file pattern. */
    mitkOverrideGetStringMacro(FilePattern);

    /**
     * \brief Check whether the given file can be read by this reader.
     *
     * Tests whether ITK has a registered ImageIO that can read the specified file.
     *
     * \param[in] filename The file name to check.
     * \param[in] filePrefix The file prefix (currently unused).
     * \param[in] filePattern The file pattern (currently unused).
     * \return \c true if the file can be read; \c false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    void GenerateData() override;

    ItkImageFileReader();

    ~ItkImageFileReader() override;

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
  };

} // namespace mitk

#endif
