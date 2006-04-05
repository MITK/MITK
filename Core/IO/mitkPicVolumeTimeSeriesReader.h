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


#ifndef _PIC_VOLUME_TIME_SERIES_READER__H_
#define _PIC_VOLUME_TIME_SERIES_READER__H_

#include "mitkCommon.h"
#include "mitkFileSeriesReader.h"
#include "mitkImageSource.h"
#include <vector>

namespace mitk
{
//##Documentation
//## @brief Reader to read a series of volume files in DKFZ-pic-format
//## @ingroup IO
class PicVolumeTimeSeriesReader : public ImageSource, public FileSeriesReader
{
public:
    mitkClassMacro( PicVolumeTimeSeriesReader, FileReader );

    /** Method for creation through the object factory. */
    itkNewMacro( Self );
    
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

    virtual void GenerateOutputInformation();

    PicVolumeTimeSeriesReader();

    ~PicVolumeTimeSeriesReader();

    //##Description
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

};

} // namespace mitk

#endif 

