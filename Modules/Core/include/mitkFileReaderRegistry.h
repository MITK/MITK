/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileReaderRegistry_h
#define mitkFileReaderRegistry_h

#include <MitkCoreExports.h>
#include <mitkBaseData.h>

#include <mitkIFileReader.h>

// Microservices
#include <usGetModuleContext.h>
#include <usServiceObjects.h>
#include <usServiceReference.h>

namespace mitk
{
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
  class MITKCORE_EXPORT FileReaderRegistry
  {
  public:
    typedef us::ServiceReference<IFileReader> ReaderReference;

    FileReaderRegistry();
    ~FileReaderRegistry();

    /**
     * @brief Get the highest ranked mime-type for the given file name.
     * @param path
     * @param context
     * @return The highest ranked mime-type containing \c extension in
     *         its extension list.
     */
    static MimeType GetMimeTypeForFile(const std::string &path, us::ModuleContext *context = us::GetModuleContext());

    static std::vector<ReaderReference> GetReferences(const MimeType &mimeType,
                                                      us::ModuleContext *context = us::GetModuleContext());

    mitk::IFileReader *GetReader(const ReaderReference &ref, us::ModuleContext *context = us::GetModuleContext());

    std::vector<mitk::IFileReader *> GetReaders(const MimeType &mimeType,
                                                us::ModuleContext *context = us::GetModuleContext());

    void UngetReader(mitk::IFileReader *reader);
    void UngetReaders(const std::vector<mitk::IFileReader *> &readers);

  private:
    // purposely not implemented
    FileReaderRegistry(const FileReaderRegistry &);
    FileReaderRegistry &operator=(const FileReaderRegistry &);

    std::map<mitk::IFileReader *, us::ServiceObjects<mitk::IFileReader>> m_ServiceObjects;
  };

} // namespace mitk

#endif
