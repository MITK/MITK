/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkSTLFileReader_h
#define mitkSTLFileReader_h

#include <mitkSurfaceSource.h>
#include <MitkLegacyIOExports.h>

namespace mitk
{
  /**
   * \brief Reader for STL (stereolithography) surface files.
   *
   * Reads surface mesh data from files in STL format and produces a
   * mitk::Surface output. Uses VTK's vtkSTLReader internally.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   * \sa mitk::STLFileIOFactory, mitk::SurfaceSource
   */
  class MITKLEGACYIO_EXPORT STLFileReader : public SurfaceSource
  {
  public:
    mitkClassMacro(STLFileReader, SurfaceSource);

    /** \brief Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the file name of the STL file to read. */
    itkSetStringMacro(FileName);
    /** \brief Get the file name of the STL file to read. */
    itkGetStringMacro(FileName);

    /** \brief Set the file prefix. */
    itkSetStringMacro(FilePrefix);
    /** \brief Get the file prefix. */
    itkGetStringMacro(FilePrefix);

    /** \brief Set the file pattern. */
    itkSetStringMacro(FilePattern);
    /** \brief Get the file pattern. */
    itkGetStringMacro(FilePattern);

    /**
     * \brief Check whether the given file can be read as an STL file.
     * \param[in] filename The file path to check (must have ".stl" extension).
     * \param[in] filePrefix The file prefix (currently unused).
     * \param[in] filePattern The file pattern (currently unused).
     * \return \c true if the file has an STL extension; \c false otherwise.
     */
    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    void GenerateData() override;

    STLFileReader();

    ~STLFileReader() override;

    std::string m_FileName, m_FilePrefix, m_FilePattern;
  };

} // namespace mitk

#endif
