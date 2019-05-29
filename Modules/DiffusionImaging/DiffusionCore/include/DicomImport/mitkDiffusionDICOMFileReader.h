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

  void AnalyzeInputFiles() override;

  bool LoadImages() override;

  bool CanHandleFile(const std::string &filename) override;

  void SetResolveMosaic( bool flag )
  {
    m_ResolveMosaic = flag;
  }

  void SetApplyRotationToGradients( bool apply )
  {
    m_ApplyRotationToGradients = apply;
  }

  std::string GetStudyName(int i){ return m_Study_names.at(i); }
  std::string GetSeriesName(int i){ return m_Series_names.at(i); }

  std::vector<std::string> sop_instance_uids() const;
  std::vector<std::string> frame_of_reference_uids() const;
  std::vector<std::string> series_instance_uids() const;
  std::vector<std::string> study_instance_uids() const;
  std::vector<std::string> patient_names() const;
  std::vector<std::string> patient_ids() const;

protected:
  DiffusionDICOMFileReader();
  ~DiffusionDICOMFileReader() override;

  bool LoadSingleOutputImage( DiffusionHeaderDICOMFileReader::DICOMHeaderListType, DICOMImageBlockDescriptor&, bool);

  //mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType m_RetrievedHeader;

  std::vector< mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType > m_OutputHeaderContainer;
  std::vector< mitk::DiffusionHeaderDICOMFileReader::Pointer> m_OutputReaderContainer;
  std::vector< bool > m_IsMosaicData;
  std::vector< std::string > m_Study_names;
  std::vector< std::string > m_Series_names;

  bool m_ResolveMosaic;
  bool m_ApplyRotationToGradients;

  std::vector< std::string > m_sop_instance_uids;
  std::vector< std::string > m_frame_of_reference_uids;
  std::vector< std::string > m_series_instance_uids;
  std::vector< std::string > m_study_instance_uids;
  std::vector< std::string > m_patient_names;
  std::vector< std::string > m_patient_ids;
};

}

#endif // MITKDIFFUSIONDICOMFILEREADER_H
