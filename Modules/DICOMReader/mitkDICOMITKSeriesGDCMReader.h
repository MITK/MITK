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

#include "DICOMReaderExports.h"

// TODO tests seem to pass if reader creates NO output at all!
// TODO ensure "C" locale!!
// TODO providing tags as properties!
// TODO preloaded volumes?? could be solved in a different way..

namespace mitk
{

class DICOMReader_EXPORT DICOMITKSeriesGDCMReader : public DICOMFileReader
{
  public:

    mitkClassMacro( DICOMITKSeriesGDCMReader, DICOMFileReader );
    mitkCloneMacro( DICOMITKSeriesGDCMReader );
    itkNewMacro( DICOMITKSeriesGDCMReader );

    virtual void AnalyzeInputFiles();

    // void AllocateOutputImages();
    virtual bool LoadImages();

    virtual bool CanHandleFile(const std::string& filename);

    virtual void AddSortingElement(DICOMDatasetSorter* sorter);

    void SetFixTiltByShearing(bool on);

  protected:

    DICOMITKSeriesGDCMReader();
    virtual ~DICOMITKSeriesGDCMReader();

    DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other);
    DICOMITKSeriesGDCMReader& operator=(const DICOMITKSeriesGDCMReader& other);

    DICOMDatasetList ToDICOMDatasetList(DICOMGDCMImageFrameList& input);
    DICOMGDCMImageFrameList FromDICOMDatasetList(DICOMDatasetList& input);
    DICOMImageFrameList ToDICOMImageFrameList(DICOMGDCMImageFrameList& input);

    typedef std::list<DICOMGDCMImageFrameList> SortingBlockList;
    /// \return REMAINING blocks
    virtual SortingBlockList Condense3DBlocks(SortingBlockList& resultOf3DGrouping);

    void EnsureMandatorySortersArePresent();

    virtual bool LoadMitkImageForOutput(unsigned int o);

    Image::Pointer FixupSpacing(Image* mitkImage, const DICOMImageBlockDescriptor& block) const;

  private:

  protected:

    // NOT nice, made available to ThreeDnTDICOMSeriesReader due to lack of time
    bool m_FixTiltByShearing; // could be removed by ITKDICOMSeriesReader NOT flagging tilt unless requested to fix it!

  private:
    bool m_3DBlocksWereCondensed;

    SortingBlockList m_SortingResultInProgress;

    typedef std::list<DICOMDatasetSorter::Pointer> SorterList;
    SorterList m_Sorter;

  protected:

    // NOT nice, made available to ThreeDnTDICOMSeriesReader due to lack of time
    mitk::EquiDistantBlocksSorter::Pointer m_EquiDistantBlocksSorter;

  private:
};

}

#endif
