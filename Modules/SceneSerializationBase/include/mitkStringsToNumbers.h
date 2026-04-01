/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStringsToNumbers_h
#define mitkStringsToNumbers_h

#include <mitkLexicalCast.h>

namespace mitk
{
  /**
   * \brief Converts an array of string representations to an array of numeric values.
   *
   * Uses boost::lexical_cast to convert \p count elements from the \p strings container
   * into the \p numbers container. Both containers must support operator[] access.
   * This is commonly used to parse serialized numeric values (e.g., from XML attributes)
   * back into Point3D, Vector3D, or similar fixed-size numeric containers.
   *
   * \tparam NUMBER_TYPE   The target numeric type (e.g., double, float, int).
   * \tparam STRING_ARRAY  A container type supporting operator[] that holds string values.
   * \tparam DOUBLE_ARRAY  A container type supporting operator[] that holds numeric values.
   *
   * \param[in]  count    The number of elements to convert from \p strings to \p numbers.
   * \param[in]  strings  A container holding at least \p count string values starting at index 0.
   * \param[out] numbers  A container with space for at least \p count numeric values starting at index 0.
   *
   * \warning This method has absolutely no means of verifying that your containers
   *          are big enough. It is the caller's responsibility to make sure that
   *          both the input and the output container can be addressed via [0] through [count-1].
   *
   * \throw boost::bad_lexical_cast Propagated when an unparsable string is encountered.
   *
   * \code
   * std::vector<std::string> serialized_double_values = ... read from some file ...
   * mitk::Point3D point;
   * try
   * {
   *   mitk::StringsToNumbers<double>(3, serialized_double_values, point);
   * }
   * catch (boost::bad_lexical_cast& e)
   * {
   *   MITK_ERROR << "Bad cast from string to double: " << e.what();
   * }
   * \endcode
   *
   * \sa mitk::BasePropertySerializer
   */
  template <typename NUMBER_TYPE, typename STRING_ARRAY, typename DOUBLE_ARRAY>
  void StringsToNumbers(unsigned int count, const STRING_ARRAY &strings, DOUBLE_ARRAY &numbers)
  {
    for (unsigned int i = 0; i < count; ++i)
    {
      numbers[i] = boost::lexical_cast<NUMBER_TYPE>(strings[i]);
    }
  }
}

#endif
