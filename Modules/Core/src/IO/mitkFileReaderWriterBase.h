/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFileReaderWriterBase_h
#define mitkFileReaderWriterBase_h

#include <mitkCustomMimeType.h>
#include <mitkMessage.h>

#include <usAny.h>
#include <usModuleContext.h>
#include <usServiceRegistration.h>

#include <memory>

namespace mitk
{
  /**
   * \brief Common base class for file reader and writer implementations.
   *
   * Provides shared functionality for file I/O classes, including option management,
   * MIME type handling, service ranking, progress callbacks, and CppMicroServices
   * MIME type registration. This class is used as a common implementation detail by
   * AbstractFileReader and AbstractFileWriter.
   *
   * \sa AbstractFileReader
   * \sa AbstractFileWriter
   * \sa CustomMimeType
   */
  class FileReaderWriterBase
  {
  public:
    /** \brief Map of option names to their values. */
    typedef std::map<std::string, us::Any> Options;

    /** \brief Delegate type for progress notification callbacks. */
    typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

    FileReaderWriterBase();
    virtual ~FileReaderWriterBase();

    /**
     * \brief Get all current options merged with defaults.
     *
     * User-set options override default options. Options not explicitly set
     * fall back to their default values.
     *
     * \return A map of all option names to their current or default values.
     */
    Options GetOptions() const;

    /**
     * \brief Get the value of a specific option.
     *
     * Looks up the option first in user-set options, then in defaults.
     *
     * \param[in] name The option name.
     * \return The option value, or an empty us::Any if not found.
     */
    us::Any GetOption(const std::string &name) const;

    /**
     * \brief Set multiple options at once.
     *
     * Each option is validated against the default options. Unknown options
     * are ignored with a warning.
     *
     * \param[in] options A map of option names to values.
     */
    void SetOptions(const Options &options);

    /**
     * \brief Set a single option value.
     *
     * Only options known from the default options can be set. Unknown options
     * produce a warning. Passing an empty us::Any resets the option to its default.
     *
     * \param[in] name The option name.
     * \param[in] value The new value, or empty to reset to default.
     */
    void SetOption(const std::string &name, const us::Any &value);

    /**
     * \brief Set the default options.
     * \param[in] defaultOptions A map of option names to default values.
     */
    void SetDefaultOptions(const Options &defaultOptions);

    /**
     * \brief Get the default options.
     * \return A map of option names to default values.
     */
    Options GetDefaultOptions() const;

    /**
     * \brief Set the service ranking for this file reader/writer.
     *
     * Default is zero and should only be chosen differently for a reason.
     * The ranking is used to determine which reader to use if several
     * equivalent readers have been found.
     * It may be used to replace a default reader from MITK in your own project.
     * E.g. if you want to use your own reader for nrrd files instead of the default,
     * implement it and give it a higher ranking than zero.
     *
     * \param[in] ranking The ranking value.
     */
    void SetRanking(int ranking);

    /**
     * \brief Get the current service ranking.
     * \return The ranking value.
     */
    int GetRanking() const;

    /**
     * \brief Set the MIME type for this reader/writer.
     * \param[in] mimeType The CustomMimeType to associate.
     */
    void SetMimeType(const CustomMimeType &mimeType);

    /**
     * \brief Get the associated MIME type (const).
     * \return Pointer to the CustomMimeType, or nullptr if not set.
     */
    const CustomMimeType *GetMimeType() const;

    /**
     * \brief Get the associated MIME type (mutable).
     * \return Pointer to the CustomMimeType, or nullptr if not set.
     */
    CustomMimeType *GetMimeType();

    /**
     * \brief Get the MIME type as registered with the service framework.
     *
     * If the MIME type has been registered, returns a MimeType object with
     * ranking and service ID. Otherwise, looks up the MIME type by name
     * in the service registry.
     *
     * \return The registered MimeType, or an invalid MimeType if not found.
     */
    MimeType GetRegisteredMimeType() const;

    /**
     * \brief Set the prefix used for auto-generated MIME type names.
     * \param[in] prefix The MIME type name prefix.
     */
    void SetMimeTypePrefix(const std::string &prefix);

    /**
     * \brief Get the prefix used for auto-generated MIME type names.
     * \return The MIME type name prefix.
     */
    std::string GetMimeTypePrefix() const;

    /**
     * \brief Set a human-readable description of this reader/writer.
     * \param[in] description The description string.
     */
    void SetDescription(const std::string &description);

    /**
     * \brief Get the human-readable description of this reader/writer.
     * \return The description string.
     */
    std::string GetDescription() const;

    /**
     * \brief Register a callback for progress notifications.
     * \param[in] callback The callback delegate to add.
     */
    void AddProgressCallback(const ProgressCallback &callback);

    /**
     * \brief Remove a previously registered progress callback.
     * \param[in] callback The callback delegate to remove.
     */
    void RemoveProgressCallback(const ProgressCallback &callback);

    /**
     * \brief Register the MIME type as a CppMicroServices service.
     *
     * If the MIME type name is empty, a synthetic name is generated from the
     * prefix and the first file extension.
     *
     * \param[in] context The module context for service registration.
     * \return The service registration object.
     * \throw std::invalid_argument if context is nullptr.
     */
    us::ServiceRegistration<CustomMimeType> RegisterMimeType(us::ModuleContext *context);

    /** \brief Unregister a previously registered MIME type service. */
    void UnregisterMimeType();

  protected:
    FileReaderWriterBase(const FileReaderWriterBase &other);

    std::string m_Description;
    int m_Ranking;

    std::string m_MimeTypePrefix;

    /**
     * \brief Options supported by this reader. Set sensible default values!
     *
     * Can be left empty if no special options are required.
     */
    Options m_Options;

    Options m_DefaultOptions;

    // us::PrototypeServiceFactory* m_PrototypeFactory;

    Message1<float> m_ProgressMessage;

    std::unique_ptr<CustomMimeType> m_CustomMimeType;
    us::ServiceRegistration<CustomMimeType> m_MimeTypeReg;

  private:
    // purposely not implemented
    FileReaderWriterBase &operator=(const FileReaderWriterBase &other);
  };
}

#endif
