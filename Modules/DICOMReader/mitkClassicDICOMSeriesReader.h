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

#include "mitkThreeDnTDICOMSeriesReader.h"

#include "MitkDICOMReaderExports.h"

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief Sorting and grouping like mitk::DicomSeriesReader until 2013.

  This class implements the same functionality as the legacy class DicomSeriesReader,
  except that is is 75 lines instead of 2500 lines.
  \warning Since the old class is known to have problems with some series,
           it is advised to use a good configuration of DICOMITKSeriesGDCMReader,
           which can be obtained by using DICOMFileReaderSelector.

  The following text documents the actual sorting logic of this reader.

  The class groups datasets that have different values in any of the following tags:
   - (0028,0010) Number of Rows
   - (0028,0011) Number of Columns
   - (0028,0030) Pixel Spacing
   - (0018,1164) Imager Pixel Spacing
   - (0020,0037) %Image Orientation (Patient)
   - (0018,0050) Slice Thickness
   - (0028,0008) Number of Frames
   - (0020,000e) Series Instance UID

  Within each of the groups, datasets are sorted by the value of the following tags (primary sorting first):
   - (0020,0032) %Image Position (Patient) (distance from zero along normal of (0020,0037) %Image Orientation (Patient))
   - (0020,0012) Aqcuisition Number
   - (0008,0032) Aqcuisition Time
   - (0018,1060) Trigger Time
   - (0008,0018) SOP Instance UID (last resort, not really meaningful but decides clearly)

  If the series was acquired using a tilted gantry, this will be "fixed" by applying a shear transformation.

  If multiple images occupy the same position in space, it is assumed that this indicated a 3D+t image.

*/
class MitkDICOMReader_EXPORT ClassicDICOMSeriesReader : public ThreeDnTDICOMSeriesReader
{
  public:

    mitkClassMacro( ClassicDICOMSeriesReader, DICOMITKSeriesGDCMReader );
    mitkCloneMacro( ClassicDICOMSeriesReader );
    itkNewMacro( ClassicDICOMSeriesReader );

    virtual bool operator==(const DICOMFileReader& other) const;

  protected:

    ClassicDICOMSeriesReader();
    virtual ~ClassicDICOMSeriesReader();

    ClassicDICOMSeriesReader(const ClassicDICOMSeriesReader& other);
    ClassicDICOMSeriesReader& operator=(const ClassicDICOMSeriesReader& other);
};

}

#endif

