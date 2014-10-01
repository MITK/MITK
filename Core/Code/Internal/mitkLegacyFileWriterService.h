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

#include <mitkAbstractFileWriter.h>
#include <mitkFileWriter.h>

namespace mitk {

class LegacyFileWriterService : public mitk::AbstractFileWriter
{
public:

  LegacyFileWriterService(mitk::FileWriter::Pointer legacyWriter,
                          const std::string& description);

  ~LegacyFileWriterService();

  using AbstractFileWriter::Write;

  virtual void Write();

  virtual ConfidenceLevel GetConfidenceLevel() const;

private:

    LegacyFileWriterService* Clone() const;

    mitk::FileWriter::Pointer m_LegacyWriter;

    us::ServiceRegistration<IFileWriter> m_ServiceRegistration;
};

} // namespace mitk

#endif /* LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521 */
