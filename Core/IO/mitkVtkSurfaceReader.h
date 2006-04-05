/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef VtkSurfaceReader_H_HEADER_INCLUDED
#define VtkSurfaceReader_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read surface files in vtk-format
//## @ingroup IO
class VtkSurfaceReader : public SurfaceSource
{
public:
    mitkClassMacro(VtkSurfaceReader, SurfaceSource);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    itkSetMacro(FileName, std::string);
    itkSetStringMacro(FileName);
    itkGetMacro(FileName, std::string);
    itkGetStringMacro(FileName);

    itkSetMacro(FilePrefix, std::string);
    itkSetStringMacro(FilePrefix);
    itkGetMacro(FilePrefix, std::string);
    itkGetStringMacro(FilePrefix);

    itkSetMacro(FilePattern, std::string);
    itkSetStringMacro(FilePattern);
    itkGetMacro(FilePattern, std::string);
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
