/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 8426 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef VtkUnstructuredGridReader_H_HEADER_INCLUDED
#define VtkUnstructuredGridReader_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkUnstructuredGridSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read unstructured grid files in vtk-format
//## @ingroup IO
class MITK_CORE_EXPORT VtkUnstructuredGridReader : public UnstructuredGridSource
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
