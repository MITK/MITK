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

  /**
   * Loads a DICOM series composed by the file names enumerated in the file names container.
   */
  static DataNode::Pointer LoadDicomSeries(const StringContainer &filenames);

  /**
   * Loads a DICOM series composed by the file names enumerated in the file names container.
   */
  static bool LoadDicomSeries(const StringContainer &filenames, DataNode &node);

  /**
   * Checks if a specific file is a DICOM.
   */
  static bool IsDicom(const std::string &filename);

  /**
   * Find all series in a particular directory.
   * For each series, an enumeration of the files contained in it is created.
   * Optionally, more criteria can be used to distinguish between different series, and series
   * restrictions can be specified.
   */
  static UidFileNamesMap GetSeries(const std::string &dir, bool additional_criteria = false,
      const StringContainer &restrictions = StringContainer());

  /**
   * Find a particular series in a directory.
   * For each series, an enumeration of the files contained in it is created.
   * Optionally, more criteria can be used to distinguish between different series, and series
   * restrictions can be specified.
   */
  static StringContainer GetSeries(const std::string &dir, const std::string &series_uid,
      bool additional_criteria = false, const StringContainer &restrictions = StringContainer());
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

  /**
   * Performs the loading of a series and creates an image having the specified pixel type.
   */
  template <typename PixelType>
  static void LoadDicom(const StringContainer &filenames, DataNode &node);
};

}

#include <mitkDicomSeriesReader.txx>

#endif /* MITKDICOMSERIESREADER_H_ */
