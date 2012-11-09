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
#ifndef VtkUnstructuredGridReader_H_HEADER_INCLUDED
#define VtkUnstructuredGridReader_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkUnstructuredGridSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read unstructured grid files in vtk-format
//## @ingroup IO
class MitkExt_EXPORT VtkUnstructuredGridReader : public UnstructuredGridSource
{
public:
    mitkClassMacro(VtkUnstructuredGridReader, UnstructuredGridSource);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

protected:
    virtual void GenerateData();

    VtkUnstructuredGridReader();

    ~VtkUnstructuredGridReader();

    std::string m_FileName, m_FilePrefix, m_FilePattern;

};

} // namespace mitk

#endif /* VtkUnstructuredGridReader_H_HEADER_INCLUDED */
