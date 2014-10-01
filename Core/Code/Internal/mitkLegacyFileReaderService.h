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


#ifndef MITKLEGACYFILEREADERSERVICE_H
#define MITKLEGACYFILEREADERSERVICE_H

#include <mitkAbstractFileReader.h>

namespace mitk {

// This class wraps mitk::FileReader instances registered as
// "mitkIOAdapter" via the ITK object factory system as a
// micro service.
class LegacyFileReaderService : public mitk::AbstractFileReader
{

public:

  LegacyFileReaderService(const LegacyFileReaderService& other);

  LegacyFileReaderService(const std::vector<std::string>& extensions, const std::string& category);
  virtual ~LegacyFileReaderService();

  using AbstractFileReader::Read;
  virtual std::vector<itk::SmartPointer<BaseData> > Read();

private:

  LegacyFileReaderService* Clone() const;

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
};

} // namespace mitk

#endif /* MITKLEGACYFILEREADERSERVICE_H */
