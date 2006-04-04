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
#ifndef STLFileReader_H_HEADER_INCLUDED
#define STLFileReader_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read files in stl-format
//## @ingroup IO
class STLFileReader : public SurfaceSource
{
public:
    mitkClassMacro(STLFileReader, SurfaceSource);

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

    STLFileReader();

    ~STLFileReader();

    std::string m_FileName, m_FilePrefix, m_FilePattern;

};

} // namespace mitk

#endif /* STLFileReader_H_HEADER_INCLUDED */
