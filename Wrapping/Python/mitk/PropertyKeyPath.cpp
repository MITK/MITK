/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "SmartPointer.h"
#include <mitkPropertyKeyPath.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <regex>

namespace py = pybind11;

void init_PropertyKeyPath(py::module_& m)
{
  // PropertyKeyPath binding
  py::class_<mitk::PropertyKeyPath>(m, "PropertyKeyPath")
    .def(py::init<>(), "Create an empty PropertyKeyPath.")
    
    // Construction from string
    .def_static("from_string", 
               [](const std::string& propertyName) 
               { return mitk::PropertyNameToPropertyKeyPath(propertyName); },
               py::arg("property_name"),
               "Parse a property name string into a PropertyKeyPath.")
    
    // Construction from single element
    .def(py::init([](const std::string& element) 
                  { return mitk::PropertyKeyPath({element}); }),
         py::arg("element"),
         "Create a PropertyKeyPath with a single element.")
    
    // Properties
    .def_property_readonly("is_explicit", &mitk::PropertyKeyPath::IsExplicit,
                           "True if the path has no wildcards.")
    .def_property_readonly("is_empty", &mitk::PropertyKeyPath::IsEmpty,
                           "True if the path has no nodes.")
    .def("__len__", &mitk::PropertyKeyPath::GetSize,
         "Return the number of nodes in the path.")
    
    // String conversion
    .def("__str__", 
         [](const mitk::PropertyKeyPath& self) 
         { return mitk::PropertyKeyPathToPropertyName(self); },
         "Return the property name string representation.")
    .def("__repr__", 
         [](const mitk::PropertyKeyPath& self) 
         { return "<mitk.PropertyKeyPath: " + mitk::PropertyKeyPathToPropertyName(self) + ">"; })
    
    // Equality comparison
    .def("__eq__",
         [](const mitk::PropertyKeyPath& self, const py::object& other)
         {
           // If other is not a PropertyKeyPath, they're not equal
           if (!py::isinstance<mitk::PropertyKeyPath>(other))
             return false;
           
           // Otherwise use normal equality
           return self == other.cast<mitk::PropertyKeyPath>();
         },
         py::arg("other"),
         "Check if two paths are structurally identical.")
    .def("__ne__", 
         [](const mitk::PropertyKeyPath& self, const py::object& other)
         {
           // If other is not a PropertyKeyPath, they're not equal
           if (!py::isinstance<mitk::PropertyKeyPath>(other))
             return true;
           
           // Otherwise use normal equality
           return !(self == other.cast<mitk::PropertyKeyPath>());
         },
         py::arg("other"))
    
    // Path concatenation with / operator (like pathlib.Path)
    .def("__truediv__",
         [](const mitk::PropertyKeyPath& self, const std::string& element)
         {
           const auto& nodes = self.GetNodes();

           // [*] — convert last Element node to AnySelection (replace, not append)
           if (element == "[*]")
           {
             if (self.IsEmpty())
               throw py::value_error("Cannot add wildcard selection to empty path");
             mitk::PropertyKeyPath result;
             for (std::size_t i = 0; i + 1 < nodes.size(); ++i)
               result.AddNode(nodes[i]);
             auto lastNode = nodes.back();
             lastNode.type = mitk::PropertyKeyPath::NodeInfo::NodeType::AnySelection;
             result.AddNode(lastNode);
             return result;
           }

           // [n] — convert last Element node to ElementSelection (replace, not append)
           if (element.size() > 2 && element.front() == '[' && element.back() == ']')
           {
             try
             {
               const std::size_t index = std::stoul(element.substr(1, element.size() - 2));
               if (self.IsEmpty())
                 throw py::value_error("Cannot add indexed selection to empty path");
               mitk::PropertyKeyPath result;
               for (std::size_t i = 0; i + 1 < nodes.size(); ++i)
                 result.AddNode(nodes[i]);
               auto lastNode = nodes.back();
               lastNode.type = mitk::PropertyKeyPath::NodeInfo::NodeType::ElementSelection;
               lastNode.selection = index;
               result.AddNode(lastNode);
               return result;
             }
             catch (const std::invalid_argument&)
             {
               throw py::value_error("Invalid selection syntax: " + element);
             }
           }

           // * — append AnyElement wildcard
           if (element == "*")
           {
             mitk::PropertyKeyPath result = self;
             result.AddAnyElement();
             return result;
           }

           // Regular named element — append
           mitk::PropertyKeyPath result = self;
           result.AddElement(element);
           return result;
         },
         py::arg("element"),
         "Append an element or selection to the path using / operator.")
    
    .def("__truediv__",
         [](const mitk::PropertyKeyPath& self, const mitk::PropertyKeyPath& other)
         {
           mitk::PropertyKeyPath result = self;
           for (const auto& node : other.GetNodes())
             result.AddNode(node);
           return result;
         },
         py::arg("other"),
         "Append another PropertyKeyPath to this one.")
    
    // Index-based access for selections (syntactic sugar)
    .def("__getitem__",
         [](const mitk::PropertyKeyPath& self, std::size_t index)
         {
           if (self.IsEmpty())
             throw py::index_error("Cannot add selection to empty path");
           const auto& nodes = self.GetNodes();
           mitk::PropertyKeyPath result;
           for (std::size_t i = 0; i + 1 < nodes.size(); ++i)
             result.AddNode(nodes[i]);
           auto lastNode = nodes.back();
           lastNode.type = mitk::PropertyKeyPath::NodeInfo::NodeType::ElementSelection;
           lastNode.selection = index;
           result.AddNode(lastNode);
           return result;
         },
         py::arg("index"),
         "Convert last element to indexed selection (e.g., path[5] -> path.element.[5]).")

    .def("__getitem__",
         [](const mitk::PropertyKeyPath& self, const std::string& wildcard)
         {
           if (wildcard != "*")
             throw py::value_error("Only '*' wildcard is supported for string indexing");
           if (self.IsEmpty())
             throw py::index_error("Cannot add wildcard selection to empty path");
           const auto& nodes = self.GetNodes();
           mitk::PropertyKeyPath result;
           for (std::size_t i = 0; i + 1 < nodes.size(); ++i)
             result.AddNode(nodes[i]);
           auto lastNode = nodes.back();
           lastNode.type = mitk::PropertyKeyPath::NodeInfo::NodeType::AnySelection;
           result.AddNode(lastNode);
           return result;
         },
         py::arg("wildcard"),
         "Convert last element to wildcard selection (e.g., path['*'] -> path.element.[*]).")
    
    // Copy protocol
    .def("__copy__", 
         [](const mitk::PropertyKeyPath& self) 
         { return mitk::PropertyKeyPath(self); })
    .def("__deepcopy__", 
         [](const mitk::PropertyKeyPath& self, py::dict /*memo*/) 
         { return mitk::PropertyKeyPath(self); },
         py::arg("memo"))

    // Node access
    .def("get_nodes", 
         [](const mitk::PropertyKeyPath& self) 
         {
           py::list nodes;
           for (const auto& node : self.GetNodes())
           {
             py::dict nodeDict;
             nodeDict["type"] = static_cast<int>(node.type);
             nodeDict["name"] = node.name;
             nodeDict["selection"] = node.selection;
             nodes.append(nodeDict);
           }
           return nodes;
         },
         "Get detailed information about all nodes in the path.")
    ;

  // Module-level function for finding indexed properties
  m.def("find_indexed_properties",
        [](py::object provider, const mitk::PropertyKeyPath& path) -> py::dict
        {
          // Precondition: exactly one AnySelection node
          int anyCount = 0;
          size_t wildcardNodeIndex = 0;
          for (size_t i = 0; i < path.GetNodes().size(); ++i) {
            if (path.GetNodes()[i].type == mitk::PropertyKeyPath::NodeInfo::NodeType::AnySelection) {
              ++anyCount;
              wildcardNodeIndex = i;
            }
          }
          
          if (anyCount != 1)
            throw py::value_error(
                "find_indexed_properties requires exactly one [*] wildcard in the path");

          // Duck-typed key retrieval
          auto keys = provider.attr("property_keys").cast<std::vector<std::string>>();

          // Build the regex pattern node by node. The wildcard node carries
          // BOTH a name and the [*] selection in MITK's path model -- e.g.
          // AnySelection("seq") serializes as "seq.[*]".
          const auto& nodeVec = path.GetNodes();
          std::string regex_pattern;

          // Nodes before the wildcard
          for (std::size_t i = 0; i < wildcardNodeIndex; ++i) {
            if (!regex_pattern.empty()) regex_pattern += "\\.";
            regex_pattern += nodeVec[i].name;
          }

          // The wildcard node itself: name + ".[(\\d+)]"
          if (!regex_pattern.empty()) regex_pattern += "\\.";
          regex_pattern += nodeVec[wildcardNodeIndex].name + "\\.\\[(\\d+)\\]";

          // Nodes after the wildcard
          for (std::size_t i = wildcardNodeIndex + 1; i < nodeVec.size(); ++i) {
            regex_pattern += "\\." + nodeVec[i].name;
          }

          std::regex re(regex_pattern);

          py::dict result;
          std::smatch match;
          for (const auto& key : keys) {
            if (std::regex_match(key, match, re)) {
              if (match.size() > 1) {
                try {
                  auto idx = static_cast<std::size_t>(std::stoul(match[1].str()));
                  result[py::int_(idx)] = py::str(key);
                } catch (...) { /* skip non-numeric captures */ }
              }
            }
          }
          return result;
        },
        py::arg("provider"), py::arg("path"),
        "Find indexed property names matching a path with one [*] wildcard.");
}