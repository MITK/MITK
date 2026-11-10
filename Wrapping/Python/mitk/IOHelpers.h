/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef IOHelpers_h
#define IOHelpers_h

#include <mitkIFileWriter.h>

#include <pybind11/pybind11.h>

#include <optional>

namespace py = pybind11;

/**
 * Converts the writer options of a Python save call: names must be str, values bool, int, float
 * or str, the value types MitkFileConverter accepts as well. None means no options.
 *
 * \throw py::type_error naming the option for any other name or value type.
 * \pre The caller holds the GIL.
 */
mitk::IFileWriter::Options ToWriterOptions(const std::optional<py::dict>& writerOptions);

#endif
