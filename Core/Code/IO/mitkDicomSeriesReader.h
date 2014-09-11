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

#ifndef mitkDicomSeriesReader_h
#define mitkDicomSeriesReader_h

#include "mitkDataNode.h"
#include "mitkConfig.h"

#include <itkGDCMImageIO.h>

#include <itkImageSeriesReader.h>
#include <itkCommand.h>

#ifdef NOMINMAX
#  define DEF_NOMINMAX
#  undef NOMINMAX
#endif

#include <gdcmConfigure.h>

#ifdef DEF_NOMINMAX
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  undef DEF_NOMINMAX
#endif

#include <gdcmDataSet.h>
#include <gdcmScanner.h>

namespace mitk
{

/**
 \brief Loading DICOM images as MITK images.

 - \ref DicomSeriesReader_purpose
 - \ref DicomSeriesReader_limitations
 - \ref DicomSeriesReader_usage
 - \ref DicomSeriesReader_sorting
   - \ref DicomSeriesReader_sorting1
   - \ref DicomSeriesReader_sorting2
   - \ref DicomSeriesReader_sorting3
   - \ref DicomSeriesReader_sorting4
 - \ref DicomSeriesReader_gantrytilt
 - \ref DicomSeriesReader_pixelspacing
 - \ref DicomSeriesReader_nextworkitems
 - \ref DicomSeriesReader_whynotinitk
 - \ref DicomSeriesReader_tests

 \section DicomSeriesReader_purpose Purpose

 DicomSeriesReader serves as a central class for loading DICOM images as mitk::Image.

 As the term "DICOM image" covers a huge variety of possible modalities and
 implementations, and since MITK assumes that 3D images are made up of continuous blocks
 of slices without any gaps or changes in orientation, the loading mechanism must
 implement a number of decisions and compromises.

 <b>The main intention of this implementation is not efficiency but correctness of generated slice positions and pixel spacings!</b>

 \section DicomSeriesReader_limitations Assumptions and limitations

 The class is working only with GDCM 2.0.14 (or possibly newer). This version is the
 default of an MITK super-build. Support for other versions or ITK's DicomIO was dropped
 because of the associated complexity of DicomSeriesReader.

 \b Assumptions
  - expected to work with certain SOP Classes (mostly CT Image Storage and MR Image Storage)
    - see ImageBlockDescriptor.GetReaderImplementationLevel() method for the details
  - special treatment for a certain type of Philips 3D ultrasound (recogized by tag 3001,0010 set to "Philips3D")
  - loader will always attempt to read multiple single slices as a single 3D image volume (i.e. mitk::Image)
    - slices will be grouped by basic properties such as orientation, rows, columns, spacing and grouped into as large blocks as possible
  - images which do NOT report a position or orientation in space (Image Position Patient, Image Orientation) will be assigned defaults
    - image position (0,0,0)
    - image orientation (1,0,0), (0,1,0)
    - such images will always be grouped separately since spatial grouping / sorting makes no sense for them

 \b Options
  - images that cover the same piece of space (i.e. position, orientation, and dimensions are equal)
    can be interpreted as time-steps of the same image, i.e. a series will be loaded as 3D+t

 \b Limitations
  - the 3D+t assumption only works if all time-steps have an equal number of slices and if all
    have the Acquisition Time attribute set to meaningful values

 \section DicomSeriesReader_usage Usage

 The starting point for an application is a set of DICOM files that should be loaded.
 For convenience, DicomSeriesReader can also parse a whole directory for DICOM files,
 but an application should better know exactly what to load.

 Loading is then done in two steps:

  1. <b>Group the files into spatial blocks</b> by calling GetSeries().
     This method will sort all passed files into meaningful blocks that
     could fit into an mitk::Image. Sorting for 3D+t loading is optional but default.
     The \b return value of this function is a list of descriptors, which
     describe a grouped list of files with its most basic properties:
     - SOP Class (CT Image Storage, Secondary Capture Image Storage, etc.)
     - Modality
     - What type of pixel spacing can be read from the provided DICOM tags
     - How well DicomSeriesReader is prepared to load this type of data

  2. <b>Load a sorted set of files</b> by calling LoadDicomSeries().
     This method expects go receive the sorting output of GetSeries().
     The method will then invoke ITK methods configured with GDCM-IO
     classes to actually load the files into memory and put them into
     mitk::Images. Again, loading as 3D+t is optional.

  Example:

\code

 // only a directory is known at this point: /home/who/dicom

 DicomSeriesReader::FileNamesGrouping allImageBlocks = DicomSeriesReader::GetSeries("/home/who/dicom/");

 // file now divided into groups of identical image size, orientation, spacing, etc.
 // each of these lists should be loadable as an mitk::Image.

 DicomSeriesReader::StringContainer seriesToLoad = allImageBlocks[...]; // decide what to load

 // final step: load into DataNode (can result in 3D+t image)
 DataNode::Pointer node = DicomSeriesReader::LoadDicomSeries( oneBlockSorted );

 itk::SmartPointer<Image> image = dynamic_cast<mitk::Image*>( node->GetData() );
\endcode

 \section DicomSeriesReader_sorting Logic for sorting 2D slices from DICOM images into 3D+t blocks for mitk::Image

 The general sorting mechanism (implemented in GetSeries) groups and sorts a set of DICOM files, each assumed to contain a single CT/MR slice.
 In the following we refer to those file groups as "blocks", since this is what they are meant to become when loaded into an mitk::Image.

 \subsection DicomSeriesReader_sorting1 Step 1: Avoiding pure non-sense

 A first pass separates slices that cannot possibly be loaded together because of restrictions of mitk::Image.
 After this steps, each block contains only slices that match in all of the following DICOM tags:

   - (0020,000e) Series Instance UID
   - (0020,0037) Image Orientation
   - (0028,0030) Pixel Spacing
   - (0018,1164) Imager Pixel Spacing
   - (0018,0050) Slice Thickness
   - (0028,0010) Number Of Rows
   - (0028,0011) Number Of Columns
   - (0028,0008) Number Of Frames

 \subsection DicomSeriesReader_sorting2 Step 2: Sort slices spatially

 Before slices are further analyzed, they are sorted spatially. As implemented by GdcmSortFunction(),
 slices are sorted by
   1. distance from origin (calculated using (0020,0032) Image Position Patient and (0020,0037) Image Orientation)
   2. when distance is equal, (0020,0012) Aquisition Number, (0008,0032) Acquisition Time and (0018,1060) Trigger Time are
      used as a backup criterions (necessary for meaningful 3D+t sorting)

 \subsection DicomSeriesReader_sorting3 Step 3: Ensure equal z spacing

 Since inter-slice distance is not recorded in DICOM tags, we must ensure that blocks are made up of
 slices that have equal distances between neighboring slices. This is especially necessary because itk::ImageSeriesReader
 is later used for the actual loading, and this class expects (and does nocht verify) equal inter-slice distance (see \ref DicomSeriesReader_whatweknowaboutitk).

 To achieve such grouping, the inter-slice distance is calculated from the first two different slice positions of a block.
 Following slices are added to a block as long as they can be added by adding the calculated inter-slice distance to the
 last slice of the block. Slices that do not fit into the expected distance pattern, are set aside for further analysis.
 This grouping is done until each file has been assigned to a group.

 Slices that share a position in space are also sorted into separate blocks during this step.
 So the result of this step is a set of blocks that contain only slices with equal z spacing
 and uniqe slices at each position.

 \subsection DicomSeriesReader_sorting4 Step 4 (optional): group 3D blocks as 3D+t when possible

 This last step depends on an option of GetSeries(). When requested, image blocks from the previous step are merged again
 whenever two blocks occupy the same portion of space (i.e. same origin, number of slices and z-spacing).

 \section DicomSeriesReader_gantrytilt Handling of gantry tilt

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


 As such gemetries do not in conjunction with mitk::Image, DicomSeriesReader performs a correction for such series
 if the groupImagesWithGantryTilt or correctGantryTilt flag in GetSeries and LoadDicomSeries is set (default = on).

 The correction algorithms undoes two errors introduced by ITK's ImageSeriesReader:
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

  \image tilt-correction.jpg

 \section DicomSeriesReader_whatweknowaboutitk The actual image loading process

 When calling LoadDicomSeries(), this method "mainly" uses an instance of itk::ImageSeriesReader,
 configured with an itk::GDCMImageIO object. Because DicomSeriesReader works around some of the
 behaviors of these classes, the following is a list of features that we find in the code and need to work with:

  - itk::ImageSeriesReader::GenerateOutputInformation() does the z-spacing handling
    + spacing is directly determined by comparing (euclidean distance) the origins of the first two slices of a series
      * this is GOOD because there is no reliable z-spacing information in DICOM images
      * this is bad because it does not work with gantry tilt, in which case the slice distance is SMALLER than the distance between two origins (see section on tilt)
  - origin and spacing are calculated by GDCMImageIO and re-used in itk::ImageSeriesReader
    + the origins are read from appropriate tags, nothing special about that
    + the spacing is read by gdcm::ImageReader, gdcm::ImageHelper::GetSpacingValue() from a tag determined by gdcm::ImageHelper::GetSpacingTagFromMediaStorage(), which basically determines ONE appropriate pixel spacing tag for each media storage type (ct image, mr image, secondary capture image, etc.)
      * this is fine for modalities such as CT/MR where the "Pixel Spacing" tag is mandatory, but for other modalities such as CR or Secondary Capture, the tag "Imager Pixel Spacing" is taken, which is no only optional but also has a more complicated relation with the "Pixel Spacing" tag. For this reason we check/modify the pixel spacing reported by itk::ImageSeriesReader after loading the image (see \ref DicomSeriesReader_pixelspacing)

 AFTER loading, DicomSeriesReader marks some of its findings as mitk::Properties to the loaded Image and DataNode:
  - <b>dicomseriesreader.SOPClass</b> : DICOM SOP Class as readable string (instead of a UID)
  - <b>dicomseriesreader.ReaderImplementationLevelString</b> : Confidence /Support level of the reader for this image as readable string
  - <b>dicomseriesreader.ReaderImplementationLevel</b> : Confidence /Support level of the reader for this image as enum value of type ReaderImplementationLevel
  - <b>dicomseriesreader.PixelSpacingInterpretationString</b> : Appropriate interpreteation of pixel spacing for this Image as readable string
  - <b>dicomseriesreader.PixelSpacingInterpretation</b> : Appropriate interpreteation of pixel spacing for this Image as enum value of type PixelSpacingInterpretation
  - <b>dicomseriesreader.MultiFrameImage</b> : bool flag to mark multi-frame images
  - <b>dicomseriesreader.GantyTiltCorrected</b> : bool flag to mark images where a gantry tilt was corrected to fit slices into an mitk::Image
  - <b>dicomseriesreader.3D+t</b> : bool flag to mark images with a time dimension (multiple 3D blocks of the same size at the same position in space)

 \section DicomSeriesReader_pixelspacing Handling of pixel spacing

 The reader implementes what is described in DICOM Part 3, chapter 10.7 (Basic Pixel Spacing Calibration Macro): Both tags
  - (0028,0030) Pixel Spacing and
  - (0018,1164) Imager Pixel Spacing

 are evaluated and the pixel spacing is set to the spacing within the patient when tags allow that.
 The result of pixel spacing interpretation can be read from a property "dicomseriesreader.PixelSpacingInterpretation",
 which refers to one of the enumerated values of type PixelSpacingInterpretation;

 \section DicomSeriesReader_supportedmodalities Limitations for specific modalities

  - <b>Enhanced Computed Tomography / Magnetic Resonance Images</b> are currently NOT supported at all, because we lack general support for multi-frame images.
  - <b>Nuclear Medicine Images</b> are not supported fully supported, only the single-frame variants are loaded properly.

 \section DicomSeriesReader_nextworkitems Possible enhancements

  This is a short list of ideas for enhancement:
   - Class has historically grown and should be reviewed again. There is probably too many duplicated scanning code
   - Multi-frame images don't mix well with the curent assumption of "one file - one slice", which is assumed by our code
     - It should be checked how well GDCM and ITK support these files (some load, some don't)
   - Specializations such as the Philips 3D code should be handled in a more generic way. The current handling of Philips 3D images is not nice at all

 \section DicomSeriesReader_whynotinitk Why is this not in ITK?

  Some of this code would probably be better located in ITK. It is just a matter of resources that this is not the
  case yet. Any attempts into this direction are welcome and can be supported. At least the gantry tilt correction
  should be a simple addition to itk::ImageSeriesReader.

 \section DicomSeriesReader_tests Tests regarding DICOM loading

 A number of tests have been implemented to check our assumptions regarding DICOM loading. Please see \ref DICOMTesting

 \todo refactor all the protected helper objects/methods into a separate header so we compile faster
*/

class Image;

class MITK_CORE_EXPORT DicomSeriesReader
{
public:

  /**
    \brief Lists of filenames.
  */
  typedef std::vector<std::string> StringContainer;

  /**
    \brief Interface for the progress callback.
  */
  typedef void (*UpdateCallBackMethod)(float);

  /**
    \brief Describes how well the reader is tested for a certain file type.

    Applications should not rely on the outcome for images which are reported
    ReaderImplementationLevel_Implemented or ReaderImplementationLevel_Unsupported.

    Errors to load images which are reported as ReaderImplementationLevel_Supported
    are considered bugs. For ReaderImplementationLevel_PartlySupported please check the appropriate paragraph in \ref DicomSeriesReader_supportedmodalities
  */
  typedef enum
  {
    ReaderImplementationLevel_Supported,       /// loader code and tests are established
    ReaderImplementationLevel_PartlySupported, /// loader code and tests are establised for specific parts of a SOP Class
    ReaderImplementationLevel_Implemented,     /// loader code is implemented but not accompanied by tests
    ReaderImplementationLevel_Unsupported,     /// loader code is not working with this SOP Class
  } ReaderImplementationLevel;

  /**
    \brief How the mitk::Image spacing should be interpreted.

    Compare DICOM PS 3.3 10.7 (Basic Pixel Spacing Calibration Macro).
  */
  typedef enum
  {
    PixelSpacingInterpretation_SpacingInPatient,  /// distances are mm within a patient
    PixelSpacingInterpretation_SpacingAtDetector, /// distances are mm at detector surface
    PixelSpacingInterpretation_SpacingUnknown     /// NO spacing information is present, we use (1,1) as default
  } PixelSpacingInterpretation;

  /**
    \brief Return type of GetSeries, describes a logical group of files.

    Files grouped into a single 3D or 3D+t block are described by an instance
    of this class. Relevant descriptive properties can be used to provide
    the application user with meaningful choices.
  */
  class MITK_CORE_EXPORT ImageBlockDescriptor
  {
    public:

      /// List of files in this group
      StringContainer GetFilenames() const;

      /// A unique ID describing this bloc (enhanced Series Instance UID).
      std::string GetImageBlockUID() const;

      /// The Series Instance UID.
      std::string GetSeriesInstanceUID() const;

      /// Series Modality (CT, MR, etc.)
      std::string GetModality() const;

      /// SOP Class UID as readable string (Computed Tomography Image Storage, Secondary Capture Image Storage, etc.)
      std::string GetSOPClassUIDAsString() const;

      /// SOP Class UID as DICOM UID
      std::string GetSOPClassUID() const;

      /// Confidence of the reader that this block can be read successfully.
      ReaderImplementationLevel GetReaderImplementationLevel() const;

      /// Whether or not the block contains a gantry tilt which will be "corrected" during loading
      bool HasGantryTiltCorrected() const;

      /// Whether or not mitk::Image spacing relates to the patient
      bool PixelSpacingRelatesToPatient() const;
      /// Whether or not mitk::Image spacing relates to the detector surface
      bool PixelSpacingRelatesToDetector() const;
      /// Whether or not mitk::Image spacing is of unknown origin
      bool PixelSpacingIsUnknown() const;

      /// How the mitk::Image spacing can meaningfully be interpreted.
      PixelSpacingInterpretation GetPixelSpacingType() const;

      /// 3D+t or not
      bool HasMultipleTimePoints() const;

      /// Multi-frame image(s) or not
      bool IsMultiFrameImage() const;

      ImageBlockDescriptor();
      ~ImageBlockDescriptor();

    private:

      friend class DicomSeriesReader;

      ImageBlockDescriptor(const StringContainer& files);

      void AddFile(const std::string& file);
      void AddFiles(const StringContainer& files);


      void SetImageBlockUID(const std::string& uid);

      void SetSeriesInstanceUID(const std::string& uid);

      void SetModality(const std::string& modality);

      void SetNumberOfFrames(const std::string& );

      void SetSOPClassUID(const std::string& mediaStorageSOPClassUID);

      void SetHasGantryTiltCorrected(bool);

      void SetPixelSpacingInformation(const std::string& pixelSpacing, const std::string& imagerPixelSpacing);

      void SetHasMultipleTimePoints(bool);

      void GetDesiredMITKImagePixelSpacing(ScalarType& spacingX, ScalarType& spacingY) const;

      StringContainer m_Filenames;
      std::string m_ImageBlockUID;
      std::string m_SeriesInstanceUID;
      std::string m_Modality;
      std::string m_SOPClassUID;
      bool m_HasGantryTiltCorrected;
      std::string m_PixelSpacing;
      std::string m_ImagerPixelSpacing;
      bool m_HasMultipleTimePoints;
      bool m_IsMultiFrameImage;
  };

  typedef std::map<std::string, ImageBlockDescriptor> FileNamesGrouping;

  /**
    \brief Provide combination of preprocessor defines that was active during compilation.

    Since this class is a combination of several possible implementations, separated only
    by ifdef's, calling instances might want to know which flags were active at compile time.
  */
  static std::string GetConfigurationString();

  /**
   \brief Checks if a specific file contains DICOM data.
  */
  static
  bool
  IsDicom(const std::string &filename);

  /**
   \brief see other GetSeries().

   Find all series (and sub-series -- see details) in a particular directory.
  */
  static FileNamesGrouping GetSeries(const std::string &dir,
                                   bool groupImagesWithGantryTilt,
                                   const StringContainer &restrictions = StringContainer());

  /**
   \brief see other GetSeries().

   \warning Untested, could or could not work.

   This differs only by having an additional restriction to a single known DICOM series.
   Internally, it uses the other GetSeries() method.
  */
  static StringContainer GetSeries(const std::string &dir,
                                   const std::string &series_uid,
                                   bool groupImagesWithGantryTilt,
                                   const StringContainer &restrictions = StringContainer());

  /**
   \brief PREFERRED version of this method - scan and sort DICOM files.

   Parse a list of files for images of DICOM series.
   For each series, an enumeration of the files contained in it is created.

   \return The resulting maps UID-like keys (based on Series Instance UID and slice properties) to sorted lists of file names.

   SeriesInstanceUID will be enhanced to be unique for each set of file names
   that is later loadable as a single mitk::Image. This implies that
   Image orientation, slice thickness, pixel spacing, rows, and columns
   must be the same for each file (i.e. the image slice contained in the file).

   If this separation logic requires that a SeriesInstanceUID must be made more specialized,
   it will follow the same logic as itk::GDCMSeriesFileNames to enhance the UID with
   more digits and dots.

   Optionally, more tags can be used to separate files into different logical series by setting
   the restrictions parameter.

   \warning Adding restrictions is not yet implemented!
   */
  static
  FileNamesGrouping
  GetSeries(const StringContainer& files,
            bool sortTo3DPlust,
            bool groupImagesWithGantryTilt,
            const StringContainer &restrictions = StringContainer());

  /**
    \brief See other GetSeries().

    Use GetSeries(const StringContainer& files, bool sortTo3DPlust, const StringContainer &restrictions) instead.
  */
  static
  FileNamesGrouping
  GetSeries(const StringContainer& files,
            bool groupImagesWithGantryTilt,
            const StringContainer &restrictions = StringContainer());

  /**
   Loads a DICOM series composed by the file names enumerated in the file names container.
   If a callback method is supplied, it will be called after every progress update with a progress value in [0,1].

   \param filenames The filenames to load.
   \param sort Whether files should be sorted spatially (true) or not (false - maybe useful if presorted)
   \param load4D Whether to load the files as 3D+t (if possible)
  */
  static DataNode::Pointer LoadDicomSeries(const StringContainer &filenames,
                                           bool sort = true,
                                           bool load4D = true,
                                           bool correctGantryTilt = true,
                                           UpdateCallBackMethod callback = 0,
                                           itk::SmartPointer<Image> preLoadedImageBlock = 0);

  /**
    \brief See LoadDicomSeries! Just a slightly different interface.

    If \p preLoadedImageBlock is provided, the reader will only "fake" loading and create appropriate
    mitk::Properties.
  */
  static bool LoadDicomSeries(const StringContainer &filenames,
                              DataNode &node,
                              bool sort = true,
                              bool load4D = true,
                              bool correctGantryTilt = true,
                              UpdateCallBackMethod callback = 0,
                              itk::SmartPointer<Image> preLoadedImageBlock = 0);

protected:

  /**
    \brief Return type of DicomSeriesReader::AnalyzeFileForITKImageSeriesReaderSpacingAssumption.

    Class contains the grouping result of method DicomSeriesReader::AnalyzeFileForITKImageSeriesReaderSpacingAssumption,
    which takes as input a number of images, which are all equally oriented and spatially sorted along their normal direction.

    The result contains of two blocks: a first one is the grouping result, all of those images can be loaded
    into one image block because they have an equal origin-to-origin distance without any gaps in-between.
  */
  class SliceGroupingAnalysisResult
  {
    public:

      SliceGroupingAnalysisResult();

      /**
        \brief Grouping result, all same origin-to-origin distance w/o gaps.
      */
      StringContainer GetBlockFilenames();

      /**
        \brief Remaining files, which could not be grouped.
      */
      StringContainer GetUnsortedFilenames();

      /**
        \brief Wheter or not the grouped result contain a gantry tilt.
      */
      bool ContainsGantryTilt();

      /**
        \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
      */
      void AddFileToSortedBlock(const std::string& filename);

      /**
        \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
      */
      void AddFileToUnsortedBlock(const std::string& filename);
      void AddFilesToUnsortedBlock(const StringContainer& filenames);

      /**
        \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
        \todo Could make sense to enhance this with an instance of GantryTiltInformation to store the whole result!
      */
      void FlagGantryTilt();

      /**
        \brief Only meaningful for use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption.
      */
      void UndoPrematureGrouping();

    protected:

      StringContainer m_GroupedFiles;
      StringContainer m_UnsortedFiles;

      bool m_GantryTilt;
  };

  /**
    \brief Gantry tilt analysis result.

    Takes geometry information for two slices of a DICOM series and
    calculates whether these fit into an orthogonal block or not.
    If NOT, they can either be the result of an acquisition with
    gantry tilt OR completly broken by some shearing transformation.

    Most calculations are done in the constructor, results can then
    be read via the remaining methods.
  */
  class GantryTiltInformation
  {
    public:

      // two types to avoid any rounding errors
      typedef  itk::Point<double,3> Point3Dd;
      typedef itk::Vector<double,3> Vector3Dd;

      /**
        \brief Just so we can create empty instances for assigning results later.
      */
      GantryTiltInformation();

      /**
        \brief THE constructor, which does all the calculations.

        Determining the amount of tilt is done by checking the distances
        of origin1 from planes through origin2. Two planes are considered:
         - normal vector along normal of slices (right x up): gives the slice distance
         - normal vector along orientation vector "up": gives the shift parallel to the plane orientation

        The tilt angle can then be calculated from these distances

        \param origin1 origin of the first slice
        \param origin2 origin of the second slice
        \param right right/up describe the orientatation of borth slices
        \param up right/up describe the orientatation of borth slices
        \param numberOfSlicesApart how many slices are the given origins apart (1 for neighboring slices)
      */
      GantryTiltInformation( const Point3D& origin1,
                             const Point3D& origin2,
                             const Vector3D& right,
                             const Vector3D& up,
                             unsigned int numberOfSlicesApart);

      /**
        \brief Whether the slices were sheared.

        True if any of the shifts along right or up vector are non-zero.
      */
      bool IsSheared() const;

      /**
        \brief Whether the shearing is a gantry tilt or more complicated.

        Gantry tilt will only produce shifts in ONE orientation, not in both.

        Since the correction code currently only coveres one tilt direction
        AND we don't know of medical images with two tilt directions, the
        loading code wants to check if our assumptions are true.
      */
      bool IsRegularGantryTilt() const;

      /**
        \brief The offset distance in Y direction for each slice in mm (describes the tilt result).
      */
      double GetMatrixCoefficientForCorrectionInWorldCoordinates() const;


      /**
        \brief The z / inter-slice spacing. Needed to correct ImageSeriesReader's result.
      */
      double GetRealZSpacing() const;

      /**
        \brief The shift between first and last slice in mm.

        Needed to resize an orthogonal image volume.
      */
      double GetTiltCorrectedAdditionalSize() const;

      /**
        \brief Calculated tilt angle in degrees.
      */
      double GetTiltAngleInDegrees() const;

    protected:

      /**
        \brief Projection of point p onto line through lineOrigin in direction of lineDirection.
      */
      Point3D projectPointOnLine( Point3Dd p, Point3Dd lineOrigin, Vector3Dd lineDirection );

      double m_ShiftUp;
      double m_ShiftRight;
      double m_ShiftNormal;
      double m_ITKAssumedSliceSpacing;
      unsigned int m_NumberOfSlicesApart;
  };

  /**
    \brief for internal sorting.
  */
  typedef std::pair<StringContainer, StringContainer> TwoStringContainers;

  /**
    \brief Maps DICOM tags to MITK properties.
  */
  typedef std::map<std::string, std::string> TagToPropertyMapType;

  /**
    \brief Ensure an equal z-spacing for a group of files.

    Takes as input a number of images, which are all equally oriented and spatially sorted along their normal direction.

    Internally used by GetSeries. Returns two lists: the first one contins slices of equal inter-slice spacing.
    The second list contains remaining files, which need to be run through AnalyzeFileForITKImageSeriesReaderSpacingAssumption again.

    Relevant code that is matched here is in
    itkImageSeriesReader.txx (ImageSeriesReader<TOutputImage>::GenerateOutputInformation(void)), lines 176 to 245 (as of ITK 3.20)
  */
  static
  SliceGroupingAnalysisResult
  AnalyzeFileForITKImageSeriesReaderSpacingAssumption(const StringContainer& files, bool groupsOfSimilarImages, const gdcm::Scanner::MappingType& tagValueMappings_);

  /**
    \brief Safely convert const char* to std::string.
  */
  static
  std::string
  ConstCharStarToString(const char* s);

  /**
    \brief Safely convert a string into pixel spacing x and y.
  */
  static
  bool
  DICOMStringToSpacing(const std::string& s, ScalarType& spacingX, ScalarType& spacingY);

  /**
    \brief Convert DICOM string describing a point to Point3D.

    DICOM tags like ImagePositionPatient contain a position as float numbers separated by backslashes:
    \verbatim
    42.7131\13.77\0.7
    \endverbatim
  */
  static
  Point3D
  DICOMStringToPoint3D(const std::string& s, bool& successful);

  /**
    \brief Convert DICOM string describing a point two Vector3D.

    DICOM tags like ImageOrientationPatient contain two vectors as float numbers separated by backslashes:
    \verbatim
    42.7131\13.77\0.7\137.76\0.3
    \endverbatim
  */
  static
  void
  DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful);

  template <typename ImageType>
  static
  typename ImageType::Pointer
  // TODO this is NOT inplace!
  InPlaceFixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo );


  /**
    \brief Sort a set of file names in an order that is meaningful for loading them into an mitk::Image.

    \warning This method assumes that input files are similar in basic properties such as
             slice thicknes, image orientation, pixel spacing, rows, columns.
             It should always be ok to put the result of a call to GetSeries(..) into this method.

    Sorting order is determined by

     1. image position along its normal (distance from world origin)
     2. acquisition time

    If P<n> denotes a position and T<n> denotes a time step, this method will order slices from three timesteps like this:
\verbatim
  P1T1 P1T2 P1T3 P2T1 P2T2 P2T3 P3T1 P3T2 P3T3
\endverbatim

   */
  static StringContainer SortSeriesSlices(const StringContainer &unsortedFilenames);

public:
  /**
   \brief Checks if a specific file is a Philips3D ultrasound DICOM file.
  */
  static bool IsPhilips3DDicom(const std::string &filename);

  static std::string ReaderImplementationLevelToString( const ReaderImplementationLevel& enumValue );
  static std::string PixelSpacingInterpretationToString( const PixelSpacingInterpretation& enumValue );

protected:

  /**
   \brief Read a Philips3D ultrasound DICOM file and put into an mitk::Image.
  */
  static bool ReadPhilips3DDicom(const std::string &filename, itk::SmartPointer<Image> output_image);

  /**
    \brief Construct a UID that takes into account sorting criteria from GetSeries().
  */
  static std::string CreateMoreUniqueSeriesIdentifier( gdcm::Scanner::TagToValue& tagValueMap );

  /**
    \brief Helper for CreateMoreUniqueSeriesIdentifier
  */
  static std::string CreateSeriesIdentifierPart( gdcm::Scanner::TagToValue& tagValueMap, const gdcm::Tag& tag );

  /**
    \brief Helper for CreateMoreUniqueSeriesIdentifier
  */
  static std::string IDifyTagValue(const std::string& value);

  typedef itk::GDCMImageIO DcmIoType;

  /**
    \brief Progress callback for DicomSeriesReader.
  */
  class CallbackCommand : public itk::Command
  {
  public:
    CallbackCommand(UpdateCallBackMethod callback)
      : m_Callback(callback)
    {
    }

    void Execute(const itk::Object *caller, const itk::EventObject&)
    {
      (*this->m_Callback)(static_cast<const itk::ProcessObject*>(caller)->GetProgress());
    }

    void Execute(itk::Object *caller, const itk::EventObject&)
    {
      (*this->m_Callback)(static_cast<itk::ProcessObject*>(caller)->GetProgress());
    }

  protected:

    UpdateCallBackMethod m_Callback;
  };

  static void FixSpacingInformation( Image* image, const ImageBlockDescriptor& imageBlockDescriptor );

  /**
   \brief Scan for slice image information
  */
  static void ScanForSliceInformation( const StringContainer &filenames, gdcm::Scanner& scanner );

  /**
   \brief Performs actual loading of a series and creates an image having the specified pixel type.
  */
  static
  void
  LoadDicom(const StringContainer &filenames, DataNode &node, bool sort, bool check_4d, bool correctTilt, UpdateCallBackMethod callback, itk::SmartPointer<Image> preLoadedImageBlock);

  /**
    \brief Feed files into itk::ImageSeriesReader and retrieve a 3D MITK image.

    \param command can be used for progress reporting
  */
  template <typename PixelType>
  static
  itk::SmartPointer<Image>
  LoadDICOMByITK( const StringContainer&, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);

  static
  itk::SmartPointer<Image> MultiplexLoadDICOMByITK(const StringContainer&, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);

  static
  itk::SmartPointer<Image> MultiplexLoadDICOMByITKScalar(const StringContainer&, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);

  static
  itk::SmartPointer<Image> MultiplexLoadDICOMByITKRGBPixel(const StringContainer&, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);


  template <typename PixelType>
  static
  itk::SmartPointer<Image>
  LoadDICOMByITK4D( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);

  static
  itk::SmartPointer<Image>
  MultiplexLoadDICOMByITK4D( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);

  static
  itk::SmartPointer<Image>
  MultiplexLoadDICOMByITK4DScalar( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);

  static
  itk::SmartPointer<Image>
  MultiplexLoadDICOMByITK4DRGBPixel( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, itk::SmartPointer<Image> preLoadedImageBlock);



  /**
    \brief Sort files into time step blocks of a 3D+t image.

    Called by LoadDicom. Expects to be fed a single list of filenames that have been sorted by
    GetSeries previously (one map entry). This method will check how many timestep can be filled
    with given files.

    Assumption is that the number of time steps is determined by how often the first position in
    space repeats. I.e. if the first three files in the input parameter all describe the same
    location in space, we'll construct three lists of files. and sort the remaining files into them.

    \todo We can probably remove this method if we somehow transfer 3D+t information from GetSeries to LoadDicomSeries.
  */
  static
  std::list<StringContainer>
  SortIntoBlocksFor3DplusT( const StringContainer& presortedFilenames, const gdcm::Scanner::MappingType& tagValueMappings_, bool sort, bool& canLoadAs4D);

  /**
   \brief Defines spatial sorting for sorting by GDCM 2.

   Sorts by image position along image normal (distance from world origin).
   In cases of conflict, acquisition time is used as a secondary sort criterium.
  */
  static
  bool
  GdcmSortFunction(const gdcm::DataSet &ds1, const gdcm::DataSet &ds2);


  /**
    \brief Copy information about files and DICOM tags from ITK's MetaDataDictionary
           and from the list of input files to the PropertyList of mitk::Image.
    \todo Tag copy must follow; image level will cause some additional files parsing, probably.
  */
  static void CopyMetaDataToImageProperties( StringContainer filenames, const gdcm::Scanner::MappingType& tagValueMappings_, DcmIoType* io, const ImageBlockDescriptor& blockInfo, Image* image);
  static void CopyMetaDataToImageProperties( std::list<StringContainer> imageBlock, const gdcm::Scanner::MappingType& tagValueMappings_, DcmIoType* io, const ImageBlockDescriptor& blockInfo, Image* image);

  /**
    \brief Map between DICOM tags and MITK properties.

    Uses as a positive list for copying specified DICOM tags (from ITK's ImageIO)
    to MITK properties. ITK provides MetaDataDictionary entries of form
    "gggg|eeee" (g = group, e = element), e.g. "0028,0109" (Largest Pixel in Series),
    which we want to sort as dicom.series.largest_pixel_in_series".
  */
  static const TagToPropertyMapType& GetDICOMTagsToMITKPropertyMap();
};

}


#endif /* mitkDicomSeriesReader_h */
