/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkDicomSeriesReader_h
#define mitkDicomSeriesReader_h

#include "mitkDataNode.h"
#include "mitkConfig.h"

#ifdef MITK_USE_GDCMIO
  #include <itkGDCMImageIO.h>
#else
  #include <itkDICOMImageIO2.h>
#endif

#include <itkImageSeriesReader.h>
#include <itkCommand.h>

#include <gdcmConfigure.h>

#if GDCM_MAJOR_VERSION >= 2
  #include <gdcmDataSet.h>
  #include <gdcmRAWCodec.h>
  #include <gdcmSorter.h>
  #include <gdcmScanner.h>
  #include <gdcmPixmapReader.h>
  #include <gdcmStringFilter.h>
#endif


namespace mitk
{

/** 
 \brief Central DICOM image loading class for MITK.

 \section DicomSeriesReader_usage Usage
 
 The DICOM series is specified through an enumeration of all files contained in it.

 A method for the creation of file name enumerations of all series contained in
 a particular directory is also provided.

 After loading the series, the generated DataNode is not named. The caller is
 responsible for given the node a name.

  Example:

\code

 // only directory known here: /home/me/dicom

 DicomSeriesReader::UidFileNamesMap allImageBlocks = DicomSeriesReader::GetSeries("/home/me/dicom/");

 // file now divided into groups of identical image size, orientation, spacing, etc.
 // each of these lists should be loadable as an mitk::Image.

 // optional step: sorting (this is also done by LoadDicomSeries if not explicitly suppressed)
 DicomSeriesReader::StringContainer seriesToLoad = allImageBlocks[...]; // decide for yourself
  
 DicomSeriesReader::StringContainer oneBlockSorted = DicomSeriesReader::SortSeriesSlices( seriesToLoad );

 // final step: load into DataNode (can result in 3D+t image)
 DataNode::Pointer node = DicomSeriesReader::LoadDicomSeries( oneBlockSorted ); // would sort again, but we could set the sort flag to false

 Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
\endcode

TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
 <b>Necessary changes (bug 7285)</b>

 * GetSeries should by default
   * sort slices
   * determine if slice groups contain gaps (this is only done fast if slices are sorted previously)
     of if slices are not equally spaced
     * if gap, split into two parts 
     * if non-continous: split into continuous groups
       * this is a little tricky: create (all?) possible sortings, then determine the one with minimum number of groups
 * If somebody demands it, this check could be explicitly disabled by a flag
 * The SortSeriesSlices method can be removed (take care of existing callers), since this is done by GetSeries
 * LoadDicomSeries changes
   * remove parameter "bool sort" (now expected to be done previously)

 * handle titled geometries!
   * ImageSeriesReader will definitely calculate a wrong spacing
   * either we load each slice separately
   * or ...?


 3D+t handling:
  * the "block sorting" in GetSeries will separate images that occupy the identical position in space
    * this process will keep the initial sorting that organized slices by 1. position 2. acquisition time
    * i.e. after block sorting, there are multiple blocks that cover identical positions and 
      have identical properties regarding orientation, dimensions, spacing etc.
  * depending on arguments passed to GetSeries, it will attempt to group as many image blocks as possible to
    a single, large 3D+t image group (default behavior is to do this).
  * LoadDicomSeries expects(!) that the filenames passed either form a valid 3D block or a 3D+t block
    as sorted by GetSeries()

  * TODO later, when dicom tags are loaded for each mitk::Image slice, there should be a verification
    step for correct positions (optional if time-consuming)

TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
 
 \section DicomSeriesReader_sorting Sorting into 3D+t blocks

 TODO: describe strategy of sorting images into blocks of 3D image stacks

 \section DicomSeriesReader_limitations Assumptions and limitations

 The class is mostly updated for working properly with GDCM 2.0.14 and MITK_USE_GDCMIO ON.
 The other version are kept here for compatibility and may be removed in the future.

 \b Assumptions
  - expected to work for CT Image and MR Image
  - special treatment for a certain type of Philips 3D ultrasound (recogized by tag 3001,0010 set to "Philips3D")

 \b Limitations
  - Secondary Capture images are expected to have the (0018,2010) tag describing the pixel spacing.
    If only the (0028,0030) tag is set, the spacing will be misinterpreted as (1,1)

 \section DicomSeriesReader_internal Internal structure

 only valid for gdcm 2.0.14+

 map<string, StringContainer> GetSeries(dir)

  - scan for DICOM files in dir (gdcm::Scanner)
  - read Series Instance UID, image orientation, pixel spacing, slice thickness, number rows, number cols
  - sort all files into groups of files that match in mentioned attributes
  - resulting groups are named as "SeriesInstanceUID + UID like string made up of attribute values"


 StringContainer SortSeriesSlices(StringContainer)

  - use gdcm::Sorter to sort spatially
    - sort by
      1. distance along slice normal direction
      2. acquisition time (if present)
      3. by input order

 list<StringContainer> SortIntoBlocksFor3DplusT(StringContainer)

  - re-sort spatially sorted files (calls SortSeriesSlices unless explicitly told not to)
  - count how many images are at the position of the first image
    - assume this many 3D image blocks
  - for each block iterate all files in input, always skip "expected number of image blocks"
    - count how many images can be sorted into the first block
    - expect this many images for all other blocks
    - verify expectation, note image series as "cannot load as 3D+t" if expectation not met

 DataNode LoadDicom(StringContainer)

  - exception for Philips3D
  - if either
      (
      - sorting by SortIntoBlocksFor3DplusT cannot form a valid 3D+t block OR
      - method is invoked to not load 3D+t
      )
    - load only first block resulting from SortIntoBlocksFor3DplusT
  - else
    - create mitk::Image with multiple time step, assign result of loading each image block to one time step





*/
class MITK_CORE_EXPORT DicomSeriesReader
{
public:

  /** 
    \brief For lists of filenames 
  */
  typedef std::vector<std::string> StringContainer;
  typedef std::pair<StringContainer, StringContainer> TwoStringContainers;

  /** 
    \brief For multiple lists of filenames, assigned an ID each 
  */
  typedef std::map<std::string, StringContainer> UidFileNamesMap;

  /** 
    \brief Interface for the progress callback 
  */
  typedef void (*UpdateCallBackMethod)(float);

  /**
   \brief Checks if a specific file contains DICOM information.
  */
  static 
  bool 
  IsDicom(const std::string &filename);

  /**
   \brief Find all series (and sub-series -- see details) in a particular directory.
  
   For each series, an enumeration of the files contained in it is created.
  
   \return The resulting map will map SeriesInstanceUID to UNSORTED lists of file names.
  
   SeriesInstanceUID will be CHANGED to be unique for each set of file names
   that should be loadable as a single mitk::Image. This implies that
   Image orientation, slice thickness, pixel spacing, rows, and columns
   must be the same for each file (i.e. the image slice contained in the file).
  
   If this separation logic requires that a SeriesInstanceUID must be made more specialized,
   it will follow the same logic as itk::GDCMSeriesFileNames to enhance the UID with
   more digits and dots.
  
   Optionally, more tags can be used to separate files into different logical series by setting
   the restrictions parameter.
  */
  static UidFileNamesMap GetSeries(const std::string &dir, 
                                   const StringContainer &restrictions = StringContainer());

  /**
   \brief see other GetSeries().

   \warning Untested, could or could not work.

   This differs only by having an additional restriction to a single known DICOM series.
   Internally, it uses the other GetSeries() method. 
  */
  static StringContainer GetSeries(const std::string &dir, 
                                   const std::string &series_uid,
                                   const StringContainer &restrictions = StringContainer());

  /**
   \brief see other GetSeries().

   Instead of a directory name, provide an explicit list of file names.
  */
  static
  UidFileNamesMap 
  GetSeries(const StringContainer& files, bool sortTo3DPlust, const StringContainer &restrictions = StringContainer());
  
  /**
    \brief Legacy interface.

    Use GetSeries(const StringContainer& files, bool sortTo3DPlust, const StringContainer &restrictions) instead.
  */
  static
  UidFileNamesMap 
  GetSeries(const StringContainer& files, const StringContainer &restrictions = StringContainer());

  /**
    relevant code that is matched here is in 
    itkImageSeriesReader.txx (ImageSeriesReader<TOutputImage>::GenerateOutputInformation(void)), lines 176 to 245 (as of ITK 3.20)

  */
  static
  TwoStringContainers
  AnalyzeFileForITKImageSeriesReaderSpacingAssumption(const StringContainer& files, const gdcm::Scanner::MappingType& tagValueMappings);

  /**
    Sort a set of file names in an order that is meaningful for loading them into an mitk::Image.
   
    \warning This method assumes that input files are similar in basic properties such as slice thicknes, image orientation, pixel spacing, rows, columns. It should always be ok to put the result of a call to GetSeries(..) into this method.
   
    Sorting order is determined by
   
     1. image position along its normal (distance from world origin)
     2. acquisition time
   
    If P<n> denotes a position and T<n> denotes a time step, this method will order slices from three timesteps like this:
\verbatim
  P1T1 P1T2 P1T3 P2T1 P2T2 P2T3 P3T1 P3T2 P3T3
\endverbatim
   
   */
  static StringContainer SortSeriesSlices(const StringContainer &unsortedFilenames);

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
                                           UpdateCallBackMethod callback = 0);

  /**
    \brief See LoadDicomSeries! Just a slightly different interface.
  */
  static bool LoadDicomSeries(const StringContainer &filenames, 
                              DataNode &node, 
                              bool sort = true, 
                              bool load4D = true, 
                              UpdateCallBackMethod callback = 0);

  /**
    \brief Provide combination of preprocessor defines that was active during compilation.

    Since this class is a combination of several possible implementations, separated only
    by ifdef's, calling instances might want to know which flags were active at compile time.
  */
  static std::string GetConfigurationString();

protected:

#if GDCM_MAJOR_VERSION >= 2
public:
  /**
   \brief Checks if a specific file is a Philips3D ultrasound DICOM file.
  */
  static bool IsPhilips3DDicom(const std::string &filename);
protected:

  /**
   \brief Read a Philips3D ultrasound DICOM file and put into an mitk::Image.
  */
  static bool ReadPhilips3DDicom(const std::string &filename, mitk::Image::Pointer output_image);
     
  /**
    \brief Construct a UID that takes into account sorting criteria from GetSeries().
  */
  static std::string CreateMoreUniqueSeriesIdentifier( gdcm::Scanner::TagToValue& tagValueMap );
  static std::string CreateSeriesIdentifierPart( gdcm::Scanner::TagToValue& tagValueMap, const gdcm::Tag& tag );
  static std::string IDifyTagValue(const std::string& value);
#endif

#ifdef MITK_USE_GDCMIO
  typedef itk::GDCMImageIO DcmIoType;
#else
  typedef itk::DICOMImageIO2 DcmIoType;
#endif

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

  /**
   \brief Performs the loading of a series and creates an image having the specified pixel type.
  */
  template <typename PixelType>
  static 
  void 
  LoadDicom(const StringContainer &filenames, DataNode &node, bool sort, bool check_4d, UpdateCallBackMethod callback);

  /**
    \brief Feed files into itk::ImageSeriesReader and retrieve a 3D MITK image.

    \param command can be used for progress reporting
  */
  template <typename PixelType>
  static 
  Image::Pointer 
  LoadDICOMByITK( const StringContainer&, CallbackCommand* command = NULL);

#if GDCM_MAJOR_VERSION >= 2
  /**
    \brief Sort files into time step blocks of a 3D+t image.

    Called by LoadDicom. Expects to be fed a single list of filenames that have been sorted by
    GetSeries previously (one map entry). This method will check how many timestep can be filled
    with given files.

    Assumption is that the number of time steps is determined by how often the first position in
    space repeats. I.e. if the first three files in the input parameter all describe the same
    location in space, we'll construct three lists of files. and sort the remaining files into them.
  */
  static 
  std::list<StringContainer> 
  SortIntoBlocksFor3DplusT( const StringContainer& presortedFilenames, bool sort, bool& canLoadAs4D );

  /*
   \brief Defines spatial sorting for sorting by GDCM 2.

   Sorts by image position along image normal (distance from world origin).
   In cases of conflict, acquisition time is used as a secondary sort criterium.
  */
  static 
  bool 
  GdcmSortFunction(const gdcm::DataSet &ds1, const gdcm::DataSet &ds2);
#endif
};

}


#endif /* MITKDICOMSERIESREADER_H_ */
