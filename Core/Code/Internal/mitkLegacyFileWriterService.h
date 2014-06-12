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


#ifndef LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521
#define LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkCoreExports.h>

// MITK
#include <mitkAbstractFileWriter.h>
#include <mitkFileWriterWithInformation.h>

namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class LegacyFileWriterService : public mitk::AbstractFileWriter
{
public:

  LegacyFileWriterService(mitk::FileWriterWithInformation::Pointer legacyWriter, const std::string& basedataType,
                          const std::string& extension, const std::string& description);

  ~LegacyFileWriterService();

  using AbstractFileWriter::Write;

  virtual void Write(const BaseData* data, const std::string& path);

  virtual void Write(const BaseData* data, std::ostream& stream );

  virtual bool CanWrite(const BaseData *data) const;

protected:

    mitk::FileWriterWithInformation::Pointer m_LegacyWriter;

private:

    LegacyFileWriterService* Clone() const;

    us::ServiceRegistration<IFileWriter> m_ServiceRegistration;
};

} // namespace mitk

#endif /* LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521 */
