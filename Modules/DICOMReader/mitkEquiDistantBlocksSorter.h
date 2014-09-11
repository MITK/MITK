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

namespace mitk
{

/**
 \ingroup DICOMReaderModule
 \brief Split inputs into blocks of equidistant slices (for use in DICOMITKSeriesGDCMReader).

 Since inter-slice distance is not recorded in DICOM tags, we must ensure that blocks are made up of
 slices that have equal distances between neighboring slices. This is especially necessary because itk::ImageSeriesReader
 is later used for the actual loading, and this class expects (and does nocht verify) equal inter-slice distance (see \ref DICOMITKSeriesGDCMReader_ForcedConfiguration).

 To achieve such grouping, the inter-slice distance is calculated from the first two different slice positions of a block.
 Following slices are added to a block as long as they can be added by adding the calculated inter-slice distance to the
 last slice of the block. Slices that do not fit into the expected distance pattern, are set aside for further analysis.
 This grouping is done until each file has been assigned to a group.

 Slices that share a position in space are also sorted into separate blocks during this step.
 So the result of this step is a set of blocks that contain only slices with equal z spacing
 and uniqe slices at each position.

 During sorting, the origins (documented in tag image position patient) are compared
 against expected origins (from former origin plus moving direction). As there will
 be minor differences in numbers (from both calculations and unprecise tag values),
 we must be a bit tolerant here. The default behavior is to expect that an origin is
 not further away from the expected position than 30% of the inter-slice distance.
 To support a legacy behavior of a former loader (DicomSeriesReader), this default can
 be restricted to a constant number of millimeters by calling SetToleratedOriginOffset(mm).

 Detailed implementation in AnalyzeFileForITKImageSeriesReaderSpacingAssumption().

*/
class MitkDICOMReader_EXPORT EquiDistantBlocksSorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( EquiDistantBlocksSorter, DICOMDatasetSorter )
    itkNewMacro( EquiDistantBlocksSorter )

    virtual DICOMTagList GetTagsOfInterest();

    /**
      \brief Delegates work to AnalyzeFileForITKImageSeriesReaderSpacingAssumption().
      AnalyzeFileForITKImageSeriesReaderSpacingAssumption() is called until it does not
      create multiple blocks anymore.
    */
    virtual void Sort();

    /**
      \brief Whether or not to accept images from a tilted acquisition in a single output group.
    */
    void SetAcceptTilt(bool accept);
    bool GetAcceptTilt() const;

    /**
      \brief See class description and SetToleratedOriginOffset().
    */
    void SetToleratedOriginOffsetToAdaptive(double fractionOfInterSliceDistanct = 0.3);
    /**
      \brief See class description and SetToleratedOriginOffsetToAdaptive().

      Default value of 0.005 is calculated so that we get a maximum of 1/10mm
      error when having a measurement crosses 20 slices in z direction (too strict? we don't know better..).
    */
    void SetToleratedOriginOffset(double millimeters = 0.005);

    double GetToleratedOriginOffset() const;
    bool IsToleratedOriginOffsetAbsolute() const;

    void SetAcceptTwoSlicesGroups(bool accept);
    bool GetAcceptTwoSlicesGroups() const;

    virtual void PrintConfiguration(std::ostream& os, const std::string& indent = "") const;

    virtual bool operator==(const DICOMDatasetSorter& other) const;

  protected:

    /**
      \brief Return type of AnalyzeFileForITKImageSeriesReaderSpacingAssumption().

      Class contains the grouping result of method AnalyzeFileForITKImageSeriesReaderSpacingAssumption(),
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
        DICOMDatasetList GetBlockDatasets();

        void SetFirstFilenameOfBlock(const std::string& filename);
        std::string GetFirstFilenameOfBlock() const;
        void SetLastFilenameOfBlock(const std::string& filename);
        std::string GetLastFilenameOfBlock() const;

        /**
          \brief Remaining files, which could not be grouped.
         */
        DICOMDatasetList GetUnsortedDatasets();

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

        GantryTiltInformation m_TiltInfo;
        std::string m_FirstFilenameOfBlock;
        std::string m_LastFilenameOfBlock;
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

    double m_ToleratedOriginOffset;
    bool m_ToleratedOriginOffsetIsAbsolute;

    bool m_AcceptTwoSlicesGroups;
};

}

#endif
