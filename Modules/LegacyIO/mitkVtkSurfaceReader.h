/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef VtkSurfaceReader_H_HEADER_INCLUDED
#define VtkSurfaceReader_H_HEADER_INCLUDED

#include <MitkLegacyIOExports.h>
#include "mitkSurfaceSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read surface files in vtk-format
//## @ingroup IO
//## @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
class MitkLegacyIO_EXPORT VtkSurfaceReader : public SurfaceSource
{
public:
    mitkClassMacro(VtkSurfaceReader, SurfaceSource);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

protected:
    virtual void GenerateData();

    VtkSurfaceReader();

    ~VtkSurfaceReader();

    std::string m_FileName, m_FilePrefix, m_FilePattern;

};

} // namespace mitk

#endif /* VtkSurfaceReader_H_HEADER_INCLUDED */
