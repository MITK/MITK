/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUIDGenerator_h
#define mitkUIDGenerator_h

#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  /**
   * \brief Generates unique identifier strings.
   *
   * Produces unique IDs based on the UUID specification (RFC 4122) using a
   * random generator. The generator is thread-safe.
   *
   * An optional prefix can be specified for human-readable context (e.g. to
   * indicate the purpose of the UID), but it is not required for uniqueness.
   *
   * \sa Identifiable
   * \ingroup Core
   */
  class MITKCORE_EXPORT UIDGenerator
  {
  public:
    /**
     * \brief Construct a UID generator with an optional prefix.
     * \param[in] prefix A string prepended to every generated UID. May be empty.
     */
    explicit UIDGenerator(const char * prefix = "");

    /**
     * \brief Generate and return a new unique ID string.
     *
     * Each call produces a different unique ID. The format is the optional
     * prefix followed by a random UUID.
     *
     * \return A unique ID string.
     */
    std::string GetUID();

  private:
    std::string m_Prefix; ///< Optional prefix prepended to each generated UID.
  };
} // namespace mitk

#endif
