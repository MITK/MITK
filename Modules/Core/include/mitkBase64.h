/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBase64_h
#define mitkBase64_h

#include <MitkCoreExports.h>

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace mitk::Base64
{
  /**
   * \brief Encode raw bytes as a canonical Base64 (RFC 4648) string.
   *
   * Uses the standard alphabet (\c '+' and \c '/' as the 62nd and 63rd
   * characters), pads the output with \c '=', and inserts no line breaks.
   */
  MITKCORE_EXPORT std::string Encode(std::span<const std::byte> data);

  /**
   * \brief Decode a canonical Base64 (RFC 4648) string into the original bytes.
   *
   * Decoding stops at the first character outside the Base64 alphabet
   * (including padding), so any trailing content is ignored.
   */
  MITKCORE_EXPORT std::vector<std::byte> Decode(const std::string& encoded);
}

#endif
