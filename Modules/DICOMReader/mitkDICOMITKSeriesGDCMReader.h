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

namespace mitk
{

class DICOMITKSeriesGDCMReader : public DICOMFileReader
{
  public:

    mitkClassMacro( DICOMITKSeriesGDCMReader, DICOMFileReader );
    mitkCloneMacro( DICOMITKSeriesGDCMReader );
    itkNewMacro( DICOMITKSeriesGDCMReader );

    virtual void AnalyzeInputFiles();

    // void AllocateOutputImages();
    virtual bool LoadImages();

    virtual bool CanHandleFile(const std::string& filename);

  protected:

    DICOMITKSeriesGDCMReader();
    virtual ~DICOMITKSeriesGDCMReader();

    DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other);
    DICOMITKSeriesGDCMReader& operator=(const DICOMITKSeriesGDCMReader& other);

  private:

    bool m_FixTiltByShearing;
    bool m_Group3DplusT;

    typedef std::list<DICOMImageFrameList> SortingBlockList;
    SortingBlockList m_SortingResultInProgress;
};

}

#endif
