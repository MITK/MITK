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


#ifndef MITKITKFILEREADERSERVICE_H
#define MITKITKFILEREADERSERVICE_H

#include "mitkAbstractFileReader.h"

namespace mitk {

// This class represents ITK image file reader instances
// registerred via the ITK object factory system as a micro
// service.
class ItkFileReaderService : public AbstractFileReader
{

public:

  ItkFileReaderService(const ItkFileReaderService& other);

  ItkFileReaderService(const std::vector<std::string>& extensions, const std::string& category);
  virtual ~ItkFileReaderService();

  using AbstractFileReader::Read;
  virtual std::vector<itk::SmartPointer<BaseData> > Read(const std::string& path);

  virtual std::vector<itk::SmartPointer<BaseData> > Read(std::istream& stream);

  using AbstractFileReader::CanRead;
  virtual bool CanRead(const std::string& path) const;

private:

  ItkFileReaderService* Clone() const;

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;

};

} // namespace mitk

#endif /* MITKITKFILEREADERSERVICE_H */
