/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/pybind11.h>

namespace py = pybind11;

void InitException(py::module_&);
void InitPoints(py::module_&);
void InitVectors(py::module_&);
void InitGeometries(py::module_&);
void InitPixelType(py::module_&);
void InitImage(py::module_&);

PYBIND11_MODULE(mitk, m)
{
  m.doc() = "MITK Python bindings";

  py::module_ os = py::module_::import("os");
  os.attr("environ")["PYMITK"] = "1";

  InitException(m);
  InitPoints(m);
  InitVectors(m);
  InitGeometries(m);
  InitPixelType(m);
  InitImage(m);
}
