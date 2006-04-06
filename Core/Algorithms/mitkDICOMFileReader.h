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


#ifndef MITKDICOMFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define MITKDICOMFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read files in DICOM-format
//## @ingroup IO
class DICOMFileReader : public ImageSource, public FileReader
{
public:
    mitkClassMacro(DICOMFileReader, FileReader);

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
    //##ModelId=3E187255016C
    virtual void GenerateData();

    //##ModelId=3E1878400265
    virtual void GenerateOutputInformation();

    //##ModelId=3E1874D202D0
    DICOMFileReader();

    //##ModelId=3E1874E50179
    ~DICOMFileReader();

    //##ModelId=3E19EA120063
    //##Description 
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

    //##ModelId=3EA6ADB80138
    int m_StartFileIndex;
  protected:
    //##ModelId=3E186FC80253
    std::string m_FileName;

    //##ModelId=3E1873B3030A
    std::string m_FilePrefix;

    //##ModelId=3E1873B30346
    std::string m_FilePattern;
private:
	std::string outputfile_name;
	ipPicDescriptor *pic;

};

} // namespace mitk

#endif /* MITKDICOMFILEREADER_H_HEADER_INCLUDED_C1F48A22 */

