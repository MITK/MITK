/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkITKDICOMSeriesReaderHelper_h
#define mitkITKDICOMSeriesReaderHelper_h

#include <mitkImage.h>
#include <mitkGantryTiltInformation.h>
#include <mitkDICOMTag.h>

#include <itkGDCMImageIO.h>

/* Forward deceleration of an DCMTK class. Used in the tpp but part of the interface.*/
class OFDateTime;

namespace mitk
{

/**
 * \ingroup DICOMModule
 * \brief Helper class that uses ITK's ImageSeriesReader to load DICOM images.
 *
 * This class encapsulates the actual loading of DICOM images using itk::ImageSeriesReader
 * and itk::GDCMImageIO. It handles pixel type dispatching, gantry tilt correction via
 * shearing, and time geometry generation for 3D+t datasets.
 *
 * \sa DICOMITKSeriesGDCMReader, GantryTiltInformation
 */
class ITKDICOMSeriesReaderHelper
{
  public:

    static const DICOMTag AcquisitionDateTag;  ///< DICOM tag (0008,0022) Acquisition Date.
    static const DICOMTag AcquisitionTimeTag;  ///< DICOM tag (0008,0032) Acquisition Time.
    static const DICOMTag TriggerTimeTag;      ///< DICOM tag (0018,1060) Trigger Time.

    /** \brief A list of file path strings for a single 3D volume. */
    typedef std::vector<std::string> StringContainer;
    /** \brief A list of StringContainer instances, one per time step (for 3D+t loading). */
    typedef std::list<StringContainer> StringContainerList;

    /**
     * \brief Load a single 3D DICOM image from the given files.
     * \param[in] filenames The list of DICOM file paths constituting one 3D volume.
     * \param[in] correctTilt Whether to apply gantry tilt correction via shearing.
     * \param[in] tiltInfo The gantry tilt information describing the tilt geometry.
     * \return A smart pointer to the loaded mitk::Image.
     */
    Image::Pointer Load( const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo );

    /**
     * \brief Load a 3D+t DICOM image from multiple time step file lists.
     * \param[in] filenamesLists A list of file lists, one per time step.
     * \param[in] correctTilt Whether to apply gantry tilt correction via shearing.
     * \param[in] tiltInfo The gantry tilt information describing the tilt geometry.
     * \return A smart pointer to the loaded mitk::Image with time geometry.
     */
    Image::Pointer Load3DnT( const StringContainerList& filenamesLists, bool correctTilt, const GantryTiltInformation& tiltInfo );

    /**
     * \brief Check if the given file can be handled by this reader (i.e. is readable by GDCM).
     * \param[in] filename The file path to check.
     * \return true if the file can be read as a DICOM image.
     */
    static bool CanHandleFile(const std::string& filename);

  private:

    typedef std::vector<TimeBounds> TimeBoundsList;
    typedef itk::FixedArray<OFDateTime,2>  DateTimeBounds;


    /** Scans the given files for the acquisition time and returns the lowest and
     highest acquisition date time as date time bounds via bounds.
     \param filenamesOfTimeStep The list of filenames to scan.
     \param bounds The acquisition date time bound extracted from the files.
     \param triggerBounds Time bounds for trigger information extracted from the files.
     If no trigger information was found then it returns trigger == [0.0, 0.0].
     \return If no acquisition date times can be found the function return will be false. Otherwise
     it returns true.
     */
    static bool ExtractDateTimeBoundsAndTriggerOfTimeStep( const StringContainer& filenamesOfTimeStep,
      DateTimeBounds& bounds, TimeBounds& triggerBounds);

    /** Determine the time bounds in ms respective to the baselineDateTime for the passed
    files. Additionally it regards the trigger time tag if set and acquisition date time
    carries not enough information.
    \param filenamesOfTimeStep The list of filenames for the time step.
    \param bounds The extracted time bounds (output).
    \param baselineDateTime The reference baseline date time.
    \param [in,out] usedTriggerBounds Variable used to communicate if preceding time bounds extractions
    used the trigger time (true). If true is passed to the method, it will use always trigger time
    to extract the next bounds. Default is false. After the method call is returning the variable indicates
    the last call used trigger time or not to extract the time bounds.*/
    static bool ExtractTimeBoundsOfTimeStep(const StringContainer& filenamesOfTimeStep,
                                                 TimeBounds& bounds,
                                                 const OFDateTime& baselineDateTime,
                                                 bool& usedTriggerBounds);

    /** Returns the list of time bounds of all passed time step containers.
     \sa ExtractTimeBoundsOfTimeStep, ExtractDateTimeBoundsAndTriggerOfTimeStep
     Time steps where no time bounds could be extracted
     are indicated by "null" time bounds (both values "0"). The order of the returned
     list equals of passed filenamesOfTimeSteps order.
     \remark The function regards acquisition date time tags and trigger time tags.*/
    static TimeBoundsList ExtractTimeBoundsOfTimeSteps (const StringContainerList& filenamesOfTimeSteps);

    /** Helper function that generates  a time geometry using the template and the passed bounds list
        (which indicates the number of time steps).
    */
    static TimeGeometry::Pointer GenerateTimeGeometry(const BaseGeometry* templateGeometry, const TimeBoundsList& boundsList);

    template <typename ImageType>
    typename ImageType::Pointer
    FixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo );

    template <typename PixelType, unsigned int TDim>
    Image::Pointer
    LoadDICOMByITK( const StringContainer& filenames,
                    bool correctTilt,
                    const GantryTiltInformation& tiltInfo,
                    itk::GDCMImageIO::Pointer& io);

    template<unsigned int TDim>
    mitk::Image::Pointer
    LoadByTypeDispatch(const StringContainer& filenames,
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
