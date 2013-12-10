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

#ifndef mitkEquiDistantBlocksSorter_h
#define mitkEquiDistantBlocksSorter_h

#include "mitkDICOMDatasetSorter.h"
#include "mitkDICOMSortCriterion.h"

#include "mitkVector.h"

#include <gdcmScanner.h>

namespace mitk
{

/**
  \brief Split inputs into blocks of equidant slices.

  This kind of splitting is used as a check before loading a DICOM series with ITK ImageSeriesReader.
*/
class DICOMReader_EXPORT EquiDistantBlocksSorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( EquiDistantBlocksSorter, DICOMDatasetSorter )
    itkNewMacro( EquiDistantBlocksSorter )

    virtual DICOMTagList GetTagsOfInterest();

    virtual void Sort();

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
        DICOMDatasetList GetBlockFilenames();

        /**
          \brief Remaining files, which could not be grouped.
         */
        DICOMDatasetList GetUnsortedFilenames();

        /**
          \brief Wheter or not the grouped result contain a gantry tilt.
         */
        bool ContainsGantryTilt();

        /**
          \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
         */
        void AddFileToSortedBlock(DICOMDatasetAccess* dataset); // TODO Dataset!

        /**
          \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
         */
        void AddFileToUnsortedBlock(DICOMDatasetAccess* dataset);
        void AddFilesToUnsortedBlock(const DICOMDatasetList& datasets);

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

        DICOMDatasetList m_GroupedFiles;
        DICOMDatasetList m_UnsortedFiles;

        bool m_GantryTilt;
    };

    /**
      \brief Ensure an equal z-spacing for a group of files.

      Takes as input a number of images, which are all equally oriented and spatially sorted along their normal direction.

      Internally used by GetSeries. Returns two lists: the first one contins slices of equal inter-slice spacing.
      The second list contains remaining files, which need to be run through AnalyzeFileForITKImageSeriesReaderSpacingAssumption again.

      Relevant code that is matched here is in
      itkImageSeriesReader.txx (ImageSeriesReader<TOutputImage>::GenerateOutputInformation(void)), lines 176 to 245 (as of ITK 3.20)
     */
    SliceGroupingAnalysisResult
    AnalyzeFileForITKImageSeriesReaderSpacingAssumption(const DICOMDatasetList& files, bool groupsOfSimilarImages);

    /**
      \brief Safely convert const char* to std::string.
     */
    std::string
    ConstCharStarToString(const char* s);

    /**
      \brief Convert DICOM string describing a point to Point3D.

      DICOM tags like ImagePositionPatient contain a position as float numbers separated by backslashes:
      \verbatim
      42.7131\13.77\0.7
      \endverbatim
     */
    Point3D
    DICOMStringToPoint3D(const std::string& s, bool& successful);

    /**
      \brief Convert DICOM string describing a point two Vector3D.

      DICOM tags like ImageOrientationPatient contain two vectors as float numbers separated by backslashes:
      \verbatim
      42.7131\13.77\0.7\137.76\0.3
      \endverbatim
     */
    void
    DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful);

    EquiDistantBlocksSorter();
    virtual ~EquiDistantBlocksSorter();

    EquiDistantBlocksSorter(const EquiDistantBlocksSorter& other);
    EquiDistantBlocksSorter& operator=(const EquiDistantBlocksSorter& other);
};

}

#endif
