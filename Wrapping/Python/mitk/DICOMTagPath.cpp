/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "SmartPointer.h"
#include <mitkDICOMTagPath.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void InitDICOMTagPath(py::module_& m)
{
  py::class_<mitk::DICOMTag>(m, "DICOMTag",
    R"(A single DICOM tag identified by ``(group, element)``.

DICOM tags are hexadecimal pairs identifying a data element in the DICOM
standard, e.g. ``(0x0010, 0x0010)`` is "Patient's Name".
)")
    .def(py::init<int, int>(),
         py::arg("group"), py::arg("element"),
         R"(Construct a DICOM tag.

Args:
    group: Tag group number (typically hexadecimal, e.g. ``0x0010``).
    element: Tag element number (typically hexadecimal, e.g. ``0x0010``).
)")
    .def("__eq__", &mitk::DICOMTag::operator==,
         py::arg("other"),
         "Check if two DICOM tags are equal.")
    .def("__repr__",
         [](const mitk::DICOMTag& self)
         {
           return "<mitk.DICOMTag: (" + 
                  std::to_string(self.GetGroup()) + ", " +
                  std::to_string(self.GetElement()) + ")>";
         })
    .def_property_readonly("group", &mitk::DICOMTag::GetGroup,
                           "The DICOM tag group number.")
    .def_property_readonly("element", &mitk::DICOMTag::GetElement,
                           "The DICOM tag element number.")
    ;

  py::class_<mitk::DICOMTagPath>(m, "DICOMTagPath",
    R"(Path through nested DICOM tags.

A ``DICOMTagPath`` is to :py:class:`DICOMTag` what :py:class:`PropertyKeyPath`
is to a property name: a structured, manipulable representation. Used to
address tags inside sequences or nested data sets.

Convert to/from MITK's flat property-name form via :py:meth:`from_string`,
:py:meth:`from_property_name`, ``str(path)``, and :py:meth:`to_property_name`.

Examples:
    >>> p = mitk.DICOMTagPath(0x0010, 0x0010)
    >>> p.to_property_name()
    'DICOM.0010.0010'
)")
    .def(py::init<>(), "Construct an empty path.")
    
    // Construction from DICOM tag
    .def(py::init([](int group, int element)
                  { return mitk::DICOMTagPath(mitk::DICOMTag(group, element)); }),
         py::arg("group"), py::arg("element"),
         "Create a DICOMTagPath from a DICOM tag (group, element).")
    
    // Construction from DICOMTag object
    .def(py::init([](const mitk::DICOMTag& tag)
                  { return mitk::DICOMTagPath(tag); }),
         py::arg("tag"),
         "Create a DICOMTagPath from a DICOMTag object.")
    
    // Parsing methods
    .def_static("from_string",
               [](const std::string& tagString)
               {
                 mitk::DICOMTagPath path;
                 path.FromStr(tagString);
                 return path;
               },
               py::arg("tag_string"),
               "Parse a DICOM tag path from string format like '(0010,0010)'.")
    
    .def_static("from_property_name",
               &mitk::PropertyNameToDICOMTagPath,
               py::arg("property_name"),
               "Parse a DICOM tag path from MITK property name format like 'DICOM.0010.0010'.")
    
    // Properties
    .def_property_readonly("is_explicit", &mitk::DICOMTagPath::IsExplicit,
                           "True if the path has no wildcards.")
    .def_property_readonly("is_empty", &mitk::DICOMTagPath::IsEmpty,
                           "True if the path has no nodes.")
    .def("__len__", &mitk::DICOMTagPath::Size,
         "Return the number of nodes in the path.")
    
    // String conversion
    .def("__str__", 
         [](const mitk::DICOMTagPath& self) 
         { return self.ToStr(); },
         "Return the DICOM tag path string representation.")
    .def("__repr__", 
         [](const mitk::DICOMTagPath& self) 
         { return "<mitk.DICOMTagPath: " + self.ToStr() + ">"; })
    
    // Property name conversion
    .def("to_property_name", 
         &mitk::DICOMTagPathToPropertyName,
         "Convert to MITK property name format (e.g., 'DICOM.0010.0010').")
    
    // Human-readable name
    .def("to_readable_name",
         [](const mitk::DICOMTagPath& self, bool include_tags = false)
         {
           return mitk::DICOMTagPathToReadableName(self, include_tags);
         },
         py::arg("include_tags") = false,
         "Convert to human-readable name using DICOM dictionary.")
    
    // Equality comparison
    .def("__eq__", &mitk::DICOMTagPath::operator==,
         py::arg("other"),
         "Check if two paths are structurally identical.")
    .def("__ne__", 
         [](const mitk::DICOMTagPath& self, const mitk::DICOMTagPath& other)
         { return !(self == other); },
         py::arg("other"))
    
    // Path concatenation with / operator (like pathlib.Path)
    .def("__truediv__",
         [](const mitk::DICOMTagPath& self, const mitk::DICOMTagPath& other)
         {
           return self + other;  // uses operator+
         },
         py::arg("other"),
         "Append another DICOMTagPath using / operator.")
    
    // Copy protocol
    .def("__copy__", 
         [](const mitk::DICOMTagPath& self) 
         { return mitk::DICOMTagPath(self); })
    .def("__deepcopy__", 
         [](const mitk::DICOMTagPath& self, py::dict /*memo*/) 
         { return mitk::DICOMTagPath(self); },
         py::arg("memo"))
    
    // Cross-type inequality
    .def("__ne__", 
         [](const mitk::DICOMTagPath& self, const py::object& other)
         {
           // If other is not a DICOMTagPath, they're not equal
           if (!py::isinstance<mitk::DICOMTagPath>(other))
             return true;
           
           // Otherwise use normal equality
           return !(self == other.cast<mitk::DICOMTagPath>());
         },
         py::arg("other"))
    
    // Node access
    .def("get_nodes", 
         [](const mitk::DICOMTagPath& self) 
         {
           py::list nodes;
           for (const auto& node : self.GetNodes())
           {
             py::dict nodeDict;
             nodeDict["type"] = static_cast<int>(node.type);
             nodeDict["tag"] = py::make_tuple(node.tag.GetGroup(), node.tag.GetElement());
             nodeDict["selection"] = node.selection;
             nodes.append(nodeDict);
           }
           return nodes;
         },
         "Get detailed information about all nodes in the path.")
    ;
}