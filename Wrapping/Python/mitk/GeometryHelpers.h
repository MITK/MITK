/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef GeometryHelpers_h
#define GeometryHelpers_h

#include <mitkBaseGeometry.h>
#include <mitkTimeGeometry.h>

#include <pybind11/numpy.h>

#include <array>
#include <tuple>

namespace py = pybind11;

// ---------------------------------------------------------------------------
// Shared geometry accessors for Image and MultiLabelSegmentation bindings
// ---------------------------------------------------------------------------

/// @brief Returns the mutable BaseGeometry for the given time step.
/// @pre tg != nullptr
/// @pre timeStep is a valid time step of tg
mitk::BaseGeometry::Pointer GetGeometryForTimeStep(mitk::TimeGeometry* tg, mitk::TimeStepType timeStep);

/// @brief Returns the const BaseGeometry for the given time step.
/// @pre tg != nullptr
/// @pre timeStep is a valid time step of tg
mitk::BaseGeometry::ConstPointer GetConstGeometryForTimeStep(const mitk::TimeGeometry* tg, mitk::TimeStepType timeStep);

/// @brief Returns the voxel spacing (x, y, z) at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
std::tuple<double, double, double> GetSpacing(const mitk::TimeGeometry* tg, mitk::TimeStepType t);

/// @brief Sets the voxel spacing at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
/// @pre all elements of spacing must be positive
void SetSpacing(mitk::TimeGeometry* tg, const std::array<double, 3>& spacing, mitk::TimeStepType t);

/// @brief Returns the origin (x, y, z) at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
std::tuple<double, double, double> GetOrigin(const mitk::TimeGeometry* tg, mitk::TimeStepType t);

/// @brief Sets the origin at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
void SetOrigin(mitk::TimeGeometry* tg, const std::array<double, 3>& origin, mitk::TimeStepType t);

/// @brief Extracts the direction cosine matrix (3x3) from a BaseGeometry.
/// @pre geom must be a valid, initialized BaseGeometry with a non-null index-to-world transform
py::array_t<double> ExtractDirection(const mitk::BaseGeometry& geom);

/// @brief Returns the direction cosine matrix (3x3) at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
py::array_t<double> GetDirection(const mitk::TimeGeometry* tg, mitk::TimeStepType t);

/// @brief Sets the direction cosine matrix at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
/// @pre direction must be a 3x3 matrix
void SetDirection(mitk::TimeGeometry* tg, py::array_t<double, py::array::c_style | py::array::forcecast> direction, mitk::TimeStepType t);

/// @brief Returns the direction cosines as a flat 9-element array at the given time step.
/// @pre tg != nullptr
/// @pre t is a valid time step of tg
std::array<double, 9> GetDirectionCosines(const mitk::TimeGeometry* tg, mitk::TimeStepType t);

/**
 * @brief Binds geometry accessors (per-time-step methods and convenience properties)
 *        onto an existing pybind11 class that has a GetTimeGeometry() method.
 *
 * @tparam PyClass   pybind11::class_<...> type
 * @tparam CppClass  The C++ class being bound (must provide GetTimeGeometry() and GetDimension())
 * @tparam BindNdim  If false, skips the ndim property (for classes that bind it themselves)
 */
template <typename PyClass, typename CppClass, bool BindNdim = true>
void BindGeometryAccessors(PyClass& cls)
{
  // Per-time-step accessors
  cls.def("get_spacing",
    [](const CppClass& obj, mitk::TimeStepType t) { return GetSpacing(obj.GetTimeGeometry(), t); },
    py::arg("time_step") = 0,
    R"(Return the voxel spacing at the given time step.

Args:
    time_step: Time-step index (default 0).

Returns:
    A 3-tuple ``(sx, sy, sz)`` in world units.
)");
  cls.def("set_spacing",
    [](CppClass& obj, const std::array<double, 3>& s, mitk::TimeStepType t) { SetSpacing(obj.GetTimeGeometry(), s, t); },
    py::arg("spacing"), py::arg("time_step") = 0,
    R"(Set the voxel spacing at the given time step.

Args:
    spacing: 3-element sequence ``(sx, sy, sz)`` in world units.
    time_step: Time-step index (default 0).
)");
  cls.def("get_origin",
    [](const CppClass& obj, mitk::TimeStepType t) { return GetOrigin(obj.GetTimeGeometry(), t); },
    py::arg("time_step") = 0,
    R"(Return the origin at the given time step.

Args:
    time_step: Time-step index (default 0).

Returns:
    A 3-tuple ``(ox, oy, oz)`` in world coordinates.
)");
  cls.def("set_origin",
    [](CppClass& obj, const std::array<double, 3>& o, mitk::TimeStepType t) { SetOrigin(obj.GetTimeGeometry(), o, t); },
    py::arg("origin"), py::arg("time_step") = 0,
    R"(Set the origin at the given time step.

Args:
    origin: 3-element sequence ``(ox, oy, oz)`` in world coordinates.
    time_step: Time-step index (default 0).
)");
  cls.def("get_direction",
    [](const CppClass& obj, mitk::TimeStepType t) { return GetDirection(obj.GetTimeGeometry(), t); },
    py::arg("time_step") = 0,
    R"(Return the direction cosine matrix at the given time step.

Args:
    time_step: Time-step index (default 0).

Returns:
    A 3x3 NumPy array of doubles.
)");
  cls.def("set_direction",
    [](CppClass& obj, py::array_t<double, py::array::c_style | py::array::forcecast> d, mitk::TimeStepType t) {
      SetDirection(obj.GetTimeGeometry(), d, t);
    },
    py::arg("direction"), py::arg("time_step") = 0,
    R"(Set the direction cosine matrix at the given time step.

Args:
    direction: 3x3 array-like (NumPy array, nested list, etc.).
    time_step: Time-step index (default 0).
)");
  cls.def("get_direction_cosines",
    [](const CppClass& obj, mitk::TimeStepType t) { return GetDirectionCosines(obj.GetTimeGeometry(), t); },
    py::arg("time_step") = 0,
    R"(Return the direction cosines as a flat 9-element tuple.

Args:
    time_step: Time-step index (default 0).

Returns:
    A 9-element sequence in row-major order.
)");
  cls.def("get_geometry",
    [](CppClass& obj, mitk::TimeStepType t) { return GetGeometryForTimeStep(obj.GetTimeGeometry(), t); },
    py::arg("time_step") = 0,
    R"(Return the full :py:class:`BaseGeometry` for the given time step.

Args:
    time_step: Time-step index (default 0).

Returns:
    The spatial geometry at the requested time step.
)");

  cls.def_property("spacing",
    [](const CppClass& obj) { return GetSpacing(obj.GetTimeGeometry(), 0); },
    [](CppClass& obj, const std::array<double, 3>& s) { SetSpacing(obj.GetTimeGeometry(), s, 0); },
    R"(Voxel spacing at time step 0 as a 3-tuple ``(sx, sy, sz)``.)");
  cls.def_property("origin",
    [](const CppClass& obj) { return GetOrigin(obj.GetTimeGeometry(), 0); },
    [](CppClass& obj, const std::array<double, 3>& o) { SetOrigin(obj.GetTimeGeometry(), o, 0); },
    R"(Origin at time step 0 as a 3-tuple ``(ox, oy, oz)``.)");
  cls.def_property("direction",
    [](const CppClass& obj) { return GetDirection(obj.GetTimeGeometry(), 0); },
    [](CppClass& obj, py::array_t<double, py::array::c_style | py::array::forcecast> d) { SetDirection(obj.GetTimeGeometry(), d, 0); },
    R"(Direction cosine matrix at time step 0 as a 3x3 NumPy array.)");
  cls.def_property_readonly("direction_cosines",
    [](const CppClass& obj) { return GetDirectionCosines(obj.GetTimeGeometry(), 0); },
    "Direction cosines at time step 0 as a flat 9-element tuple.");
  if constexpr (BindNdim)
  {
    cls.def_property_readonly("ndim",
      [](const CppClass& obj) { return obj.GetDimension(); },
      "Number of dimensions of the object.");
  }
  cls.def_property_readonly("time_steps",
    [](const CppClass& obj) { return obj.GetTimeGeometry()->CountTimeSteps(); },
    "Number of time steps.");
  cls.def_property_readonly("time_geometry",
    [](CppClass& obj) { return mitk::TimeGeometry::Pointer(obj.GetTimeGeometry()); },
    "The full :py:class:`TimeGeometry` of this object.");
}

#endif
