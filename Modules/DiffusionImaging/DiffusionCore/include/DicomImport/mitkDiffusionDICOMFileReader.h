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
#include "mitkClassicDICOMSeriesReader.h"

namespace mitk
{

class MITKDIFFUSIONCORE_EXPORT DiffusionDICOMFileReader
        : public ClassicDICOMSeriesReader
{
public:

  mitkClassMacro( DiffusionDICOMFileReader, ClassicDICOMSeriesReader )
  mitkCloneMacro( DiffusionDICOMFileReader )

  itkNewMacro( DiffusionDICOMFileReader )

  virtual void AnalyzeInputFiles() override;

  virtual bool LoadImages() override;

  virtual bool CanHandleFile(const std::string &filename) override;

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
