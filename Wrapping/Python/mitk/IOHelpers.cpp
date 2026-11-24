/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "IOHelpers.h"

#include <limits>
#include <string>

mitk::IFileWriter::Options ToWriterOptions(const std::optional<py::dict>& writerOptions)
{
  mitk::IFileWriter::Options result;
  if (!writerOptions)
    return result;

  for (const auto& [key, value] : *writerOptions)
  {
    if (!py::isinstance<py::str>(key))
      throw py::type_error("Writer option names must be str, got " + py::repr(key).cast<std::string>());

    const auto name = key.cast<std::string>();

    // bool before int: Python's bool is a subclass of int.
    if (py::isinstance<py::bool_>(value))
      result[name] = value.cast<bool>();
    else if (py::isinstance<py::int_>(value))
    {
      // The readers and writers take int options as C++ int; a Python int is unbounded.
      try
      {
        result[name] = value.cast<int>();
      }
      catch (const py::cast_error&)
      {
        const auto message = "Writer option \"" + name + "\" must fit a C++ int [" +
                             std::to_string(std::numeric_limits<int>::min()) + ", " +
                             std::to_string(std::numeric_limits<int>::max()) + "], got " +
                             py::repr(value).cast<std::string>();
        PyErr_SetString(PyExc_OverflowError, message.c_str());
        throw py::error_already_set();
      }
    }
    else if (py::isinstance<py::float_>(value))
      result[name] = value.cast<double>();
    else if (py::isinstance<py::str>(value))
      result[name] = value.cast<std::string>();
    else
      throw py::type_error("Writer option \"" + name + "\" must be bool, int, float or str, got " +
                           py::str(py::type::of(value).attr("__name__")).cast<std::string>());
  }

  return result;
}
