/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLEGACYFILEREADERSERVICE_H
#define MITKLEGACYFILEREADERSERVICE_H

#include <mitkAbstractFileReader.h>

namespace mitk
{
  // This class wraps mitk::FileReader instances registered as
  // "mitkIOAdapter" via the ITK object factory system as a
  // micro service.
  class LegacyFileReaderService : public mitk::AbstractFileReader
  {
  public:
    LegacyFileReaderService(const LegacyFileReaderService &other);

    LegacyFileReaderService(const std::vector<std::string> &extensions, const std::string &category);
    ~LegacyFileReaderService() override;

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    LegacyFileReaderService *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} // namespace mitk

#endif /* MITKLEGACYFILEREADERSERVICE_H */
