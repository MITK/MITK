/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileWriterRegistry_h
#define mitkFileWriterRegistry_h

#include <MitkCoreExports.h>

// Microservices
#include <usGetModuleContext.h>
#include <usServiceObjects.h>
#include <usServiceReference.h>

#include <mitkIFileWriter.h>

namespace mitk
{
  class BaseData;
}

namespace mitk
{
  /**
   * \ingroup IO
   *
   * \brief Provides convenient access to mitk::IFileWriter instances and writing
   * files from mitk::BaseData types.
   *
   * \note The life-time of all mitk::IFileWriter objects returned by an
   * instance of this class ends with the destruction of that instance.
   *
   * \sa IFileWriter
   */
  class MITKCORE_EXPORT FileWriterRegistry
  {
  public:
    typedef us::ServiceReference<IFileWriter> WriterReference;

    /** \brief Constructor. */
    FileWriterRegistry();

    /** \brief Destructor. Releases all acquired service objects. */
    ~FileWriterRegistry();

    /**
     * \brief Get service references for writers that can handle the given BaseData.
     *
     * \param baseData The data object for which to find compatible writers.
     * \param context The module context to use for service lookup.
     * \return A vector of matching writer service references.
     */
    static std::vector<WriterReference> GetReferences(const BaseData *baseData,
                                                      us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Get service references for writers that can handle the given BaseData and MIME type.
     *
     * \param baseData The data object for which to find compatible writers.
     * \param mimeType The MIME type to filter by.
     * \param context The module context to use for service lookup.
     * \return A vector of matching writer service references.
     */
    static std::vector<WriterReference> GetReferences(const BaseData *baseData,
                                                      const std::string &mimeType,
                                                      us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Get a writer instance from a service reference.
     *
     * \param ref The writer service reference.
     * \param context The module context to use for service lookup.
     * \return A pointer to the writer instance, or nullptr if the reference is invalid.
     */
    IFileWriter *GetWriter(const WriterReference &ref, us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Get all writer instances for the given BaseData and MIME type.
     *
     * \param baseData The data object for which to find compatible writers.
     * \param mimeType The MIME type to filter by.
     * \param context The module context to use for service lookup.
     * \return A vector of writer pointers sorted by service ranking.
     */
    std::vector<IFileWriter *> GetWriters(const BaseData *baseData,
                                          const std::string &mimeType,
                                          us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Release a previously acquired writer instance.
     *
     * \param writer The writer to release.
     */
    void UngetWriter(IFileWriter *writer);

    /**
     * \brief Release multiple previously acquired writer instances.
     *
     * \param writers The vector of writers to release.
     */
    void UngetWriters(const std::vector<IFileWriter *> &writers);

  private:
    // purposely not implemented
    FileWriterRegistry(const FileWriterRegistry &);
    FileWriterRegistry &operator=(const FileWriterRegistry &);

    std::map<IFileWriter *, us::ServiceObjects<IFileWriter>> m_ServiceObjects;
  };

} // namespace mitk

#endif
