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
 * @ingroup Process
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

  /**
   * @brief Get the default file name extension for writing.
   *
   * The default extension is computed by retrieving the highest ranked
   * mitk::IMimeType instance for the given mitk::IFileWriter reference
   * and using the first extension from the mime types extension list.
   *
   * @param ref The IFileWriter service reference
   * @param context The us::ModuleContext to look up IMimeType services
   * @return The default extension without a leading period for the given
   *         \c ref. Returns an empty string if there is no registered
   *         mime type with this file writer reference.
   */
  static std::string GetDefaultExtension(const WriterReference& ref, us::ModuleContext* context = us::GetModuleContext());

  static WriterReference GetReference(const BaseData* baseData, us::ModuleContext* context = us::GetModuleContext());
  static WriterReference GetReference(const std::string& baseDataType, us::ModuleContext* context = us::GetModuleContext());

  static std::vector<WriterReference> GetReferences(const BaseData* baseData, us::ModuleContext* context = us::GetModuleContext());
  static std::vector<WriterReference> GetReferences(const std::string& baseDataType, us::ModuleContext* context = us::GetModuleContext());

  IFileWriter* GetWriter(const WriterReference& ref, us::ModuleContext* context = us::GetModuleContext());

  IFileWriter* GetWriter(const std::string& baseDataType, us::ModuleContext* context = us::GetModuleContext());
  IFileWriter* GetWriter(const BaseData* baseData, us::ModuleContext* context = us::GetModuleContext());

  std::vector<IFileWriter*> GetWriters(const std::string& baseDataType, us::ModuleContext* context = us::GetModuleContext());
  std::vector<IFileWriter*> GetWriters(const BaseData* baseData, us::ModuleContext* context = us::GetModuleContext());

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
