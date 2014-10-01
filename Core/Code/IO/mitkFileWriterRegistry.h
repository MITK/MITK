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

#ifndef FileWriterRegistry_H_HEADER_INCLUDED_C1E7E521
#define FileWriterRegistry_H_HEADER_INCLUDED_C1E7E521

#include <MitkCoreExports.h>

// Microservices
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <usServiceObjects.h>

#include "mitkIFileWriter.h"

namespace mitk {
  class BaseData;
}

namespace mitk {

/**
 * @ingroup IO
 *
 * Provides convenient access to mitk::IFileWriter instances and writing
 * files from mitk::BaseData types.
 *
 * \note The life-time of all mitk::IFileWriter objects returned by an
 * instance of this class ends with the destruction of that instance.
 */
class MITK_CORE_EXPORT FileWriterRegistry
{

public:

  typedef us::ServiceReference<IFileWriter> WriterReference;

  FileWriterRegistry();
  ~FileWriterRegistry();

  static std::vector<WriterReference> GetReferences(const BaseData* baseData, us::ModuleContext* context = us::GetModuleContext());
  static std::vector<WriterReference> GetReferences(const BaseData* baseData, const std::string& mimeType, us::ModuleContext* context = us::GetModuleContext());

  IFileWriter* GetWriter(const WriterReference& ref, us::ModuleContext* context = us::GetModuleContext());

  std::vector<IFileWriter*> GetWriters(const BaseData* baseData, const std::string& mimeType, us::ModuleContext* context = us::GetModuleContext());

  void UngetWriter(IFileWriter* writer);
  void UngetWriters(const std::vector<IFileWriter*>& writers);

private:

  // purposely not implemented
  FileWriterRegistry(const FileWriterRegistry&);
  FileWriterRegistry& operator=(const FileWriterRegistry&);

  std::map<IFileWriter*, us::ServiceObjects<IFileWriter> > m_ServiceObjects;
};

} // namespace mitk

#endif /* FileWriterRegistry_H_HEADER_INCLUDED_C1E7E521 */
