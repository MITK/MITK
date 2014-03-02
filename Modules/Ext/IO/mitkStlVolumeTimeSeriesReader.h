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


#ifndef _Stl_VOLUME_TIME_SERIES_READER__H_
#define _Stl_VOLUME_TIME_SERIES_READER__H_

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkFileSeriesReader.h"
#include "mitkSurfaceSource.h"

namespace mitk
{
//##Documentation
//## @brief Reader to read a series of volume files in stl-format
//## @ingroup IO
class MitkExt_EXPORT StlVolumeTimeSeriesReader : public SurfaceSource, public FileSeriesReader
{
public:
    mitkClassMacro( StlVolumeTimeSeriesReader, FileReader );

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

    StlVolumeTimeSeriesReader();

    ~StlVolumeTimeSeriesReader();

    //##Description
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

};

} // namespace mitk

#endif


