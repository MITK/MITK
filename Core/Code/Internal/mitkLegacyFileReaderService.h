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


#ifndef LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521
#define LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>

// MITK
#include <mitkBaseData.h>
#include <mitkAbstractFileReader.h>


namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class LegacyFileReaderService : public mitk::AbstractFileReader
{

public:

  LegacyFileReaderService(const LegacyFileReaderService& other);

  LegacyFileReaderService(const std::string& mimeType, const std::string& extension, const std::string& description);
  virtual ~LegacyFileReaderService();

  using mitk::AbstractFileReader::Read;

  virtual std::vector<itk::SmartPointer<BaseData> > Read(const std::string& path);

  virtual std::vector<itk::SmartPointer<BaseData> > Read(std::istream& stream);

protected:

  us::ServiceRegistration<IMimeType> RegisterMimeType(us::ModuleContext* context);

private:

  LegacyFileReaderService* Clone() const;

  std::vector<mitk::BaseData::Pointer> ReadFileSeriesTypeDCM(const std::string& path);

  std::vector<mitk::BaseData::Pointer> LoadBaseDataFromFile(const std::string& path);

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
};

} // namespace mitk

#endif /* LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521 */
