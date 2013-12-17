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

#ifndef mitkThreeDnTDICOMSeriesReader_h
#define mitkThreeDnTDICOMSeriesReader_h

#include "mitkDICOMITKSeriesGDCMReader.h"

#include "DICOMReaderExports.h"

namespace mitk
{

/*
   \brief Extends sorting/grouping to ThreeD+t image blocks.
*/
class DICOMReader_EXPORT ThreeDnTDICOMSeriesReader : public DICOMITKSeriesGDCMReader
{
  public:

    mitkClassMacro( ThreeDnTDICOMSeriesReader, DICOMITKSeriesGDCMReader );
    mitkCloneMacro( ThreeDnTDICOMSeriesReader );
    itkNewMacro( ThreeDnTDICOMSeriesReader );

    void SetGroup3DandT(bool on);

    // void AllocateOutputImages();
    virtual bool LoadImages();


  protected:

    ThreeDnTDICOMSeriesReader();
    virtual ~ThreeDnTDICOMSeriesReader();

    ThreeDnTDICOMSeriesReader(const ThreeDnTDICOMSeriesReader& other);
    ThreeDnTDICOMSeriesReader& operator=(const ThreeDnTDICOMSeriesReader& other);

    virtual SortingBlockList Condense3DBlocks(SortingBlockList&);

    bool LoadMitkImageForOutput(unsigned int o);

    bool m_Group3DandT;
};

}

#endif
