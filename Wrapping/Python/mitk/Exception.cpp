/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/pybind11.h>
#include <mitkException.h>

namespace py = pybind11;
using namespace mitk;

void InitException(py::module_& m)
{
  py::register_exception<mitk::Exception>(m, "Exception");
}
