/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "SmartPointer.h"
#include "TemporoSpatialStringSerialization.h"
#include <mitkTemporoSpatialStringProperty.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_TemporoSpatialStringProperty(py::module_& m)
{
  // TemporoSpatialStringProperty binding
  py::class_<mitk::TemporoSpatialStringProperty, mitk::BaseProperty, mitk::TemporoSpatialStringProperty::Pointer>(m, "TemporoSpatialStringProperty")
    .def(py::init([](const std::string& value) { return mitk::TemporoSpatialStringProperty::New(value); }),
         py::arg("value") = "")
    
    // Uniform value access
    .def_property_readonly("value", 
                           [](const mitk::TemporoSpatialStringProperty& p) { return p.GetValue(); })
    
    // Time- and slice-specific access
    .def("get_value",
         [](const mitk::TemporoSpatialStringProperty& p, 
            std::optional<mitk::TimeStepType> time_step, 
            std::optional<mitk::TemporoSpatialStringProperty::IndexValueType> z_slice) 
         {
           if (time_step && z_slice)
             return p.GetValue(*time_step, *z_slice);
           else if (time_step)
             return p.GetValueByTimeStep(*time_step);
           else if (z_slice)
             return p.GetValueBySlice(*z_slice);
           else
             return p.GetValue();
         },
         py::arg("time_step") = py::none(),
         py::arg("z_slice") = py::none(),
         "Get value at specific time step and/or slice. If both omitted, returns uniform value.")
    
    // Setting values with flexible dispatch
    .def("set_value",
         [](mitk::TemporoSpatialStringProperty& p, const std::string& value,
            std::optional<mitk::TimeStepType> time_step,
            std::optional<mitk::TemporoSpatialStringProperty::IndexValueType> z_slice)
         {
           if (time_step && z_slice)
           {
             // Specific time step and slice
             p.SetValue(*time_step, *z_slice, value);
           }
           else if (time_step)
           {
             // All slices in specific time step
             auto ts = *time_step;
             auto slices = p.GetAvailableSlices(ts);
             if (slices.empty())
             {
               // No slices exist at this time step yet -- create slice 0
               p.SetValue(ts, 0, value);
             }
             else
             {
               for (auto s : slices)
                 p.SetValue(ts, s, value);
             }
           }
           else if (z_slice)
           {
             // All time steps for specific slice
             auto slice = *z_slice;
             auto time_steps = p.GetAvailableTimeSteps();
             if (time_steps.empty())
             {
               // No time steps exist yet -- create time step 0
               p.SetValue(0, slice, value);
             }
             else
             {
               for (auto ts : time_steps)
                 p.SetValue(ts, slice, value);
             }
           }
           else
           {
             // Set uniform value (clears all existing values)
             p.SetValue(value);
           }
         },
         py::arg("value"),
         py::arg("time_step") = py::none(),
         py::arg("z_slice") = py::none(),
         "Set value at specific time step and/or slice. If both omitted, sets uniform value (clears all).")
    
    // Query available indices
    .def("get_available_time_steps",
         [](const mitk::TemporoSpatialStringProperty& p) 
         { return p.GetAvailableTimeSteps(); },
         "Get all time steps that have stored values.")
    
    .def("get_available_time_steps",
         [](const mitk::TemporoSpatialStringProperty& p, 
            mitk::TemporoSpatialStringProperty::IndexValueType slice) 
         { return p.GetAvailableTimeSteps(slice); },
         py::arg("z_slice"),
         "Get time steps that have values for the specified slice.")
    
    .def("get_available_slices",
         [](const mitk::TemporoSpatialStringProperty& p) 
         { return p.GetAvailableSlices(); },
         "Get all unique slice indices across all time steps.")
    
    .def("get_available_slices",
         [](const mitk::TemporoSpatialStringProperty& p, 
            mitk::TimeStepType time_step) 
         { return p.GetAvailableSlices(time_step); },
         py::arg("time_step"),
         "Get slice indices for the specified time step.")
    
    // Check if uniform
    .def_property_readonly("is_uniform", 
                           &mitk::TemporoSpatialStringProperty::IsUniform,
                           "True if all stored values are identical.")
    
    // Check existence
    .def("has_value",
         [](const mitk::TemporoSpatialStringProperty& p,
            std::optional<mitk::TimeStepType> time_step,
            std::optional<mitk::TemporoSpatialStringProperty::IndexValueType> z_slice)
         {
           if (time_step && z_slice)
             return p.HasValue(*time_step, *z_slice);
           else if (time_step)
             return p.HasValueByTimeStep(*time_step);
           else if (z_slice)
             return p.HasValueBySlice(*z_slice);
           else
             return p.HasValue();
         },
         py::arg("time_step") = py::none(),
         py::arg("z_slice") = py::none(),
         "Check if a value exists at specific time step and/or slice.")
    
    // Inherited to_dict works via the serialization helper
    // No need to override - the base class to_dict dispatches correctly
    ;
}