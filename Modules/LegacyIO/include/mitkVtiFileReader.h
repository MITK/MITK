/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtiFileReader_h
#define mitkVtiFileReader_h

#include <mitkFileReader.h>
#include <mitkImageSource.h>
#include <MitkLegacyIOExports.h>

namespace mitk
{
  /**
   * \brief Reader for VTK XML ImageData (.vti) files.
   *
   * Reads image data from VTK XML ImageData format files using
   * vtkXMLImageDataReader and converts the result to a mitk::Image.
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::VtiFileIOFactory, mitk::ImageSource, mitk::FileReader
   */
  class MITKLEGACYIO_EXPORT VtiFileReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(VtiFileReader, FileReader);

    /** \brief Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the file name of the VTI file to read. */
    mitkOverrideSetStringMacro(FileName);
    /** \brief Get the file name of the VTI file to read. */
    mitkOverrideGetStringMacro(FileName);

    /** \brief Set the file prefix. */
    mitkOverrideSetStringMacro(FilePrefix);
    /** \brief Get the file prefix. */
    mitkOverrideGetStringMacro(FilePrefix);

    /** \brief Set the file pattern. */
    mitkOverrideSetStringMacro(FilePattern);
    /** \brief Get the file pattern. */
    mitkOverrideGetStringMacro(FilePattern);

    /**
     * \brief Check whether the given file can be read as a VTI file.
     * \param[in] filename The file path to check (must have ".vti" extension).
     * \param[in] filePrefix The file prefix (currently unused).
     * \param[in] filePattern The file pattern (currently unused).
     * \return \c true if the file has a VTI extension; \c false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    VtiFileReader();

    ~VtiFileReader() override;

    void GenerateData() override;

    /** \brief Time when header was last read. */
    // itk::TimeStamp m_ReadHeaderTime;

  protected:
    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
  };

} // namespace mitk

#endif
