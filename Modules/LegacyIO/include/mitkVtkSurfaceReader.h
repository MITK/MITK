/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVtkSurfaceReader_h
#define mitkVtkSurfaceReader_h

#include <mitkSurfaceSource.h>
#include <MitkLegacyIOExports.h>

namespace mitk
{
  /**
   * \brief Reader to read surface files in VTK format.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   */
  class MITKLEGACYIO_EXPORT VtkSurfaceReader : public SurfaceSource
  {
  public:
    mitkClassMacro(VtkSurfaceReader, SurfaceSource);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    /**
     * \brief Check if the given file can be read.
     * \param filename The name of the file to check.
     * \param filePrefix The file prefix.
     * \param filePattern The file pattern.
     * \return True if the file can be read, false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    /** \brief Generate the output data. */
    void GenerateData() override;

    /** \brief Default constructor. */
    VtkSurfaceReader();

    /** \brief Destructor. */
    ~VtkSurfaceReader() override;

    std::string m_FileName;     ///< \brief The file name to read.
    std::string m_FilePrefix;   ///< \brief The file prefix.
    std::string m_FilePattern;  ///< \brief The file pattern.
  };

} // namespace mitk

#endif
