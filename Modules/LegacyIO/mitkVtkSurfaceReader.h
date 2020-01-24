/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef VtkSurfaceReader_H_HEADER_INCLUDED
#define VtkSurfaceReader_H_HEADER_INCLUDED

#include "mitkSurfaceSource.h"
#include <MitkLegacyIOExports.h>

namespace mitk
{
  //##Documentation
  //## @brief Reader to read surface files in vtk-format
  //## @ingroup MitkLegacyIOModule
  //## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
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

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

  protected:
    void GenerateData() override;

    VtkSurfaceReader();

    ~VtkSurfaceReader() override;

    std::string m_FileName, m_FilePrefix, m_FilePattern;
  };

} // namespace mitk

#endif /* VtkSurfaceReader_H_HEADER_INCLUDED */
