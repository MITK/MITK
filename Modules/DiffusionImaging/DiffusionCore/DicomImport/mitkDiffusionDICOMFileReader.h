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

#ifndef MITKDIFFUSIONDICOMFILEREADER_H
#define MITKDIFFUSIONDICOMFILEREADER_H

#include "MitkDiffusionCoreExports.h"

#include "mitkDICOMITKSeriesGDCMReader.h"
#include "mitkDiffusionHeaderDICOMFileReader.h"
#include "mitkThreeDnTDICOMSeriesReader.h"

namespace mitk
{

class MitkDiffusionCore_EXPORT DiffusionDICOMFileReader
        : public ThreeDnTDICOMSeriesReader
{
public:

  mitkClassMacro( DiffusionDICOMFileReader, DICOMITKSeriesGDCMReader )
  mitkCloneMacro( DiffusionDICOMFileReader )

  itkNewMacro( DiffusionDICOMFileReader )

  virtual void AnalyzeInputFiles();

  virtual bool LoadImages();

  virtual bool CanHandleFile(const std::string &filename);

  void SetResolveMosaic( bool flag )
  {
    m_ResolveMosaic = flag;
  }

protected:
    DiffusionDICOMFileReader();
    virtual ~DiffusionDICOMFileReader();

    bool LoadSingleOutputImage( DiffusionHeaderDICOMFileReader::DICOMHeaderListType, DICOMImageBlockDescriptor&, bool);

    //mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType m_RetrievedHeader;

    std::vector< mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType > m_OutputHeaderContainer;
    std::vector< mitk::DiffusionHeaderDICOMFileReader::Pointer> m_OutputReaderContainer;
    std::vector< bool > m_IsMosaicData;

    bool m_ResolveMosaic;
};

}

#endif // MITKDIFFUSIONDICOMFILEREADER_H
