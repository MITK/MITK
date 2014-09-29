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

struct MITK_CORE_EXPORT IFileIO
{
  // The order of the enum values is important: it is used
  // to rank writer implementations
  enum ConfidenceLevel
  {
    Unsupported = 0,
    PartiallySupported = 8,
    Supported = 16
  };

  typedef std::map<std::string, us::Any> Options;

  typedef mitk::MessageAbstractDelegate1<float> ProgressCallback;

  virtual ~IFileIO();

  virtual ConfidenceLevel GetConfidenceLevel() const = 0;

  /**
   * \brief returns a list of the supported Options
   *
   * Options are strings that are treated as flags when passed to the write method.
   */
  virtual Options GetOptions() const = 0;

  virtual us::Any GetOption(const std::string& name) const = 0;

  virtual void SetOptions(const Options& options) = 0;

  virtual void SetOption(const std::string& name, const us::Any& value) = 0;

  /**
   * \brief Returns a value between 0 and 1 depending on the progress of the writing process.
   * This method need not necessarily be implemented meaningfully, always returning zero is accepted.
   */
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
