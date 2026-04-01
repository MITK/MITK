/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIFileIO_h
#define mitkIFileIO_h

#include <MitkCoreExports.h>
#include <mitkMessage.h>

#include <usAny.h>

#include <map>
#include <string>

namespace mitk
{
  /**
   * \ingroup IO
   *
   * \brief Common interface for all MITK file readers and writers.
   *
   * This interface defines the shared API for file I/O operations in MITK,
   * including confidence levels for format support, user-configurable options,
   * and progress reporting. It is not meant to be implemented directly. It is
   * inherited from IFileWriter and IFileReader, and a default implementation is
   * provided in AbstractFileIO via AbstractFileWriter and AbstractFileReader.
   *
   * \sa IFileReader
   * \sa IFileWriter
   * \sa AbstractFileIO
   * \sa AbstractFileReader
   * \sa AbstractFileWriter
   */
  struct MITKCORE_EXPORT IFileIO
  {
    // The order of the enum values is important: it is used
    // to rank writer implementations

    /**
     * \brief A confidence level describing the reader's or writer's ability
     *        to handle the given data or file format.
     *
     * Higher values indicate stronger confidence. The level is used to rank
     * multiple competing reader or writer implementations so that the most
     * suitable one is selected automatically.
     */
    enum ConfidenceLevel
    {
      Unsupported = 0,        ///< The reader/writer cannot handle the data.
      PartiallySupported = 8, ///< The reader/writer can handle the data with limitations.
      Supported = 16          ///< The reader/writer fully supports the data.
    };

    /**
     * \brief Options for reading or writing data.
     *
     * Options are key-value pairs used in reader or writer implementations to
     * customize certain aspects of the reading/writing process. Options may be
     * supplied by users calling the SetOption() method or they may be presented
     * as an input mask in the context of a graphical user interface.
     *
     * The keys are human-readable labels and the type of the option value
     * specifies the user interface representation. If a key contains multiple
     * segments (sub-strings separated by a '.' character), only the last segment
     * is displayed. If the last segment is equal to the string "enum", the option
     * is treated as a finite enumeration of values where the values are contained
     * in a corresponding \c std::vector<std::string> value. The same option key
     * without the "enum" segment is considered to hold the current selection from
     * the enumeration.
     */
    typedef std::map<std::string, us::Any> Options;

    /**
     * \brief Callback delegate type for progress reporting.
     *
     * The delegate receives a float value between 0.0 and 1.0 indicating the
     * current progress of a read or write operation.
     */
    typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

    virtual ~IFileIO();

    /**
     * \brief Get the confidence level of this reader or writer for the current input.
     * \return The confidence level indicating how well this implementation can handle
     *         the current data or file format.
     *
     * The level is used to rank multiple competing implementations. A higher
     * confidence level results in the reader or writer being preferred.
     */
    virtual ConfidenceLevel GetConfidenceLevel() const = 0;

    /**
     * \brief Get the current options map.
     * \return A map of option names to their current values.
     *
     * The returned map contains both default and user-set options.
     */
    virtual Options GetOptions() const = 0;

    /**
     * \brief Get the value of a specific option.
     * \param[in] name The name of the option.
     * \return The current value of the option.
     */
    virtual us::Any GetOption(const std::string &name) const = 0;

    /**
     * \brief Set multiple options at once.
     * \param[in] options A map of option names to their new values.
     */
    virtual void SetOptions(const Options &options) = 0;

    /**
     * \brief Set the value of a specific option.
     * \param[in] name The name of the option.
     * \param[in] value The new value for the option.
     */
    virtual void SetOption(const std::string &name, const us::Any &value) = 0;

    /**
     * \brief Register a progress callback.
     * \param[in] callback The callback delegate to add.
     *
     * The callback will be invoked during read or write operations to report
     * progress.
     */
    virtual void AddProgressCallback(const ProgressCallback &callback) = 0;

    /**
     * \brief Remove a previously registered progress callback.
     * \param[in] callback The callback delegate to remove.
     */
    virtual void RemoveProgressCallback(const ProgressCallback &callback) = 0;

    /**
     * \brief Service property name for a human-readable description.
     *
     * The property value must be of type \c std::string.
     *
     * \return The property name string.
     */
    static std::string PROP_DESCRIPTION();

    /**
     * \brief Service property name for the mime-type associated with this reader or writer.
     *
     * The property value must be of type \c std::string and must correspond to
     * a registered CustomMimeType name.
     *
     * \return The property name string.
     *
     * \sa CustomMimeType
     */
    static std::string PROP_MIMETYPE();
  };
}

#endif
