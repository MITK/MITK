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

#ifndef MITKIFILEIO_H
#define MITKIFILEIO_H

#include <MitkCoreExports.h>
#include <mitkMessage.h>

#include <usAny.h>

#include <map>
#include <string>

namespace mitk {

/**
 * \ingroup IO
 *
 * \brief Common interface for all MITK file reader and writer.
 *
 * This interface is not meant to be implemented directly. It is inherited
 * from IFileWriter and IFileReader and a default implementation is provided
 * in AbstractFileIO via AbstractFileWriter and AbstractFileReader.
 */
struct MITKCORE_EXPORT IFileIO
{
  // The order of the enum values is important: it is used
  // to rank writer implementations

  /**
   * @brief A confidence level describing the confidence of the reader or writer
   * in handling the given data.
   *
   */
  enum ConfidenceLevel
  {
    Unsupported = 0,
    PartiallySupported = 8,
    Supported = 16
  };

  /**
   * @brief Options for reading or writing data.
   *
   * Options are used in reader or writer implementations to customize
   * certain aspects of the reading/writing process. Options may be supplied
   * by users calling the SetOption() method or they may be presented
   * as an input mask in the context of a graphical user interface.
   *
   * The keys are human-readable labels and the type of the option value
   * specifies the user interface representation. If a key contains multiple
   * segments (sub-strings separated by a '.' character, only the last segment
   * is displayed. If the last segment is equal to the string 'enum', the option
   * is treated as a finite enumeration of values where the values are contained
   * in a corresponding \c std::vector<std::string> value. the same option key
   * without the 'enum' segment is considered to hold the current selection from
   * the enumeration.
   */
  typedef std::map<std::string, us::Any> Options;

  typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

  virtual ~IFileIO();

  /**
   * @brief The confidence level of the reader or writer implementation.
   * @return Confidence level.
   *
   * The level is used to rank multiple competing implementations.
   */
  virtual ConfidenceLevel GetConfidenceLevel() const = 0;

  /**
   * \brief returns a list of the supported options
   *
   * Options are strings that are treated as flags when passed to the write method.
   */
  virtual Options GetOptions() const = 0;

  virtual us::Any GetOption(const std::string& name) const = 0;

  virtual void SetOptions(const Options& options) = 0;

  virtual void SetOption(const std::string& name, const us::Any& value) = 0;

  virtual void AddProgressCallback(const ProgressCallback& callback) = 0;

  virtual void RemoveProgressCallback(const ProgressCallback& callback) = 0;

  /**
   * @brief Service property name for a description.
   *
   * The property value must be of type \c std::string.
   *
   * @return The property name.
   */
  static std::string PROP_DESCRIPTION();

  /**
   * @brief Service property name for the mime-type associated with this file writer.
   *
   * The property value must be of type \c std::string.
   *
   * @return The property name.
   */
  static std::string PROP_MIMETYPE();

};

}

#endif // MITKIFILEIO_H
