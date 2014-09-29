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

#ifndef mitkDICOMITKSeriesGDCMReader_h
#define mitkDICOMITKSeriesGDCMReader_h

#include "mitkDICOMFileReader.h"
#include "mitkDICOMDatasetSorter.h"

#include "mitkDICOMGDCMImageFrameInfo.h"
#include "mitkEquiDistantBlocksSorter.h"
#include "mitkNormalDirectionConsistencySorter.h"

#include "mitkITKDICOMSeriesReaderHelper.h"

#include "MitkDICOMReaderExports.h"

#include <stack>

namespace itk
{
  class TimeProbesCollectorBase;
}

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief Flexible reader based on itk::ImageSeriesReader and GDCM, for single-slice modalities like CT, MR, PET, CR, etc.

  Implements the loading processed as structured by DICOMFileReader offers configuration
  of its loading strategy.

  Documentation sections:
   - \ref DICOMITKSeriesGDCMReader_LoadingStrategy
   - \ref DICOMITKSeriesGDCMReader_ForcedConfiguration
   - \ref DICOMITKSeriesGDCMReader_UserConfiguration
   - \ref DICOMITKSeriesGDCMReader_GantryTilt
   - \ref DICOMITKSeriesGDCMReader_Testing
   - \ref DICOMITKSeriesGDCMReader_Internals
     - \ref DICOMITKSeriesGDCMReader_RelatedClasses
     - \ref DICOMITKSeriesGDCMReader_TiltInternals
     - \ref DICOMITKSeriesGDCMReader_Condensing


  \section DICOMITKSeriesGDCMReader_LoadingStrategy Loading strategy

  The set of input files is processed by a number of DICOMDatasetSorter objects which may do two sort of things:
   1. split a list of input frames into multiple lists, based on DICOM tags such as "Rows", "Columns", which cannot be mixed within a single mitk::Image
   2. sort the frames within the input lists, based on the values of DICOM tags such as "Image Position Patient"

  When the DICOMITKSeriesGDCMReader is configured with DICOMDatasetSorter%s, the list of input files is processed
  as follows:
   1. build an initial set of output groups, simply by grouping all input files.
   2. for each configured DICOMDatasetSorter, process:
     - for each output group:
       1. set this group's files as input to the sorter
       2. let the sorter sort (and split)
       3. integrate the sorter's output groups with our own output groups

  \section DICOMITKSeriesGDCMReader_ForcedConfiguration Forced Configuration

  In all cases, the reader will add two DICOMDatasetSorter objects that are required to load
  mitk::Images properly via itk::ImageSeriesReader:
   1. As a \b first step, the input files will be split into groups that are not compatible because they differ in essential aspects:
     - (0028,0010) Number of Rows
     - (0028,0011) Number of Columns
     - (0028,0030) Pixel Spacing
     - (0018,1164) Imager Pixel Spacing
     - (0020,0037) %Image Orientation (Patient)
     - (0018,0050) Slice Thickness
     - (0028,0008) Number of Frames
    2. As are two forced \b last steps:
      1. There will always be an instance of EquiDistantBlocksSorter,
         which ensures that there is an equal distance between all the frames of an Image.
         This is required to achieve correct geometrical positions in the mitk::Image,
         i.e. it is essential to be able to make measurements in images.
         - whether or not the distance is required to be orthogonal to the image planes is configured by SetFixTiltByShearing().
         - during this check, we need to tolerate some minor errors in documented vs. calculated image origins.
           The amount of tolerance can be adjusted by SetToleratedOriginOffset() and SetToleratedOriginOffsetToAdaptive().
           Please see EquiDistantBlocksSorter for more details. The default should be good for most cases.
      2. There is always an instance of NormalDirectionConsistencySorter,
         which makes the order of images go along the image normals (see NormalDirectionConsistencySorter)

  \section DICOMITKSeriesGDCMReader_UserConfiguration User Configuration

  The user of this class can add more sorting steps (similar to the one described in above section) by calling AddSortingElement().

  Usually, an application will add sorting by "Image Position Patient", by "Instance Number", and by other relevant tags here.

  \section DICOMITKSeriesGDCMReader_GantryTilt Gantry tilt handling

  When CT gantry tilt is used, the gantry plane (= X-Ray source and detector ring) and the vertical plane do not align
  anymore. This scanner feature is used for example to reduce metal artifacs (e.g. <i>Lee C , Evaluation of Using CT
  Gantry Tilt Scan on Head and Neck Cancer Patients with Dental Structure: Scans Show Less Metal Artifacts. Presented
  at: Radiological Society of North America 2011 Scientific Assembly and Annual Meeting; November 27- December 2,
  2011 Chicago IL.</i>).

  The acquired planes of such CT series do not match the expectations of a orthogonal geometry in mitk::Image: if you
  stack the slices, they show a small shift along the Y axis:
\verbatim

    without tilt       with tilt

      ||||||             //////
      ||||||            //////
  --  |||||| --------- ////// -------- table orientation
      ||||||          //////
      ||||||         //////

  Stacked slices:

    without tilt       with tilt

   --------------    --------------
   --------------     --------------
   --------------      --------------
   --------------       --------------
   --------------        --------------

\endverbatim


   As such gemetries do not "work" in conjunction with mitk::Image, DICOMITKSeriesGDCMReader is able to perform a correction for such series.
   Whether or not such correction should be attempted is controlled by SetFixTiltByShearing(), the default being correction.
   For details, see "Internals" below.
  \section DICOMITKSeriesGDCMReader_Testing Testing

  A number of tests is implemented in module DICOMTesting, which is documented at \ref DICOMTesting.

  \section DICOMITKSeriesGDCMReader_Internals Class internals

  Internally, the class is based on GDCM and it depends heavily on the gdcm::Scanner class.

  Since the sorting elements (see DICOMDatasetSorter and DICOMSortCriterion) can access tags only via the DICOMDatasetAccess interface,
  BUT DICOMITKSeriesGDCMReader holds a list of more specific classes DICOMGDCMImageFrameInfo, we must convert between the two
  types sometimes. This explains the methods ToDICOMDatasetList(), FromDICOMDatasetList().

  The intermediate result of all the sorting efforts is held in m_SortingResultInProgress,
  which is modified through InternalExecuteSortingStep().

  \subsection DICOMITKSeriesGDCMReader_RelatedClasses Overview of related classes

  The following diagram gives an overview of the related classes:

  \image html implementeditkseriesgdcmreader.jpg

  \subsection DICOMITKSeriesGDCMReader_TiltInternals Details about the tilt correction

  The gantry tilt "correction" algorithm fixes two errors introduced by ITK's ImageSeriesReader:
    - the plane shift that is ignored by ITK's reader is recreated by applying a shearing transformation using itk::ResampleFilter.
    - the spacing is corrected (it is calculated by ITK's reader from the distance between two origins, which is NOT the slice distance in this special case)

  Both errors are introduced in
  itkImageSeriesReader.txx (ImageSeriesReader<TOutputImage>::GenerateOutputInformation(void)), lines 176 to 245 (as of ITK 3.20)

  For the correction, we examine two consecutive slices of a series, both described as a pair (origin/orientation):
    - we calculate if the first origin is on a line along the normal of the second slice
      - if this is not the case, the geometry will not fit a normal mitk::Image/mitk::Geometry3D
      - we then project the second origin into the first slice's coordinate system to quantify the shift
      - both is done in class GantryTiltInformation with quite some comments.

  The geometry of image stacks with tilted geometries is illustrated below:
    - green: the DICOM images as described by their tags: origin as a point with the line indicating the orientation
    - red: the output of ITK ImageSeriesReader: wrong, larger spacing, no tilt
    - blue: how much a shear must correct

  \image html tilt-correction.jpg

  \subsection DICOMITKSeriesGDCMReader_Condensing Sub-classes can condense multiple blocks into a single larger block

  The sorting/splitting process described above is helpful for at least two more DICOM readers, which either try to load 3D+t images or which load diffusion data.

  In both cases, a single pixel of the mitk::Image is made up of multiple values, in one case values over time, in the other case multiple measurements of a single point.

  The specialized readers for these cases (e.g. ThreeDnTDICOMSeriesReader) can reuse most of the methods in DICOMITKSeriesGDCMReader,
  except that they need an extra step after the usual sorting, in which they can merge already grouped 3D blocks. What blocks are merged
  depends on the specialized reader's understanding of these images. To allow for such merging, a method Condense3DBlocks() is called
  as an absolute last step of AnalyzeInputFiles(). Given this, a sub-class could implement only LoadImages() and Condense3DBlocks() instead
  repeating most of AnalyzeInputFiles().

*/
class MitkDICOMReader_EXPORT DICOMITKSeriesGDCMReader : public DICOMFileReader
{
  public:

    mitkClassMacro( DICOMITKSeriesGDCMReader, DICOMFileReader );
    mitkCloneMacro( DICOMITKSeriesGDCMReader );
    itkNewMacro( DICOMITKSeriesGDCMReader );
    mitkNewMacro1Param( DICOMITKSeriesGDCMReader, unsigned int );

    /**
      \brief Runs the sorting / splitting process described in \ref DICOMITKSeriesGDCMReader_LoadingStrategy.
      Method required by DICOMFileReader.
    */
    virtual void AnalyzeInputFiles();

    // void AllocateOutputImages();
    /**
      \brief Loads images using itk::ImageSeriesReader, potentially applies shearing to correct gantry tilt.
    */
    virtual bool LoadImages();

    // re-implemented from super-class
    virtual bool CanHandleFile(const std::string& filename);

    /**
      \brief Add an element to the sorting procedure described in \ref DICOMITKSeriesGDCMReader_LoadingStrategy.
    */
    virtual void AddSortingElement(DICOMDatasetSorter* sorter, bool atFront = false);

    typedef const std::list<DICOMDatasetSorter::ConstPointer> ConstSorterList;
    ConstSorterList GetFreelyConfiguredSortingElements() const;

    /**
      \brief Controls whether to "fix" tilted acquisitions by shearing the output (see \ref DICOMITKSeriesGDCMReader_GantryTilt).
    */
    void SetFixTiltByShearing(bool on);

    bool GetFixTiltByShearing() const;

    /**
      \brief Controls whether groups of only two images are accepted when ensuring consecutive slices via EquiDistantBlocksSorter.
    */
    void SetAcceptTwoSlicesGroups(bool accept);
    bool GetAcceptTwoSlicesGroups() const;

    /**
      \brief See \ref DICOMITKSeriesGDCMReader_ForcedConfiguration.
    */
    void SetToleratedOriginOffsetToAdaptive(double fractionOfInterSliceDistanct = 0.3);

    /**
      \brief See \ref DICOMITKSeriesGDCMReader_ForcedConfiguration.
    */
    void SetToleratedOriginOffset(double millimeters = 0.005);

    double GetToleratedOriginError() const;
    bool IsToleratedOriginOffsetAbsolute() const;

    double GetDecimalPlacesForOrientation() const;

    virtual bool operator==(const DICOMFileReader& other) const;

    virtual DICOMTagList GetTagsOfInterest() const;

  protected:

    virtual void InternalPrintConfiguration(std::ostream& os) const;

    /// \brief Return active C locale
    std::string GetActiveLocale() const;
    /**
      \brief Remember current locale on stack, activate "C" locale.
      "C" locale is required for correct parsing of numbers by itk::ImageSeriesReader
    */
    void PushLocale() const;
    /**
      \brief Activate last remembered locale from locale stack
      "C" locale is required for correct parsing of numbers by itk::ImageSeriesReader
    */
    void PopLocale() const;

    DICOMITKSeriesGDCMReader(unsigned int decimalPlacesForOrientation = 5);
    virtual ~DICOMITKSeriesGDCMReader();

    DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other);
    DICOMITKSeriesGDCMReader& operator=(const DICOMITKSeriesGDCMReader& other);

    /// \brief See \ref DICOMITKSeriesGDCMReader_Internals
    DICOMDatasetList ToDICOMDatasetList(const DICOMGDCMImageFrameList& input);
    /// \brief See \ref DICOMITKSeriesGDCMReader_Internals
    DICOMGDCMImageFrameList FromDICOMDatasetList(const DICOMDatasetList& input);
    /// \brief See \ref DICOMITKSeriesGDCMReader_Internals
    DICOMImageFrameList ToDICOMImageFrameList(const DICOMGDCMImageFrameList& input);

    typedef std::list<DICOMGDCMImageFrameList> SortingBlockList;
    /**
      \brief "Hook" for sub-classes, see \ref DICOMITKSeriesGDCMReader_Condensing
      \return REMAINING blocks
    */
    virtual SortingBlockList Condense3DBlocks(SortingBlockList& resultOf3DGrouping);

    virtual DICOMTagCache::Pointer GetTagCache() const;
    void SetTagCache(DICOMTagCache::Pointer);

    /// \brief Sorting step as described in \ref DICOMITKSeriesGDCMReader_LoadingStrategy
    SortingBlockList InternalExecuteSortingStep(
        unsigned int sortingStepIndex,
        DICOMDatasetSorter::Pointer sorter,
        const SortingBlockList& input);

    /// \brief Loads the mitk::Image by means of an itk::ImageSeriesReader
    virtual bool LoadMitkImageForOutput(unsigned int o);

    virtual bool LoadMitkImageForImageBlockDescriptor(DICOMImageBlockDescriptor& block) const;

    /**
      \brief Shear the loaded mitk::Image to "correct" a spatial error introduced by itk::ImageSeriesReader
      See \ref DICOMITKSeriesGDCMReader_GantryTilt for details.
    */
    Image::Pointer FixupSpacing(Image* mitkImage, const DICOMImageBlockDescriptor& block) const;

    /// \brief Describe this reader's confidence for given SOP class UID
    ReaderImplementationLevel GetReaderImplementationLevel(const std::string sopClassUID) const;
  private:

    /// \brief Creates the required sorting steps described in \ref DICOMITKSeriesGDCMReader_ForcedConfiguration
    void EnsureMandatorySortersArePresent(unsigned int decimalPlacesForOrientation);

  protected:

    // NOT nice, made available to ThreeDnTDICOMSeriesReader due to lack of time
    bool m_FixTiltByShearing; // could be removed by ITKDICOMSeriesReader NOT flagging tilt unless requested to fix it!

  private:

    SortingBlockList m_SortingResultInProgress;

    typedef std::list<DICOMDatasetSorter::Pointer> SorterList;
    SorterList m_Sorter;

  protected:

    // NOT nice, made available to ThreeDnTDICOMSeriesReader and ClassicDICOMSeriesReader due to lack of time
    mitk::EquiDistantBlocksSorter::Pointer m_EquiDistantBlocksSorter;

    mitk::NormalDirectionConsistencySorter::Pointer m_NormalDirectionConsistencySorter;

  private:

    mutable std::stack<std::string> m_ReplacedCLocales;
    mutable std::stack<std::locale> m_ReplacedCinLocales;

    double m_DecimalPlacesForOrientation;

    DICOMTagCache::Pointer m_TagCache;
};

}

#endif
