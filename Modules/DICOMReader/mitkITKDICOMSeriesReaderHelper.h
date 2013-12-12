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

#ifndef mitkDICOMSeriesReaderHelper_h
#define mitkDICOMSeriesReaderHelper_h

#include "mitkImage.h"
#include "mitkGantryTiltInformation.h"

#include <itkGDCMImageIO.h>

namespace mitk
{

class ITKDICOMSeriesReaderHelper
{
  public:

    typedef std::vector<std::string> StringContainer;

    typedef itk::GDCMImageIO DcmIoType; // TODO remove, we are NOT flexible here


    Image::Pointer Load( const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo );

    template <typename ImageType>
    typename ImageType::Pointer
    // TODO this is NOT inplace!
    InPlaceFixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo );

    template <typename PixelType>
    Image::Pointer
    LoadDICOMByITK( const StringContainer& filenames,
                    bool correctTilt,
                    const GantryTiltInformation& tiltInfo,
                    DcmIoType::Pointer& io,
                    Image::Pointer preLoadedImageBlock );

};

}

#endif
