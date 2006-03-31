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
#include "mitkFileReader.h"
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

    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    static bool CanReadFile(const char* file);

protected:
    virtual void GenerateData();

    STLFileReader();

    ~STLFileReader();

    std::string m_FileName;

};

} // namespace mitk

#endif /* STLFileReader_H_HEADER_INCLUDED */
