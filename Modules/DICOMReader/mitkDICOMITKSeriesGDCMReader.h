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

#include <stack>

// TODO tests seem to pass if reader creates NO output at all!
// TODO preloaded volumes?? could be solved in a different way..

namespace itk
{
  class TimeProbesCollectorBase;
}

namespace mitk
{

class DICOMReader_EXPORT DICOMITKSeriesGDCMReader : public DICOMFileReader, protected DICOMTagCache
{
  public:

    mitkClassMacro( DICOMITKSeriesGDCMReader, DICOMFileReader );
    mitkCloneMacro( DICOMITKSeriesGDCMReader );
    itkNewMacro( DICOMITKSeriesGDCMReader );

    virtual void AnalyzeInputFiles();

    // void AllocateOutputImages();
    virtual bool LoadImages();

    virtual bool CanHandleFile(const std::string& filename);

    virtual void AddSortingElement(DICOMDatasetSorter* sorter, bool atFront = false);

    void SetFixTiltByShearing(bool on);

  protected:

    virtual void InternalPrintConfiguration(std::ostream& os) const;

    virtual std::string GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const;

    std::string GetActiveLocale() const;
    void PushLocale();
    void PopLocale();

    DICOMITKSeriesGDCMReader();
    virtual ~DICOMITKSeriesGDCMReader();

    DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other);
    DICOMITKSeriesGDCMReader& operator=(const DICOMITKSeriesGDCMReader& other);

    DICOMDatasetList ToDICOMDatasetList(const DICOMGDCMImageFrameList& input);
    DICOMGDCMImageFrameList FromDICOMDatasetList(const DICOMDatasetList& input);
    DICOMImageFrameList ToDICOMImageFrameList(const DICOMGDCMImageFrameList& input);

    typedef std::list<DICOMGDCMImageFrameList> SortingBlockList;
    /// \return REMAINING blocks
    virtual SortingBlockList Condense3DBlocks(SortingBlockList& resultOf3DGrouping);

    SortingBlockList InternalExecuteSortingStep(
        unsigned int sortingStepIndex,
        DICOMDatasetSorter::Pointer sorter,
        const SortingBlockList& input,
        itk::TimeProbesCollectorBase* timer);

    void EnsureMandatorySortersArePresent();

    virtual bool LoadMitkImageForOutput(unsigned int o);

    Image::Pointer FixupSpacing(Image* mitkImage, const DICOMImageBlockDescriptor& block) const;

    ReaderImplementationLevel GetReaderImplementationLevel(const std::string uid) const;
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

    std::stack<std::string> m_ReplacedCLocales;
    std::stack<std::locale> m_ReplacedCinLocales;

    DICOMGDCMImageFrameList m_InputFrameList;

    gdcm::Scanner m_GDCMScanner;
};

}

#endif
