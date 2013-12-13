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

#ifndef mitkClassicDICOMSeriesReader_h
#define mitkClassicDICOMSeriesReader_h

#include "mitkDICOMITKSeriesGDCMReader.h"

#include "DICOMReaderExports.h"

namespace mitk
{

/*
   \brief Sorting and grouping like mitk::DicomSeriesReader until 2013.
*/
class DICOMReader_EXPORT ClassicDICOMSeriesReader : public DICOMITKSeriesGDCMReader
{
  public:

    mitkClassMacro( ClassicDICOMSeriesReader, DICOMITKSeriesGDCMReader );
    mitkCloneMacro( ClassicDICOMSeriesReader );
    itkNewMacro( ClassicDICOMSeriesReader );

  protected:

    ClassicDICOMSeriesReader();
    virtual ~ClassicDICOMSeriesReader();

    ClassicDICOMSeriesReader(const ClassicDICOMSeriesReader& other);
    ClassicDICOMSeriesReader& operator=(const ClassicDICOMSeriesReader& other);
};

}

#endif

