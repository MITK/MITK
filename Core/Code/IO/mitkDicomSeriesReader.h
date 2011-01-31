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

#ifndef MITKDICOMSERIESREADER_H_
#define MITKDICOMSERIESREADER_H_

#include <mitkDataNode.h>
#include <mitkConfig.h>

#ifdef MITK_USE_GDCMIO
#include <itkGDCMImageIO.h>
#else
#include <itkDICOMImageIO2.h>
#endif

#include <gdcmConfigure.h>

#if GDCM_MAJOR_VERSION >= 2
#include <gdcmDataSet.h>
#include <gdcmRAWCodec.h>
#include <gdcmSorter.h>
#include <gdcmScanner.h>
#include <gdcmPixmapReader.h>
#include <gdcmStringFilter.h>
#endif
#include <itkImageSeriesReader.h>
#include <itkCommand.h>

namespace mitk
{

/**
* Provides functions for loading DICOM series.
* The series is specified through an enumeration of all files contained in it.
* A method for the creation of file name enumerations of all series contained in
* a particular directory is also provided.
* After loading the series, the generated DataNode is not named. The caller is
* responsible for given the node a name.
*/
class MITK_CORE_EXPORT DicomSeriesReader
{
public:
  typedef std::vector<std::string> StringContainer;
  typedef std::map<std::string, StringContainer> UidFileNamesMap;
  typedef void (*UpdateCallBackMethod)(float);

  /**
  * Loads a DICOM series composed by the file names enumerated in the file names container.
  * If a callback method is supplied, it will be called after every progress update with a progress value in [0,1].
  */
  static DataNode::Pointer LoadDicomSeries(const StringContainer &filenames, bool sort = true, bool check_4d = true, UpdateCallBackMethod callback = 0);

  /**
  * Loads a DICOM series composed by the file names enumerated in the file names container.
  * If a callback method is supplied, it will be called after every progress update with a progress value in [0,1].
  */
  static bool LoadDicomSeries(const StringContainer &filenames, DataNode &node, bool sort = true, bool check_4d = true, UpdateCallBackMethod callback = 0);

  /**
  * Checks if a specific file is a DICOM.
  */
  static bool IsDicom(const std::string &filename);

#if GDCM_MAJOR_VERSION >= 2
  /**
  * Checks if a specific file is a Philips3D Ultrasound DICOM file.
  */
  static bool IsPhilips3DDicom(const std::string &filename);

  /**
  * Read a Philips3D Ultrasound DICOM file and put into an mitk image
  */
  static bool ReadPhilips3DDicom(const std::string &filename, mitk::Image::Pointer output_image);
     
  /**
    Construct a UID that takes into account sorting criteria from GetSeries().
  */
  static std::string CreateMoreUniqueSeriesIdentifier( const gdcm::Scanner::TagToValue& tagValueMap );
  static std::string IDifyTagValue(const std::string& value);
#endif

  /**
  * \brief Find all series (and sub-series -- see details) in a particular directory.
  *
  * For each series, an enumeration of the files contained in it is created.
  *
  * \return The resulting map will map SeriesInstanceUID to UNSORTED lists of file names.
  *
  * SeriesInstanceUID will be FORCED to be unique for each set of file names
  * that should be loadable as a single mitk::Image. This implies that
  * Image orientation, slice thickness, pixel spacing, rows, and columns
  * must be the same for each file (i.e. the image slice contained in the file).
  *
  * If this separation logic requires that a SeriesInstanceUID must be made more specialized,
  * it will follow the same logic as itk::GDCMSeriesFileNames to enhance the UID with
  * more digits and dots.
  *
  * Optionally, more tags can be used to separate files into different logical series by setting
  * the restrictions parameter.
  */
  static UidFileNamesMap GetSeries(const std::string &dir, const StringContainer &restrictions = StringContainer());

  /**
  * Find a particular series in a directory.
  * For each series, an enumeration of the files contained in it is created.
  * Optionally, more criteria (restrictions) can be used to distinguish between different series, and series
  * restrictions can be specified.
  */
  static StringContainer GetSeries(const std::string &dir, const std::string &series_uid,
                                   const StringContainer &restrictions = StringContainer());

  /**
   * Sort a set of file names in an order that is meaningful for loading them into an mitk::Image.
   *
   * \warning This method assumes that input files are similar in basic properties such as slice thicknes, image orientation, pixel spacing, rows, columns.
   *
   * It should always be ok to put the result of a call to GetSeries(..) into this method.(..).
   *
   * Sorting order is determined by
   *
   *  1. TODO
   *  2.
   */
  static StringContainer SortSeriesSlices(const StringContainer &unsortedFilenames);
protected:
  inline DicomSeriesReader()
  {}

  inline ~DicomSeriesReader()
  {}

#ifdef MITK_USE_GDCMIO
  typedef itk::GDCMImageIO DcmIoType;
#else
  typedef itk::DICOMImageIO2 DcmIoType;
#endif

  class CallbackCommand : public itk::Command
  {
  public:
    inline CallbackCommand(UpdateCallBackMethod callback)
      : m_Callback(callback)
    {}

    inline void Execute(const itk::Object *caller, const itk::EventObject&)
    {
      (*this->m_Callback)(static_cast<const itk::ProcessObject*>(caller)->GetProgress());
    }

    inline void Execute(itk::Object *caller, const itk::EventObject&)
    {
      (*this->m_Callback)(static_cast<itk::ProcessObject*>(caller)->GetProgress());
    }
  protected:
    UpdateCallBackMethod m_Callback;
  };

  /**
  * Performs the loading of a series and creates an image having the specified pixel type.
  */
  template <typename PixelType>
  static void LoadDicom(const StringContainer &filenames, DataNode &node, bool sort, bool check_4d, UpdateCallBackMethod callback);

  /**
    Feed files into itk::ImageSeriesReader and retrieve a 3D MITK image.

    \param command can be used for progress reporting
  */
  template <typename PixelType>
  static Image::Pointer LoadDICOMByITK( const StringContainer&, CallbackCommand* command = NULL);

  
  /**
    Called by LoadDicom. Expects to be fed a single list of filenames that have been sorted by
    GetSeries previously (one map entry). This method will check how many timestep can be filled
    with given files.

    Assumption is that the number of time steps is determined by how often the first position in
    space repeats. I.e. if the first three files in the input parameter all describe the same
    location in space, we'll construct three lists of files. and sort the remaining files into them.
  */
  static std::list<StringContainer> SortIntoBlocksFor3DplusT( const StringContainer& presortedFilenames, bool sort, bool& canLoadAs4D );

#if GDCM_MAJOR_VERSION >= 2
  /*
  * Auxiliary sort function for Gdcm Dicom sorting. It is used for sorting
  * 4D Dicom data.
  */
  static bool GdcmSortFunction(const gdcm::DataSet &ds1, const gdcm::DataSet &ds2);
#endif
};

}


#endif /* MITKDICOMSERIESREADER_H_ */
