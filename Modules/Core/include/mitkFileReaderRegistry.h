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
   * \ingroup IO
   *
   * \brief Registry that provides convenient access to IFileReader service instances.
   *
   * FileReaderRegistry manages the life-cycle of IFileReader service objects
   * obtained from the CppMicroServices registry. It provides methods to look
   * up readers by mime-type and to resolve the best-matching mime-type for a
   * given file path.
   *
   * \note The life-time of all mitk::IFileReader objects returned by an
   * instance of this class ends with the destruction of that instance.
   * Readers obtained via GetReader() or GetReaders() must not be used after
   * the owning FileReaderRegistry is destroyed, or after UngetReader() /
   * UngetReaders() has been called on them.
   *
   * \sa IFileReader
   * \sa FileReaderSelector
   * \sa MimeType
   */
  class MITKCORE_EXPORT FileReaderRegistry
  {
  public:
    /** \brief Service reference type for IFileReader services. */
    typedef us::ServiceReference<IFileReader> ReaderReference;

    /** \brief Construct a FileReaderRegistry. */
    FileReaderRegistry();

    /**
     * \brief Destroy the registry, releasing all obtained reader service objects.
     */
    ~FileReaderRegistry();

    /**
     * \brief Get the highest-ranked mime-type for the given file path.
     * \param[in] path The file path to match (must not be empty).
     * \param[in] context The module context for service look-up. Defaults to the
     *            current module context.
     * \return The highest-ranked MimeType whose extensions match \c path,
     *         or an invalid MimeType if none matched.
     * \throw mitk::Exception if \c path is empty.
     */
    static MimeType GetMimeTypeForFile(const std::string &path, us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Get service references for all readers supporting the given mime-type.
     * \param[in] mimeType The mime-type to filter by.
     * \param[in] context The module context for service look-up.
     * \return A vector of service references for matching IFileReader services.
     */
    static std::vector<ReaderReference> GetReferences(const MimeType &mimeType,
                                                      us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Get a reader instance for the given service reference.
     * \param[in] ref The service reference to resolve.
     * \param[in] context The module context for service look-up.
     * \return A pointer to the IFileReader instance. The reader is owned by this
     *         registry and must be released via UngetReader().
     */
    mitk::IFileReader *GetReader(const ReaderReference &ref, us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Get all reader instances for the given mime-type, sorted by ranking.
     * \param[in] mimeType The mime-type to filter by.
     * \param[in] context The module context for service look-up.
     * \return A vector of IFileReader pointers, sorted by descending service ranking.
     *         Each reader is owned by this registry and must be released via
     *         UngetReader() or UngetReaders().
     */
    std::vector<mitk::IFileReader *> GetReaders(const MimeType &mimeType,
                                                us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Release a reader previously obtained via GetReader() or GetReaders().
     * \param[in] reader The reader to release.
     */
    void UngetReader(mitk::IFileReader *reader);

    /**
     * \brief Release multiple readers previously obtained via GetReaders().
     * \param[in] readers The readers to release.
     */
    void UngetReaders(const std::vector<mitk::IFileReader *> &readers);

  private:
    // purposely not implemented
    FileReaderRegistry(const FileReaderRegistry &);
    FileReaderRegistry &operator=(const FileReaderRegistry &);

    std::map<mitk::IFileReader *, us::ServiceObjects<mitk::IFileReader>> m_ServiceObjects;
  };

} // namespace mitk

#endif
