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


#ifndef ItkImageFileReader_H_HEADER_INCLUDED
#define ItkImageFileReader_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read file formats supported by itk
//## @ingroup IO
class ItkImageFileReader : public ImageSource, public FileReader
{
public:
    mitkClassMacro(ItkImageFileReader, FileReader);

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

    ItkImageFileReader();

    ~ItkImageFileReader();

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

};

} // namespace mitk

#endif /* ItkImageFileReader_H_HEADER_INCLUDED */
