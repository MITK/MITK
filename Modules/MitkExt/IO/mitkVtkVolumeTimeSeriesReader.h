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


#ifndef _Vtk_VOLUME_TIME_SERIES_READER__H_
#define _Vtk_VOLUME_TIME_SERIES_READER__H_

#include "mitkCommon.h"
#include "mitkFileSeriesReader.h"
#include "mitkSurfaceSource.h"

namespace mitk
{
//##Documentation
//## @brief Reader to read a series of volume files in Vtk-format
//## @ingroup IO
class MITK_CORE_EXPORT VtkVolumeTimeSeriesReader : public SurfaceSource, public FileSeriesReader
{
public:
    mitkClassMacro( VtkVolumeTimeSeriesReader, FileReader );

    /** Method for creation through the object factory. */
    itkNewMacro( Self );
    
    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

protected:
    virtual void GenerateData();

    VtkVolumeTimeSeriesReader();

    ~VtkVolumeTimeSeriesReader();

    //##Description
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

};

} // namespace mitk

#endif 


