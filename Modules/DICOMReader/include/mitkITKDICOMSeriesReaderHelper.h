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

/* Forward deceleration of an DCMTK class. Used in the txx but part of the interface.*/
class OFDateTime;

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

    typedef std::vector<TimeBounds> TimeBoundsList;
    typedef itk::FixedArray<OFDateTime,2>  DateTimeBounds;

    /** Scans the given files for the acquisition time and returns the lowest and
     highest acquisition time as time bounds via bounds. If no acquisition times can be found
     the function return will be false.
     */
    static bool ExtractTimeBoundsOfTimeStep (const StringContainer& filenamesOfTimeStep, DateTimeBounds& bounds);

    /** Returns the list of time bounds of all passed time step containers.
     (sa ExtracTimeBoundsOfTimeStep). Time steps where no time bounds could be extracted
     are indecated by "null" time bounds (both values "0"). The order of the returned
     list equals of passed filenamesOfTimeSteps order.*/
    static TimeBoundsList ExtractTimeBoundsOfTimeSteps (const StringContainerList& filenamesOfTimeSteps);

    /** Helper function that generates  a time geometry using the template and the passed boundslist
        (which indicates the number of time steps).
    */
    static TimeGeometry::Pointer GenerateTimeGeometry(const BaseGeometry* templateGeometry, const TimeBoundsList& boundsList);

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
