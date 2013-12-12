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

// TODO ensure "C" locale!!
// TODO 3D+t

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

    void EnsureMandatorySortersArePresent();

  private:

    bool m_FixTiltByShearing;
    bool m_Group3DplusT;

    typedef std::list<DICOMGDCMImageFrameList> SortingBlockList;
    SortingBlockList m_SortingResultInProgress;

    typedef std::list<DICOMDatasetSorter::Pointer> SorterList;
    SorterList m_Sorter;

    mitk::EquiDistantBlocksSorter::Pointer m_EquiDistantBlocksSorter;
};

}

#endif
