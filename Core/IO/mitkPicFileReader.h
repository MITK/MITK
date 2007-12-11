/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef PICFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PICFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##ModelId=3D7B112F00B0
//##Documentation
//## @brief Reader to read files in DKFZ-pic-format
//## @ingroup IO
class PicFileReader : public ImageSource, public FileReader
{
public:
    mitkClassMacro(PicFileReader, FileReader);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    virtual void EnlargeOutputRequestedRegion(itk::DataObject *output);

    static void ConvertHandedness(ipPicDescriptor* pic);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

protected:
    //##ModelId=3E187255016C
    virtual void GenerateData();

    //##ModelId=3E1878400265
    virtual void GenerateOutputInformation();

    //##ModelId=3E1874D202D0
    PicFileReader();

    //##ModelId=3E1874E50179
    ~PicFileReader();

    //##ModelId=3E19EA120063
    //##Description 
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

    //##ModelId=3EA6ADB80138
    int m_StartFileIndex;

    //##ModelId=3E186FC80253
    std::string m_FileName;

    //##ModelId=3E1873B3030A
    std::string m_FilePrefix;

    //##ModelId=3E1873B30346
    std::string m_FilePattern;

};

} // namespace mitk

#endif /* PICFILEREADER_H_HEADER_INCLUDED_C1F48A22 */

