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
    typedef std::list<StringContainer> StringContainerList;

    Image::Pointer Load( const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo );
    Image::Pointer Load3DnT( const StringContainerList& filenamesLists, bool correctTilt, const GantryTiltInformation& tiltInfo );

    static bool CanHandleFile(const std::string& filename);

  private:

    template <typename ImageType>
    typename ImageType::Pointer
    FixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo );

    template <typename PixelType>
    Image::Pointer
    LoadDICOMByITK( const StringContainer& filenames,
                    bool correctTilt,
                    const GantryTiltInformation& tiltInfo,
                    itk::GDCMImageIO::Pointer& io);

    template <typename PixelType>
    Image::Pointer
    LoadDICOMByITK3DnT( const StringContainerList& filenames,
                        bool correctTilt,
                        const GantryTiltInformation& tiltInfo,
                        itk::GDCMImageIO::Pointer& io);


};

}

#endif
