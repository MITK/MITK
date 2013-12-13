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
#include "mitkGantryTiltInformation.h"

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

    void SetAcceptTilt(bool accept);
    const GantryTiltInformation GetTiltInformation(const std::string& filename, bool& hasTiltInfo); // TODO ugly, but bool flag will be removed

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
        DICOMDatasetList GetBlockFilenames(); // TODO rename --> ... Dataset ... instead of filename

        void SetFirstFilenameOfBlock(const std::string& filename);
        std::string GetFirstFilenameOfBlock() const;

        /**
          \brief Remaining files, which could not be grouped.
         */
        DICOMDatasetList GetUnsortedFilenames();

        /**
          \brief Wheter or not the grouped result contain a gantry tilt.
         */
        bool ContainsGantryTilt();

        /**
          \brief Detailed description of gantry tilt.
        */
        const GantryTiltInformation& GetTiltInfo() const;

        /**
          \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
         */
        void AddFileToSortedBlock(DICOMDatasetAccess* dataset);

        /**
          \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
         */
        void AddFileToUnsortedBlock(DICOMDatasetAccess* dataset);
        void AddFilesToUnsortedBlock(const DICOMDatasetList& datasets);

        /**
          \brief Meant for internal use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption only.
          \todo Could make sense to enhance this with an instance of GantryTiltInformation to store the whole result!
         */
        void FlagGantryTilt(const GantryTiltInformation& tiltInfo);

        /**
          \brief Only meaningful for use by AnalyzeFileForITKImageSeriesReaderSpacingAssumption.
         */
        void UndoPrematureGrouping();

      protected:

        DICOMDatasetList m_GroupedFiles;
        DICOMDatasetList m_UnsortedFiles;

        bool m_GantryTilt; // TODO make the flag part of GantryTiltInformation!
        GantryTiltInformation m_TiltInfo;
        std::string m_FirstFilenameOfBlock;
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

    EquiDistantBlocksSorter();
    virtual ~EquiDistantBlocksSorter();

    EquiDistantBlocksSorter(const EquiDistantBlocksSorter& other);
    EquiDistantBlocksSorter& operator=(const EquiDistantBlocksSorter& other);

    bool m_AcceptTilt;

    typedef std::vector<SliceGroupingAnalysisResult> ResultsList;
    ResultsList m_SliceGroupingResults;
};

}

#endif
