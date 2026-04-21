/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * @file TemporoSpatialStringSerialization.h
 * @brief Header-only helpers for TemporoSpatialStringProperty serialization
 * 
 * This header provides serialization functions for TemporoSpatialStringProperty
 * that are used by Property.cpp and PropertyList.cpp without modifying
 * PropertyConversionUtils.h.
 */

#pragma once

#include <mitkTemporoSpatialStringProperty.h>
#include <mitkBaseProperty.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace mitk::python
{
    // Returns true if prop was TemporoSpatialStringProperty and was serialized into result.
    inline bool tryTemporoSpatialStringToDict(const mitk::BaseProperty& prop, py::dict& result)
    {
        const auto* tsProp = dynamic_cast<const mitk::TemporoSpatialStringProperty*>(&prop);
        if (!tsProp) return false;
        
        result["type"] = "TemporoSpatialStringProperty";
        py::list entries;
        
        for (auto ts : tsProp->GetAvailableTimeSteps())
            for (auto slice : tsProp->GetAvailableSlices(ts))
                entries.append(py::make_tuple(ts, slice, tsProp->GetValue(ts, slice)));
        
        result["entries"] = entries;
        return true;
    }

    // Returns non-null if type == "TemporoSpatialStringProperty".
    inline mitk::BaseProperty::Pointer tryDictToTemporoSpatialString(const py::dict& d)
    {
        if (d["type"].cast<std::string>() != "TemporoSpatialStringProperty")
            return nullptr;
        
        auto prop = mitk::TemporoSpatialStringProperty::New();
        
        for (auto entry : d["entries"].cast<py::list>())
        {
            auto t = entry.cast<py::tuple>();
            prop->SetValue(t[0].cast<mitk::TimeStepType>(),
                         t[1].cast<mitk::TemporoSpatialStringProperty::IndexValueType>(),
                         t[2].cast<std::string>());
        }
        
        return prop.GetPointer();
    }

} // namespace mitk::python