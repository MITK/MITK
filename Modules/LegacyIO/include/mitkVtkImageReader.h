/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkImageReader_h
#define mitkVtkImageReader_h

#include <mitkFileReader.h>
#include <mitkImageSource.h>
#include <MitkLegacyIOExports.h>

namespace mitk
{
  /**
   * \brief Reader to read image files in VTK file format.
   *
   * \ingroup MitkLegacyIOModule
   */
  class MITKLEGACYIO_EXPORT VtkImageReader : public ImageSource, public FileReader
  {
  public:
    mitkClassMacro(VtkImageReader, FileReader);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkOverrideSetStringMacro(FileName);
    mitkOverrideGetStringMacro(FileName);

    mitkOverrideSetStringMacro(FilePrefix);
    mitkOverrideGetStringMacro(FilePrefix);

    mitkOverrideSetStringMacro(FilePattern);
    mitkOverrideGetStringMacro(FilePattern);

    /**
     * \brief Check if the given file can be read.
     * \param filename The name of the file to check.
     * \param filePrefix The file prefix.
     * \param filePattern The file pattern.
     * \return True if the file can be read, false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    /** \brief Default constructor. */
    VtkImageReader();

    /** \brief Destructor. */
    ~VtkImageReader() override;

    /** \brief Generate the output data. */
    void GenerateData() override;

    /** \brief Time when header was last read. */
    // itk::TimeStamp m_ReadHeaderTime;

  protected:
    std::string m_FileName;     ///< \brief The file name to read.

    std::string m_FilePrefix;   ///< \brief The file prefix.

    std::string m_FilePattern;  ///< \brief The file pattern.
  };

} // namespace mitk

#endif
