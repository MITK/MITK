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


#ifndef mitkDicomRTPlanReader_h
#define mitkDicomRTPlanReader_h

#include "mitkAbstractFileReader.h"

#include <string>
#include <vector>
#include <tuple>

#include "mitkImage.h"
#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

#include "MitkDicomRTExports.h"

namespace mitk
{
    struct DICOMDatasetFinding;
    class DICOMTagPath;

    class MITKDICOMRT_EXPORT RTPlanReader : public mitk::AbstractFileReader
  {

  public:
      RTPlanReader();
      RTPlanReader(const RTPlanReader& other);

      using AbstractFileReader::Read;
      virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

      virtual ~RTPlanReader();

  private:
    RTPlanReader* Clone() const override;

    DICOMTagPath GenerateDicomTagPath(unsigned int tag1, unsigned int tag2, unsigned int sqTag1, unsigned int sqTag2) const;
    std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> > GeneratePathsOfInterest() const;
    std::vector<DICOMTagPath> ExtractDicomPathList(const std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> >& pathsOfInterestInformation) const;
    std::vector<std::tuple<std::string, std::string, std::list<DICOMDatasetFinding> > > ReadPathsOfInterest(const std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> >& pathsOfInterestInformation, const DICOMDatasetAccessingImageFrameList& frames) const;
    void SetProperties(Image::Pointer dummyImage, const std::vector<std::tuple<std::string, std::string, std::list<DICOMDatasetFinding> > >& findings) const;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };
}

#endif
