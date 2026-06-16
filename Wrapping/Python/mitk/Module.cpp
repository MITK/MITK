/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/pybind11.h>

namespace py = pybind11;

void InitCppMicroServices(py::module_&);
void InitException(py::module_&);
void InitPoints(py::module_&);
void InitVectors(py::module_&);
void InitGeometries(py::module_&);
void InitColor(py::module_&);
void InitPixelType(py::module_&);
void InitImage(py::module_&);
void InitIOUtil(py::module_&);
void InitProperty(py::module_&);
void InitPropertyList(py::module_&);
void InitTemporoSpatialStringProperty(py::module_&);
void InitPropertyKeyPath(py::module_&);
void InitDICOMTagPath(py::module_&);
void InitMultiLabelSegmentation(py::module_&);
void InitRelations(py::module_&);
void InitDicom(py::module_&);

PYBIND11_MODULE(mitk, m)
{
  m.doc() = R"(Python bindings for the Medical Imaging Interaction Toolkit (MITK).

The ``mitk`` package exposes the data structures and core utilities of MITK
to Python. The central type is :py:class:`Image`, with full NumPy interop,
file I/O via :py:class:`IOUtil`, geometry classes (:py:class:`BaseGeometry`,
:py:class:`TimeGeometry`, ...), points and vectors, typed properties, and
:py:class:`MultiLabelSegmentation` for label-set data.

Full user documentation, including a Getting Started guide and an
auto-generated API reference, lives at https://mitk-python.readthedocs.io/en/latest/.

The wider MITK C++ stack is documented at https://docs.mitk.org/latest/.
)";

  py::module_ os = py::module_::import("os");
  os.attr("environ")["PYMITK"] = "1";

  InitCppMicroServices(m);
  InitException(m);
  InitPoints(m);
  InitVectors(m);
  InitGeometries(m);
  InitColor(m);
  InitPixelType(m);
  InitImage(m);
  InitIOUtil(m);
  InitProperty(m);
  InitPropertyList(m);
  InitTemporoSpatialStringProperty(m);
  InitPropertyKeyPath(m);
  InitDICOMTagPath(m);
  InitMultiLabelSegmentation(m);
  // mitk.relations and mitk.dicom are initialized after the data classes
  // they reference so the type registrations they depend on exist.
  InitRelations(m);
  InitDicom(m);
}
