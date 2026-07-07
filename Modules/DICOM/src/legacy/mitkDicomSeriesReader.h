/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDicomSeriesReader_h
#define mitkDicomSeriesReader_h

#include <mitkConfig.h>
#include <mitkDataNode.h>

#include <itkGDCMImageIO.h>

#include <itkCommand.h>
#include <itkImageSeriesReader.h>

#ifdef NOMINMAX
#define DEF_NOMINMAX
#undef NOMINMAX
#endif

#include <gdcmConfigure.h>

#ifdef DEF_NOMINMAX
#ifndef NOMINMAX
#define NOMINMAX
#endif
#undef DEF_NOMINMAX
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

   <b>The main intention of this implementation is not efficiency but correctness of generated slice positions and pixel
  spacings!</b>

   \section DicomSeriesReader_limitations Assumptions and limitations

   The class is working only with GDCM 2.0.14 (or possibly newer). This version is the
   default of an MITK super-build. Support for other versions or ITK's DicomIO was dropped
   because of the associated complexity of DicomSeriesReader.

   \b Assumptions
    - expected to work with certain SOP Classes (mostly CT Image Storage and MR Image Storage)
      - see ImageBlockDescriptor.GetReaderImplementationLevel() method for the details
    - special treatment for a certain type of Philips 3D ultrasound (recognized by tag 3001,0010 set to "Philips3D")
    - loader will always attempt to read multiple single slices as a single 3D image volume (i.e. mitk::Image)
      - slices will be grouped by basic properties such as orientation, rows, columns, spacing and grouped into as large
  blocks as possible
    - images which do NOT report a position or orientation in space (Image Position Patient, Image Orientation) will be
  assigned defaults
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

   The general sorting mechanism (implemented in GetSeries) groups and sorts a set of DICOM files, each assumed to
  contain
  a single CT/MR slice.
   In the following we refer to those file groups as "blocks", since this is what they are meant to become when loaded
  into an mitk::Image.

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
     2. when distance is equal, (0020,0012) Acquisition Number, (0008,0032) Acquisition Time and (0018,1060) Trigger Time
  are
        used as a backup criterions (necessary for meaningful 3D+t sorting)

   \subsection DicomSeriesReader_sorting3 Step 3: Ensure equal z spacing

   Since inter-slice distance is not recorded in DICOM tags, we must ensure that blocks are made up of
   slices that have equal distances between neighboring slices. This is especially necessary because
  itk::ImageSeriesReader
   is later used for the actual loading, and this class expects (and does nocht verify) equal inter-slice distance (see
  \ref DicomSeriesReader_whatweknowaboutitk).

   To achieve such grouping, the inter-slice distance is calculated from the first two different slice positions of a
  block.
   Following slices are added to a block as long as they can be added by adding the calculated inter-slice distance to
  the
   last slice of the block. Slices that do not fit into the expected distance pattern, are set aside for further
  analysis.
   This grouping is done until each file has been assigned to a group.

   Slices that share a position in space are also sorted into separate blocks during this step.
   So the result of this step is a set of blocks that contain only slices with equal z spacing
   and unique slices at each position.

   \subsection DicomSeriesReader_sorting4 Step 4 (optional): group 3D blocks as 3D+t when possible

   This last step depends on an option of GetSeries(). When requested, image blocks from the previous step are merged
  again
   whenever two blocks occupy the same portion of space (i.e. same origin, number of slices and z-spacing).

   \section DicomSeriesReader_gantrytilt Handling of gantry tilt

   When CT gantry tilt is used, the gantry plane (= X-Ray source and detector ring) and the vertical plane do not align
   anymore. This scanner feature is used for example to reduce metal artifacts (e.g. <i>Lee C , Evaluation of Using CT
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
    - the plane shift that is ignored by ITK's reader is recreated by applying a shearing transformation using
  itk::ResampleFilter.
    - the spacing is corrected (it is calculated by ITK's reader from the distance between two origins, which is NOT the
  slice distance in this special case)

   Both errors are introduced in
   itkImageSeriesReader.tpp (ImageSeriesReader<TOutputImage>::GenerateOutputInformation(void)), lines 176 to 245 (as of
  ITK 3.20)

   For the correction, we examine two consecutive slices of a series, both described as a pair (origin/orientation):
    - we calculate if the first origin is on a line along the normal of the second slice
      - if this is not the case, the geometry will not fit a normal mitk::Image/mitk::Geometry3D
      - we then project the second origin into the first slice's coordinate system to quantify the shift
      - both is done in class GantryTiltInformation with quite some comments.

   The geometry of image stacks with tilted geometries is illustrated below:
    - green: the DICOM images as described by their tags: origin as a point with the line indicating the orientation
    - red: the output of ITK ImageSeriesReader: wrong, larger spacing, no tilt
    - blue: how much a shear must correct

    \image html Modules/DICOM/doc/Doxygen/tilt-correction.jpg

   \section DicomSeriesReader_whatweknowaboutitk The actual image loading process

   When calling LoadDicomSeries(), this method "mainly" uses an instance of itk::ImageSeriesReader,
   configured with an itk::GDCMImageIO object. Because DicomSeriesReader works around some of the
   behaviors of these classes, the following is a list of features that we find in the code and need to work with:

    - itk::ImageSeriesReader::GenerateOutputInformation() does the z-spacing handling
      + spacing is directly determined by comparing (euclidean distance) the origins of the first two slices of a series
        * this is GOOD because there is no reliable z-spacing information in DICOM images
        * this is bad because it does not work with gantry tilt, in which case the slice distance is SMALLER than the
  distance between two origins (see section on tilt)
    - origin and spacing are calculated by GDCMImageIO and re-used in itk::ImageSeriesReader
      + the origins are read from appropriate tags, nothing special about that
      + the spacing is read by gdcm::ImageReader, gdcm::ImageHelper::GetSpacingValue() from a tag determined by
  gdcm::ImageHelper::GetSpacingTagFromMediaStorage(), which basically determines ONE appropriate pixel spacing tag for
  each media storage type (ct image, mr image, secondary capture image, etc.)
        * this is fine for modalities such as CT/MR where the "Pixel Spacing" tag is mandatory, but for other modalities
  such as CR or Secondary Capture, the tag "Imager Pixel Spacing" is taken, which is no only optional but also has a
  more
  complicated relation with the "Pixel Spacing" tag. For this reason we check/modify the pixel spacing reported by
  itk::ImageSeriesReader after loading the image (see \ref DicomSeriesReader_pixelspacing)

   AFTER loading, DicomSeriesReader marks some of its findings as mitk::Properties to the loaded Image and DataNode:
    - <b>dicomseriesreader.SOPClass</b> : DICOM SOP Class as readable string (instead of a UID)
    - <b>dicomseriesreader.ReaderImplementationLevelString</b> : Confidence /Support level of the reader for this image
  as
  readable string
    - <b>dicomseriesreader.ReaderImplementationLevel</b> : Confidence /Support level of the reader for this image as
  enum
  value of type ReaderImplementationLevel
    - <b>dicomseriesreader.PixelSpacingInterpretationString</b> : Appropriate interpreteation of pixel spacing for this
  Image as readable string
    - <b>dicomseriesreader.PixelSpacingInterpretation</b> : Appropriate interpreteation of pixel spacing for this Image
  as
  enum value of type PixelSpacingInterpretation
    - <b>dicomseriesreader.MultiFrameImage</b> : bool flag to mark multi-frame images
    - <b>dicomseriesreader.GantyTiltCorrected</b> : bool flag to mark images where a gantry tilt was corrected to fit
  slices into an mitk::Image
    - <b>dicomseriesreader.3D+t</b> : bool flag to mark images with a time dimension (multiple 3D blocks of the same
  size
  at the same position in space)

   \section DicomSeriesReader_pixelspacing Handling of pixel spacing

   The reader implements what is described in DICOM Part 3, chapter 10.7 (Basic Pixel Spacing Calibration Macro): Both
  tags
    - (0028,0030) Pixel Spacing and
    - (0018,1164) Imager Pixel Spacing

   are evaluated and the pixel spacing is set to the spacing within the patient when tags allow that.
   The result of pixel spacing interpretation can be read from a property
  "dicomseriesreader.PixelSpacingInterpretation",
   which refers to one of the enumerated values of type PixelSpacingInterpretation;

   \section DicomSeriesReader_supportedmodalities Limitations for specific modalities

    - <b>Enhanced Computed Tomography / Magnetic Resonance Images</b> are currently NOT supported at all, because we
  lack
  general support for multi-frame images.
    - <b>Nuclear Medicine Images</b> are not supported fully supported, only the single-frame variants are loaded
  properly.

   \section DicomSeriesReader_nextworkitems Possible enhancements

    This is a short list of ideas for enhancement:
     - Class has historically grown and should be reviewed again. There is probably too many duplicated scanning code
     - Multi-frame images don't mix well with the current assumption of "one file - one slice", which is assumed by our
  code
       - It should be checked how well GDCM and ITK support these files (some load, some don't)
     - Specializations such as the Philips 3D code should be handled in a more generic way. The current handling of
  Philips 3D images is not nice at all

   \section DicomSeriesReader_whynotinitk Why is this not in ITK?

    Some of this code would probably be better located in ITK. It is just a matter of resources that this is not the
    case yet. Any attempts into this direction are welcome and can be supported. At least the gantry tilt correction
    should be a simple addition to itk::ImageSeriesReader.

   \section DicomSeriesReader_tests Tests regarding DICOM loading

   A number of tests have been implemented to check our assumptions regarding DICOM loading. Please see \ref
  DICOMTesting

   \todo refactor all the protected helper objects/methods into a separate header so we compile faster
  */

  class Image;

  class DicomSeriesReader
  {
  public:
    /**
     * \brief Container type for lists of DICOM filenames.
     */
    typedef std::vector<std::string> StringContainer;

    /**
     * \brief Function pointer type for progress callbacks.
     *
     * The callback receives a float value in [0,1] representing the loading progress.
     */
    typedef void (*UpdateCallBackMethod)(float);

    /**
     * \brief Describes how well the reader is tested for a certain file type (SOP Class).
     *
     * Applications should not rely on the outcome for images which are reported as
     * ReaderImplementationLevel_Implemented or ReaderImplementationLevel_Unsupported.
     *
     * Errors loading images reported as ReaderImplementationLevel_Supported are considered
     * bugs. For ReaderImplementationLevel_PartlySupported, check the appropriate paragraph
     * in \ref DicomSeriesReader_supportedmodalities.
     */
    typedef enum {
      ReaderImplementationLevel_Supported,       ///< Loader code and tests are established.
      ReaderImplementationLevel_PartlySupported, ///< Loader code and tests are established for specific parts of a SOP Class.
      ReaderImplementationLevel_Implemented,     ///< Loader code is implemented but not accompanied by tests.
      ReaderImplementationLevel_Unsupported,     ///< Loader code is not working with this SOP Class.
    } ReaderImplementationLevel;

    /**
     * \brief How the mitk::Image pixel spacing should be interpreted.
     *
     * Compare DICOM PS 3.3 10.7 (Basic Pixel Spacing Calibration Macro).
     */
    typedef enum {
      PixelSpacingInterpretation_SpacingInPatient,  ///< Distances are mm within a patient.
      PixelSpacingInterpretation_SpacingAtDetector, ///< Distances are mm at detector surface.
      PixelSpacingInterpretation_SpacingUnknown     ///< No spacing information is present; (1,1) is used as default.
    } PixelSpacingInterpretation;

    /**
     * \brief Return type of GetSeries, describes a logical group of DICOM files.
     *
     * Files grouped into a single 3D or 3D+t block are described by an instance
     * of this class. Relevant descriptive properties can be used to provide
     * the application user with meaningful choices about which data to load.
     */
    class ImageBlockDescriptor
    {
    public:
      /** \brief Get the list of filenames in this group. */
      StringContainer GetFilenames() const;

      /** \brief Get a unique ID describing this block (enhanced Series Instance UID). */
      std::string GetImageBlockUID() const;

      /** \brief Get the Series Instance UID. */
      std::string GetSeriesInstanceUID() const;

      /** \brief Get the series modality (CT, MR, etc.). */
      std::string GetModality() const;

      /** \brief Get the SOP Class UID as a readable string (e.g., "Computed Tomography Image Storage"). */
      std::string GetSOPClassUIDAsString() const;

      /** \brief Get the SOP Class UID as a DICOM UID string. */
      std::string GetSOPClassUID() const;

      /** \brief Get the confidence level of the reader for this block. */
      ReaderImplementationLevel GetReaderImplementationLevel() const;

      /** \brief Whether the block contains a gantry tilt which will be corrected during loading. */
      bool HasGantryTiltCorrected() const;

      /** \brief Whether mitk::Image spacing relates to the patient. */
      bool PixelSpacingRelatesToPatient() const;
      /** \brief Whether mitk::Image spacing relates to the detector surface. */
      bool PixelSpacingRelatesToDetector() const;
      /** \brief Whether mitk::Image spacing origin is unknown. */
      bool PixelSpacingIsUnknown() const;

      /** \brief Get the interpretation of the mitk::Image pixel spacing. */
      PixelSpacingInterpretation GetPixelSpacingType() const;

      /** \brief Whether this block contains multiple time points (3D+t). */
      bool HasMultipleTimePoints() const;

      /** \brief Whether this block contains multi-frame image(s). */
      bool IsMultiFrameImage() const;

      ImageBlockDescriptor();
      ~ImageBlockDescriptor();

    private:
      friend class DicomSeriesReader;

      ImageBlockDescriptor(const StringContainer &files);

      void AddFile(const std::string &file);
      void AddFiles(const StringContainer &files);

      void SetImageBlockUID(const std::string &uid);

      void SetSeriesInstanceUID(const std::string &uid);

      void SetModality(const std::string &modality);

      void SetNumberOfFrames(const std::string &);

      void SetSOPClassUID(const std::string &mediaStorageSOPClassUID);

      void SetHasGantryTiltCorrected(bool);

      void SetPixelSpacingInformation(const std::string &pixelSpacing, const std::string &imagerPixelSpacing);

      void SetHasMultipleTimePoints(bool);

      void GetDesiredMITKImagePixelSpacing(ScalarType &spacingX, ScalarType &spacingY) const;

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

    /** \brief Map type from enhanced UID strings to ImageBlockDescriptor instances. */
    typedef std::map<std::string, ImageBlockDescriptor> FileNamesGrouping;

    /**
     * \brief Provide the combination of preprocessor defines that was active during compilation.
     *
     * Since this class is a combination of several possible implementations separated only
     * by ifdefs, calling instances might want to know which flags were active at compile time.
     *
     * \return A human-readable configuration string.
     */
    static std::string GetConfigurationString();

    /**
     * \brief Check if a specific file contains DICOM data.
     *
     * \param filename Path to the file to check.
     * \return True if the file contains DICOM data, false otherwise.
     */
    static bool IsDicom(const std::string &filename);

    /**
     * \brief Find all series (and sub-series) in a particular directory.
     *
     * Scans the given directory for DICOM files, groups them, and returns
     * the result. See the preferred overload taking a StringContainer for details.
     *
     * \param dir The directory to scan for DICOM files.
     * \param groupImagesWithGantryTilt Whether to apply gantry tilt correction grouping.
     * \param restrictions Additional DICOM tag restrictions for grouping.
     * \return A map from enhanced UID strings to ImageBlockDescriptor instances.
     *
     * \sa GetSeries(const StringContainer&, bool, bool, const StringContainer&)
     */
    static FileNamesGrouping GetSeries(const std::string &dir,
                                       bool groupImagesWithGantryTilt,
                                       const StringContainer &restrictions = StringContainer());

    /**
     * \brief Find files belonging to a specific DICOM series in a directory.
     *
     * This differs from the other GetSeries() overloads by restricting results to a
     * single known DICOM series, identified by its Series Instance UID.
     * Internally delegates to the directory-based GetSeries() method.
     *
     * \warning Untested; may or may not work correctly.
     *
     * \param dir The directory to scan for DICOM files.
     * \param series_uid The Series Instance UID to filter for.
     * \param groupImagesWithGantryTilt Whether to apply gantry tilt correction grouping.
     * \param restrictions Additional DICOM tag restrictions for grouping.
     * \return A sorted list of filenames belonging to the specified series.
     */
    static StringContainer GetSeries(const std::string &dir,
                                     const std::string &series_uid,
                                     bool groupImagesWithGantryTilt,
                                     const StringContainer &restrictions = StringContainer());

    /**
     * \brief PREFERRED version -- scan and sort a list of DICOM files into loadable blocks.
     *
     * Parse a list of files for images of DICOM series.
     * For each series, an enumeration of the files contained in it is created.
     *
     * SeriesInstanceUID will be enhanced to be unique for each set of file names
     * that is later loadable as a single mitk::Image. This implies that
     * image orientation, slice thickness, pixel spacing, rows, and columns
     * must be the same for each file (i.e. the image slice contained in the file).
     *
     * If this separation logic requires that a SeriesInstanceUID must be made more specialized,
     * it will follow the same logic as itk::GDCMSeriesFileNames to enhance the UID with
     * more digits and dots.
     *
     * \param files List of DICOM file paths to analyze.
     * \param sortTo3DPlust Whether to attempt grouping into 3D+t blocks.
     * \param groupImagesWithGantryTilt Whether to apply gantry tilt correction grouping.
     * \param restrictions Additional DICOM tag restrictions for grouping.
     * \return A map from UID-like keys to ImageBlockDescriptor instances with sorted file lists.
     *
     * \warning Adding restrictions is not yet implemented.
     */
    static FileNamesGrouping GetSeries(const StringContainer &files,
                                       bool sortTo3DPlust,
                                       bool groupImagesWithGantryTilt,
                                       const StringContainer &restrictions = StringContainer());

    /**
     * \brief Scan and sort DICOM files (without explicit 3D+t control).
     *
     * Convenience overload that defaults 3D+t sorting to true.
     * Prefer the overload with explicit \p sortTo3DPlust parameter.
     *
     * \param files List of DICOM file paths to analyze.
     * \param groupImagesWithGantryTilt Whether to apply gantry tilt correction grouping.
     * \param restrictions Additional DICOM tag restrictions for grouping.
     * \return A map from UID-like keys to ImageBlockDescriptor instances.
     *
     * \sa GetSeries(const StringContainer&, bool, bool, const StringContainer&)
     */
    static FileNamesGrouping GetSeries(const StringContainer &files,
                                       bool groupImagesWithGantryTilt,
                                       const StringContainer &restrictions = StringContainer());

    /**
     * \brief Load a DICOM series into a new DataNode.
     *
     * Loads the DICOM series composed of the given filenames and wraps the result
     * in a DataNode. If a callback method is supplied, it will be called after
     * every progress update with a progress value in [0,1].
     *
     * \param filenames The DICOM file paths to load.
     * \param sort Whether files should be sorted spatially (true) or not (false, useful if presorted).
     * \param load4D Whether to attempt loading the files as 3D+t (if possible).
     * \param correctGantryTilt Whether to correct for gantry tilt during loading.
     * \param callback Optional progress callback function.
     * \param preLoadedImageBlock Optional pre-loaded image to use instead of reading from disk.
     * \return A DataNode containing the loaded mitk::Image, or nullptr on failure.
     */
    static DataNode::Pointer LoadDicomSeries(const StringContainer &filenames,
                                             bool sort = true,
                                             bool load4D = true,
                                             bool correctGantryTilt = true,
                                             UpdateCallBackMethod callback = nullptr,
                                             itk::SmartPointer<Image> preLoadedImageBlock = nullptr);

    /**
     * \brief Load a DICOM series into an existing DataNode.
     *
     * Same as the other LoadDicomSeries() overload but stores the result into the
     * provided DataNode reference. If \p preLoadedImageBlock is provided, the reader
     * will only create appropriate mitk::Properties without actually reading pixel data.
     *
     * \param filenames The DICOM file paths to load.
     * \param node The DataNode to populate with the loaded image and properties.
     * \param sort Whether files should be sorted spatially.
     * \param load4D Whether to attempt loading the files as 3D+t.
     * \param correctGantryTilt Whether to correct for gantry tilt during loading.
     * \param callback Optional progress callback function.
     * \param preLoadedImageBlock Optional pre-loaded image to use instead of reading from disk.
     * \return True if loading succeeded, false otherwise.
     */
    static bool LoadDicomSeries(const StringContainer &filenames,
                                DataNode &node,
                                bool sort = true,
                                bool load4D = true,
                                bool correctGantryTilt = true,
                                UpdateCallBackMethod callback = nullptr,
                                itk::SmartPointer<Image> preLoadedImageBlock = nullptr);

  protected:
    /**
     * \brief Return type of DicomSeriesReader::AnalyzeFileForITKImageSeriesReaderSpacingAssumption.
     *
     * Contains the grouping result of AnalyzeFileForITKImageSeriesReaderSpacingAssumption(),
     * which takes as input a number of images that are all equally oriented and spatially
     * sorted along their normal direction.
     *
     * The result consists of two blocks: the first contains files that can be loaded
     * into one image block because they have equal origin-to-origin distance without gaps.
     * The second contains remaining files that need further analysis.
     */
    class SliceGroupingAnalysisResult
    {
    public:
      SliceGroupingAnalysisResult();

      /**
       * \brief Get the grouped filenames (all with same origin-to-origin distance, no gaps).
       * \return The list of filenames that form a valid image block.
       */
      StringContainer GetBlockFilenames();

      /**
       * \brief Get the remaining files which could not be grouped.
       * \return The list of filenames not fitting into the primary block.
       */
      StringContainer GetUnsortedFilenames();

      /**
       * \brief Whether the grouped result contains a gantry tilt.
       * \return True if gantry tilt was detected in the grouped block.
       */
      bool ContainsGantryTilt();

      /**
       * \brief Add a file to the sorted (grouped) block.
       * \note Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
       * \param filename Path to the file to add.
       */
      void AddFileToSortedBlock(const std::string &filename);

      /**
       * \brief Add a file to the unsorted (remaining) block.
       * \note Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
       * \param filename Path to the file to add.
       */
      void AddFileToUnsortedBlock(const std::string &filename);

      /**
       * \brief Add multiple files to the unsorted (remaining) block.
       * \note Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
       * \param filenames Paths to the files to add.
       */
      void AddFilesToUnsortedBlock(const StringContainer &filenames);

      /**
       * \brief Flag this result as containing a gantry tilt.
       * \note Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
       * \todo Could make sense to enhance this with an instance of GantryTiltInformation to store the whole result.
       */
      void FlagGantryTilt();

      /**
       * \brief Undo a premature grouping decision.
       *
       * Moves all files from the sorted block back into the unsorted block.
       * Only meaningful for use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption.
       */
      void UndoPrematureGrouping();

    protected:
      StringContainer m_GroupedFiles;   ///< \brief Files that form a valid image block.
      StringContainer m_UnsortedFiles;  ///< \brief Files that could not be grouped.

      bool m_GantryTilt; ///< \brief Whether gantry tilt was detected.
    };

    /**
      \brief Gantry tilt analysis result.

      Takes geometry information for two slices of a DICOM series and
      calculates whether these fit into an orthogonal block or not.
      If NOT, they can either be the result of an acquisition with
      gantry tilt OR completely broken by some shearing transformation.

      Most calculations are done in the constructor, results can then
      be read via the remaining methods.
    */
    class GantryTiltInformation
    {
    public:
      // two types to avoid any rounding errors
      typedef itk::Point<double, 3> Point3Dd;
      typedef itk::Vector<double, 3> Vector3Dd;

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
      GantryTiltInformation(const Point3D &origin1,
                            const Point3D &origin2,
                            const Vector3D &right,
                            const Vector3D &up,
                            unsigned int numberOfSlicesApart);

      /**
        \brief Whether the slices were sheared.

        True if any of the shifts along right or up vector are non-zero.
      */
      bool IsSheared() const;

      /**
        \brief Whether the shearing is a gantry tilt or more complicated.

        Gantry tilt will only produce shifts in ONE orientation, not in both.

        Since the correction code currently only covers one tilt direction
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
      Point3D projectPointOnLine(Point3Dd p, Point3Dd lineOrigin, Vector3Dd lineDirection);

      double m_ShiftUp;
      double m_ShiftRight;
      double m_ShiftNormal;
      double m_ITKAssumedSliceSpacing;
      unsigned int m_NumberOfSlicesApart;
    };

    /** \brief Pair of string containers used for internal sorting operations. */
    typedef std::pair<StringContainer, StringContainer> TwoStringContainers;

    /** \brief Map type from DICOM tag strings to MITK property name strings. */
    typedef std::map<std::string, std::string> TagToPropertyMapType;

    /**
     * \brief Ensure equal z-spacing for a group of files.
     *
     * Takes as input a number of images which are all equally oriented and spatially
     * sorted along their normal direction. Internally used by GetSeries().
     *
     * Returns a SliceGroupingAnalysisResult where the first list contains slices with
     * equal inter-slice spacing and the second list contains remaining files that need
     * to be run through this method again.
     *
     * \param files Spatially sorted list of DICOM filenames to analyze.
     * \param groupsOfSimilarImages Whether to group images with similar but not identical spacing.
     * \param tagValueMappings_ GDCM scanner tag-value mappings for the files.
     * \return The grouping analysis result.
     */
    static SliceGroupingAnalysisResult AnalyzeFileForITKImageSeriesReaderSpacingAssumption(
      const StringContainer &files, bool groupsOfSimilarImages, const gdcm::Scanner::MappingType &tagValueMappings_);

    /**
     * \brief Safely convert a const char pointer to std::string.
     *
     * \param s The C-string to convert (may be nullptr).
     * \return The string value, or an empty string if s is nullptr.
     */
    static std::string ConstCharStarToString(const char *s);

    /**
     * \brief Parse a DICOM spacing string into pixel spacing X and Y values.
     *
     * \param s The DICOM spacing string (backslash-separated).
     * \param spacingX Output parameter for the X spacing.
     * \param spacingY Output parameter for the Y spacing.
     * \return True if parsing succeeded, false otherwise.
     */
    static bool DICOMStringToSpacing(const std::string &s, ScalarType &spacingX, ScalarType &spacingY);

    /**
     * \brief Convert a DICOM string describing a 3D point to Point3D.
     *
     * DICOM tags like ImagePositionPatient contain a position as float numbers
     * separated by backslashes (e.g., "42.7131\\13.77\\0.7").
     *
     * \param s The DICOM position string.
     * \param successful Output flag indicating whether parsing succeeded.
     * \return The parsed 3D point.
     */
    static Point3D DICOMStringToPoint3D(const std::string &s, bool &successful);

    /**
     * \brief Convert a DICOM orientation string to two direction vectors.
     *
     * DICOM tags like ImageOrientationPatient contain two vectors as float numbers
     * separated by backslashes (e.g., "1\\0\\0\\0\\1\\0").
     *
     * \param s The DICOM orientation string.
     * \param right Output parameter for the row direction vector.
     * \param up Output parameter for the column direction vector.
     * \param successful Output flag indicating whether parsing succeeded.
     */
    static void DICOMStringToOrientationVectors(const std::string &s, Vector3D &right, Vector3D &up, bool &successful);

    /**
     * \brief Correct a tilted image geometry by resampling with a shear transformation.
     *
     * Applies a correction for gantry tilt by resampling the image using itk::ResampleFilter.
     *
     * \tparam ImageType The ITK image type.
     * \param input The input image with tilted geometry.
     * \param tiltInfo The gantry tilt information describing the correction needed.
     * \return The corrected image.
     *
     * \note Despite the name, this is NOT an in-place operation.
     */
    template <typename ImageType>
    static typename ImageType::Pointer
      InPlaceFixUpTiltedGeometry(ImageType *input, const GantryTiltInformation &tiltInfo);

    /**
      \brief Sort a set of file names in an order that is meaningful for loading them into an mitk::Image.

      \warning This method assumes that input files are similar in basic properties such as
               slice thickness, image orientation, pixel spacing, rows, columns.
               It should always be ok to put the result of a call to GetSeries(..) into this method.

      Sorting order is determined by

       1. image position along its normal (distance from world origin)
       2. acquisition time

      If P<n> denotes a position and T<n> denotes a time step, this method will order slices from three timesteps like
  this:
  \verbatim
    P1T1 P1T2 P1T3 P2T1 P2T2 P2T3 P3T1 P3T2 P3T3
  \endverbatim

     */
    static StringContainer SortSeriesSlices(const StringContainer &unsortedFilenames);

  public:
    /**
     * \brief Check if a specific file is a Philips 3D ultrasound DICOM file.
     *
     * Checks for the presence of tag (3001,0010) set to "Philips3D".
     *
     * \param filename Path to the DICOM file to check.
     * \return True if the file is a Philips 3D ultrasound DICOM, false otherwise.
     */
    static bool IsPhilips3DDicom(const std::string &filename);

    /**
     * \brief Convert a ReaderImplementationLevel enum value to a human-readable string.
     *
     * \param enumValue The enum value to convert.
     * \return A readable string representation.
     */
    static std::string ReaderImplementationLevelToString(const ReaderImplementationLevel &enumValue);

    /**
     * \brief Convert a PixelSpacingInterpretation enum value to a human-readable string.
     *
     * \param enumValue The enum value to convert.
     * \return A readable string representation.
     */
    static std::string PixelSpacingInterpretationToString(const PixelSpacingInterpretation &enumValue);

  protected:
    /**
     * \brief Read a Philips 3D ultrasound DICOM file and load it into an mitk::Image.
     *
     * \param filename Path to the Philips 3D DICOM file.
     * \param output_image The image to populate with the loaded data.
     * \return True if reading succeeded, false otherwise.
     */
    static bool ReadPhilips3DDicom(const std::string &filename, itk::SmartPointer<Image> output_image);

    /**
     * \brief Construct a UID that incorporates sorting criteria from GetSeries().
     *
     * Enhances the Series Instance UID with additional tag values to create a
     * unique identifier for each loadable image block.
     *
     * \param tagValueMap The GDCM tag-to-value mapping for the series.
     * \return An enhanced unique identifier string.
     */
    static std::string CreateMoreUniqueSeriesIdentifier(gdcm::Scanner::TagToValue &tagValueMap);

    /**
     * \brief Extract a single tag value as a string for use in CreateMoreUniqueSeriesIdentifier().
     *
     * \param tagValueMap The GDCM tag-to-value mapping.
     * \param tag The DICOM tag to extract.
     * \return The tag value as a string, or an empty string if not found.
     */
    static std::string CreateSeriesIdentifierPart(gdcm::Scanner::TagToValue &tagValueMap, const gdcm::Tag &tag);

    /**
     * \brief Sanitize a tag value string for use as part of a UID.
     *
     * Removes or replaces characters that would not be suitable in an identifier string.
     *
     * \param value The raw tag value string.
     * \return A sanitized string suitable for use in a UID.
     */
    static std::string IDifyTagValue(const std::string &value);

    /** \brief Typedef for the GDCM image IO class used internally. */
    typedef itk::GDCMImageIO DcmIoType;

    /**
     * \brief ITK command adapter for progress callbacks.
     *
     * Wraps an UpdateCallBackMethod function pointer into an itk::Command
     * so it can be used with ITK process objects for progress reporting.
     */
    class CallbackCommand : public itk::Command
    {
    public:
      /** \brief Construct with the given callback function pointer. */
      CallbackCommand(UpdateCallBackMethod callback) : m_Callback(callback) {}

      /** \brief Execute the callback with the progress value from a const ITK process object. */
      void Execute(const itk::Object *caller, const itk::EventObject &) override
      {
        (*this->m_Callback)(static_cast<const itk::ProcessObject *>(caller)->GetProgress());
      }

      /** \brief Execute the callback with the progress value from an ITK process object. */
      void Execute(itk::Object *caller, const itk::EventObject &) override
      {
        (*this->m_Callback)(static_cast<itk::ProcessObject *>(caller)->GetProgress());
      }

    protected:
      UpdateCallBackMethod m_Callback; ///< \brief The wrapped progress callback.
    };

    /**
     * \brief Correct the pixel spacing of a loaded image based on the image block descriptor.
     *
     * \param image The loaded image whose spacing may need correction.
     * \param imageBlockDescriptor The descriptor containing pixel spacing information.
     */
    static void FixSpacingInformation(Image *image, const ImageBlockDescriptor &imageBlockDescriptor);

    /**
     * \brief Scan DICOM files for slice-relevant tag information.
     *
     * Configures and runs the GDCM scanner to extract tags needed for
     * spatial sorting and grouping (position, orientation, spacing, etc.).
     *
     * \param filenames The DICOM files to scan.
     * \param scanner The GDCM scanner to configure and execute.
     */
    static void ScanForSliceInformation(const StringContainer &filenames, gdcm::Scanner &scanner);

    /**
     * \brief Perform the actual loading of a DICOM series.
     *
     * Determines the pixel type, invokes the appropriate templated loader,
     * and populates the DataNode with the resulting image and properties.
     *
     * \param filenames The DICOM file paths to load.
     * \param node The DataNode to populate.
     * \param sort Whether to sort files spatially before loading.
     * \param check_4d Whether to attempt 3D+t loading.
     * \param correctTilt Whether to correct for gantry tilt.
     * \param callback Optional progress callback.
     * \param preLoadedImageBlock Optional pre-loaded image block.
     */
    static void LoadDicom(const StringContainer &filenames,
                          DataNode &node,
                          bool sort,
                          bool check_4d,
                          bool correctTilt,
                          UpdateCallBackMethod callback,
                          itk::SmartPointer<Image> preLoadedImageBlock);

    /**
     * \brief Feed files into itk::ImageSeriesReader and retrieve a 3D MITK image.
     *
     * \tparam PixelType The pixel type for the ITK image reader.
     * \param correctTilt Whether to apply gantry tilt correction after loading.
     * \param tiltInfo The gantry tilt information for correction.
     * \param io The GDCM image IO instance to use.
     * \param command Optional ITK command for progress reporting.
     * \param preLoadedImageBlock Optional pre-loaded image to use instead of reading.
     * \return The loaded MITK image, or nullptr on failure.
     */
    template <typename PixelType>
    static itk::SmartPointer<Image> LoadDICOMByITK(const StringContainer &,
                                                   bool correctTilt,
                                                   const GantryTiltInformation &tiltInfo,
                                                   DcmIoType::Pointer &io,
                                                   CallbackCommand *command,
                                                   itk::SmartPointer<Image> preLoadedImageBlock);

    /** \brief Pixel-type multiplexer for LoadDICOMByITK -- dispatches to scalar or RGB variants. */
    static itk::SmartPointer<Image> MultiplexLoadDICOMByITK(const StringContainer &,
                                                            bool correctTilt,
                                                            const GantryTiltInformation &tiltInfo,
                                                            DcmIoType::Pointer &io,
                                                            CallbackCommand *command,
                                                            itk::SmartPointer<Image> preLoadedImageBlock);

    /** \brief Scalar pixel type multiplexer for LoadDICOMByITK. */
    static itk::SmartPointer<Image> MultiplexLoadDICOMByITKScalar(const StringContainer &,
                                                                  bool correctTilt,
                                                                  const GantryTiltInformation &tiltInfo,
                                                                  DcmIoType::Pointer &io,
                                                                  CallbackCommand *command,
                                                                  itk::SmartPointer<Image> preLoadedImageBlock);

    /** \brief RGB pixel type multiplexer for LoadDICOMByITK. */
    static itk::SmartPointer<Image> MultiplexLoadDICOMByITKRGBPixel(const StringContainer &,
                                                                    bool correctTilt,
                                                                    const GantryTiltInformation &tiltInfo,
                                                                    DcmIoType::Pointer &io,
                                                                    CallbackCommand *command,
                                                                    itk::SmartPointer<Image> preLoadedImageBlock);

    /**
     * \brief Feed files into itk::ImageSeriesReader for 3D+t loading.
     *
     * Loads multiple time-step blocks into a single 4D MITK image.
     *
     * \tparam PixelType The pixel type for the ITK image reader.
     * \param imageBlocks List of filename containers, one per time step.
     * \param imageBlockDescriptor Descriptor for the image block.
     * \param correctTilt Whether to apply gantry tilt correction.
     * \param tiltInfo The gantry tilt information for correction.
     * \param io The GDCM image IO instance to use.
     * \param command Optional ITK command for progress reporting.
     * \param preLoadedImageBlock Optional pre-loaded image block.
     * \return The loaded 3D+t MITK image, or nullptr on failure.
     */
    template <typename PixelType>
    static itk::SmartPointer<Image> LoadDICOMByITK4D(std::list<StringContainer> &imageBlocks,
                                                     ImageBlockDescriptor imageBlockDescriptor,
                                                     bool correctTilt,
                                                     const GantryTiltInformation &tiltInfo,
                                                     DcmIoType::Pointer &io,
                                                     CallbackCommand *command,
                                                     itk::SmartPointer<Image> preLoadedImageBlock);

    /** \brief Pixel-type multiplexer for LoadDICOMByITK4D -- dispatches to scalar or RGB variants. */
    static itk::SmartPointer<Image> MultiplexLoadDICOMByITK4D(std::list<StringContainer> &imageBlocks,
                                                              ImageBlockDescriptor imageBlockDescriptor,
                                                              bool correctTilt,
                                                              const GantryTiltInformation &tiltInfo,
                                                              DcmIoType::Pointer &io,
                                                              CallbackCommand *command,
                                                              itk::SmartPointer<Image> preLoadedImageBlock);

    /** \brief Scalar pixel type multiplexer for LoadDICOMByITK4D. */
    static itk::SmartPointer<Image> MultiplexLoadDICOMByITK4DScalar(std::list<StringContainer> &imageBlocks,
                                                                    ImageBlockDescriptor imageBlockDescriptor,
                                                                    bool correctTilt,
                                                                    const GantryTiltInformation &tiltInfo,
                                                                    DcmIoType::Pointer &io,
                                                                    CallbackCommand *command,
                                                                    itk::SmartPointer<Image> preLoadedImageBlock);

    /** \brief RGB pixel type multiplexer for LoadDICOMByITK4D. */
    static itk::SmartPointer<Image> MultiplexLoadDICOMByITK4DRGBPixel(std::list<StringContainer> &imageBlocks,
                                                                      ImageBlockDescriptor imageBlockDescriptor,
                                                                      bool correctTilt,
                                                                      const GantryTiltInformation &tiltInfo,
                                                                      DcmIoType::Pointer &io,
                                                                      CallbackCommand *command,
                                                                      itk::SmartPointer<Image> preLoadedImageBlock);

    /**
     * \brief Sort files into time step blocks for 3D+t image loading.
     *
     * Called by LoadDicom(). Expects a single list of filenames previously sorted by
     * GetSeries(). Determines how many time steps can be filled with the given files.
     *
     * The number of time steps is determined by how often the first spatial position
     * repeats in the sorted input. For example, if the first three files describe the
     * same location in space, three time-step blocks are constructed and the remaining
     * files are distributed among them.
     *
     * \param presortedFilenames The spatially sorted list of DICOM filenames.
     * \param tagValueMappings_ GDCM scanner tag-value mappings for the files.
     * \param sort Whether to sort within each time step block.
     * \param canLoadAs4D Output flag indicating whether 4D loading is possible.
     * \return A list of StringContainers, one per time step.
     *
     * \todo We can probably remove this method if we somehow transfer 3D+t information
     *       from GetSeries to LoadDicomSeries.
     */
    static std::list<StringContainer> SortIntoBlocksFor3DplusT(const StringContainer &presortedFilenames,
                                                               const gdcm::Scanner::MappingType &tagValueMappings_,
                                                               bool sort,
                                                               bool &canLoadAs4D);

    /**
     * \brief Spatial sorting comparator for GDCM 2 datasets.
     *
     * Sorts by image position along image normal (distance from world origin).
     * In cases of conflict, acquisition time is used as a secondary criterion.
     *
     * \param ds1 First GDCM dataset to compare.
     * \param ds2 Second GDCM dataset to compare.
     * \return True if ds1 should precede ds2 in the sort order.
     */
    static bool GdcmSortFunction(const gdcm::DataSet &ds1, const gdcm::DataSet &ds2);

    /**
     * \brief Copy DICOM tag information to the PropertyList of an mitk::Image (single block).
     *
     * Extracts metadata from ITK's MetaDataDictionary and from the GDCM scanner tag
     * mappings, then stores them as MITK properties on the image.
     *
     * \param filenames The DICOM file paths for metadata extraction.
     * \param tagValueMappings_ GDCM scanner tag-value mappings.
     * \param io The GDCM image IO instance with loaded metadata.
     * \param blockInfo The image block descriptor with additional metadata.
     * \param image The MITK image to receive the properties.
     *
     * \todo Tag copy must follow; image level will cause some additional files parsing.
     */
    static void CopyMetaDataToImageProperties(StringContainer filenames,
                                              const gdcm::Scanner::MappingType &tagValueMappings_,
                                              DcmIoType *io,
                                              const ImageBlockDescriptor &blockInfo,
                                              Image *image);

    /**
     * \brief Copy DICOM tag information to the PropertyList of an mitk::Image (4D blocks).
     *
     * Overload for 3D+t images with multiple time-step blocks.
     *
     * \param imageBlock List of filename containers, one per time step.
     * \param tagValueMappings_ GDCM scanner tag-value mappings.
     * \param io The GDCM image IO instance with loaded metadata.
     * \param blockInfo The image block descriptor with additional metadata.
     * \param image The MITK image to receive the properties.
     */
    static void CopyMetaDataToImageProperties(std::list<StringContainer> imageBlock,
                                              const gdcm::Scanner::MappingType &tagValueMappings_,
                                              DcmIoType *io,
                                              const ImageBlockDescriptor &blockInfo,
                                              Image *image);

    /**
     * \brief Get the mapping from DICOM tags to MITK property names.
     *
     * Returns a positive list for copying specified DICOM tags (from ITK's ImageIO)
     * to MITK properties. ITK provides MetaDataDictionary entries of form
     * "gggg|eeee" (g = group, e = element), e.g. "0028,0109" (Largest Pixel in Series),
     * which are mapped to names like "dicom.series.largest_pixel_in_series".
     *
     * \return A const reference to the tag-to-property map.
     */
    static const TagToPropertyMapType &GetDICOMTagsToMITKPropertyMap();
  };
}

#endif
