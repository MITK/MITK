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
  //! Convert an array of strings to an array of numbers via boost::lexical_cast.
  //!
  //! Call mitk::StringsToNumbers for all count elements of something that can be accessed
  //! via operator[], e.g. to fill a Point3D / Vector3D.
  //!
  //! \param count the number of elements to convert from "strings" to "numbers"
  //! \param numbers a container for at least "count" numeric values with indices starting from 0
  //! \param strings a container for at least three string values with indices starting from 0
  //!
  //! \warning This method has absolutely no means of verifying that your containers
  //!          are big enough. It is the caller's responsibility to make sure that
  //!          both the input and the output container can be addressed via [0], [1], [2].
  //!
  //! \exception propagates boost::bad_lexical_cast exception when unparsable strings are encountered
  //!
  //! \code
  //! std::vector<std::string> serialized_double_values = ... read from some file ...
  //! mitk::Point3D point;
  //! try
  //! {
  //!   mitk::StringToDouble(3, serialized_double_values, point);
  //! }
  //! catch (boost::bad_lexical_cast& e)
  //! {
  //!   MITK_ERROR << "Bad cast from string to double: " << e.what();
  //! }
  //! \endcode
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
