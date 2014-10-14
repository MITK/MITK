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


#ifndef FileReaderRegistry_H_HEADER_INCLUDED_C1E7E521
#define FileReaderRegistry_H_HEADER_INCLUDED_C1E7E521

#include <MitkCoreExports.h>
#include <mitkBaseData.h>

#include <mitkIFileReader.h>

// Microservices
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <usServiceObjects.h>


namespace mitk {

class MimeType;

/**
 * @ingroup IO
 *
 * Provides convenient access to mitk::IFileReader instances and reading
 * files into mitk::BaseData types.
 *
 * \note The life-time of all mitk::IFileReader objects returned by an
 * instance of this class ends with the destruction of that instance.
 */
class MITK_CORE_EXPORT FileReaderRegistry
{

public:

  typedef us::ServiceReference<IFileReader> ReaderReference;

  FileReaderRegistry();
  ~FileReaderRegistry();

  /**
   * @brief Get the highest ranked mime-type for the given file name.
   * @param extension A file name extension without a leading dot.
   * @param context
   * @return The highest ranked mime-type containing \c extension in
   *         its extension list.
   */
  static MimeType GetMimeTypeForFile(const std::string& path, us::ModuleContext* context = us::GetModuleContext());

  static std::vector<ReaderReference> GetReferences(const MimeType& mimeType, us::ModuleContext* context = us::GetModuleContext());

  mitk::IFileReader* GetReader(const ReaderReference& ref, us::ModuleContext* context = us::GetModuleContext());

  std::vector<mitk::IFileReader*> GetReaders(const MimeType& mimeType, us::ModuleContext* context = us::GetModuleContext());

  void UngetReader(mitk::IFileReader* reader);
  void UngetReaders(const std::vector<mitk::IFileReader*>& readers);

private:

  // purposely not implemented
  FileReaderRegistry(const FileReaderRegistry&);
  FileReaderRegistry& operator=(const FileReaderRegistry&);

  std::map<mitk::IFileReader*, us::ServiceObjects<mitk::IFileReader> > m_ServiceObjects;

};

} // namespace mitk

#endif /* FileReaderRegistry_H_HEADER_INCLUDED_C1E7E521 */
