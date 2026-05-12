/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/stl.h>

#include "SmartPointer.h"

#include <mitkGeometry3D.h>
#include <mitkPlaneGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkProportionalTimeGeometry.h>

namespace py = pybind11;
using namespace mitk;

void InitBaseGeometry(py::module_& m)
{
  py::class_<BaseGeometry, BaseGeometry::Pointer>(m, "BaseGeometry",
    R"(Spatial geometry of a 3D object.

A ``BaseGeometry`` ties the discrete index space (voxel indices) of an
image or data object to a continuous world coordinate system. It carries:

- An **origin**: world coordinates of the index ``(0, 0, 0)``.
- A **spacing**: world distance between adjacent indices along each axis.
- A **direction cosine matrix**: orientation of the index axes in world
  space.
- **Bounds**: the index-space extent of the data.

``BaseGeometry`` is abstract. The concrete subclasses bound here are
:py:class:`Geometry3D`, :py:class:`PlaneGeometry`, and
:py:class:`SlicedGeometry3D`.
)")
    .def_static("new", &BaseGeometry::New,
      "Construct a new identity-initialized geometry.")
    .def_property("image_geometry", &BaseGeometry::GetImageGeometry, &BaseGeometry::SetImageGeometry,
      R"(Whether this geometry is interpreted as an image geometry.

When True, world coordinates address voxel centers (corner-based addressing
shifted by half a voxel); when False, world coordinates address voxel
corners.
)")
    .def_property("origin", &BaseGeometry::GetOrigin, &BaseGeometry::SetOrigin,
      "World coordinates of the index ``(0, 0, 0)`` as a :py:class:`Point3D`.")
    .def_property("spacing", &BaseGeometry::GetSpacing, &BaseGeometry::SetSpacing,
      "Voxel spacing in world units as a :py:class:`Vector3D` ``(sx, sy, sz)``.")
    .def("enforce_spacing",
      [](BaseGeometry& g, const Vector3D& s) {
        g.SetSpacing(s, true);
      },
      py::arg("spacing"),
      R"(Set spacing and rescale the index-to-world transform accordingly.

Equivalent to ``SetSpacing(spacing, enforceSetSpacing=True)``: rather than
storing the spacing as an independent value, the transform is rebuilt so
that the requested spacing is preserved exactly when the matrix is
re-decomposed.

Args:
    spacing: New spacing as a :py:class:`Vector3D`.
)")
    .def_property("frame_of_reference_id", &BaseGeometry::GetFrameOfReferenceID, &BaseGeometry::SetFrameOfReferenceID,
      "DICOM Frame of Reference ID used to group geometries that share a coordinate system.")
    .def_property("bounds",
      [](const BaseGeometry& g) {
        const auto b = g.GetBounds();
        return std::vector<ScalarType>(b.begin(), b.end());
      },
      [](BaseGeometry& g, const std::vector<ScalarType>& v) {
        if (v.size() != 6)
          mitkThrow() << "Bounds must be of length 6";

        BaseGeometry::BoundsArrayType b;
        std::copy(v.begin(), v.end(), b.begin());

        g.SetBounds(b);
      },
      R"(Index-space bounding box as a 6-element list.

Layout: ``[xmin, xmax, ymin, ymax, zmin, zmax]``. Assigning a list of any
other length raises an exception.
)")
    .def_property_readonly("index_to_world_last_modified", &BaseGeometry::GetIndexToWorldTransformLastModified,
      "Modified-time of the index-to-world transform; useful for cache invalidation.")
    .def_property_readonly("modified", &BaseGeometry::Modified,
      "Modified-time of the geometry as a whole.")
    .def_property_readonly("center", &BaseGeometry::GetCenter,
      "World coordinates of the center of the bounding box as a :py:class:`Point3D`.")
    .def_property_readonly("diagonal_length", &BaseGeometry::GetDiagonalLength,
      "Length of the bounding-box diagonal in world units.")
    .def("initialize", &BaseGeometry::Initialize,
      "Reset to an identity geometry with empty bounds.")
    .def("initialize_geometry", &BaseGeometry::InitializeGeometry, py::arg("geometry"),
      R"(Copy origin, spacing, direction, and bounds from another geometry.

Args:
    geometry: Source geometry to copy from.
)")
    .def("set_identity", &BaseGeometry::SetIdentity,
      "Reset the index-to-world transform to the identity matrix.")
    .def("is_valid", &BaseGeometry::IsValid,
      "Return True if the geometry is fully initialized and self-consistent.")
    .def("get_axis_vector", &BaseGeometry::GetAxisVector, py::arg("direction"),
      R"(Return the world-space vector along the given index axis.

Args:
    direction: Index axis (0 = x, 1 = y, 2 = z).

Returns:
    The axis vector as a :py:class:`Vector3D`, of length ``extent_in_mm``.
)")
    .def("get_extent", &BaseGeometry::GetExtent, py::arg("direction"),
      R"(Return the index-space extent along the given axis.

Args:
    direction: Index axis (0 = x, 1 = y, 2 = z).

Returns:
    Extent in voxels.
)")
    .def("get_extent_in_mm", &BaseGeometry::GetExtentInMM, py::arg("direction"),
      R"(Return the world-space extent along the given axis.

Args:
    direction: Index axis (0 = x, 1 = y, 2 = z).

Returns:
    Extent in world units (typically millimeters).
)")
    .def("set_extent_in_mm", &BaseGeometry::SetExtentInMM, py::arg("direction"), py::arg("extent"),
      R"(Set the world-space extent along the given axis (preserves spacing).

Args:
    direction: Index axis (0 = x, 1 = y, 2 = z).
    extent: New extent in world units.
)")
    .def("get_corner_point", py::overload_cast<int>(&BaseGeometry::GetCornerPoint, py::const_), py::arg("id"),
      R"(Return the world coordinates of one of the eight corners.

Args:
    id: Corner index in ``[0, 7]``.
)")
    .def("is_inside", &BaseGeometry::IsInside, py::arg("point"),
      R"(Test whether a world-space point lies inside the geometry.

Args:
    point: Point in world coordinates as a :py:class:`Point3D`.

Returns:
    True if the point is inside the bounding box.
)")
    .def("is_index_inside", [](const BaseGeometry& g, const Point3D& p) { return g.IsIndexInside(p); }, py::arg("index"),
      R"(Test whether an index-space point lies inside the geometry.

Args:
    index: Point in index coordinates as a :py:class:`Point3D`.

Returns:
    True if the index is inside the bounding box.
)")
    .def("clamp_point", &BaseGeometry::ClampPoint, py::arg("point"),
      R"(Clamp a world-space point to the geometry's bounding box.

Args:
    point: Point in world coordinates.

Returns:
    The nearest point inside the bounding box.
)")
    .def("world_to_index", [](const BaseGeometry& g, const Point3D& w, Point3D& i) { return g.WorldToIndex(w, i); }, py::arg("world"), py::arg("index"),
      R"(Convert a world-coordinate point to an index-coordinate point.

Args:
    world: Input point in world coordinates.
    index: Output point in index coordinates (filled in by the call).
)")
    .def("world_to_index", [](const BaseGeometry& g, const Vector3D& w, Vector3D& i) { return g.WorldToIndex(w, i); }, py::arg("world"), py::arg("index"),
      R"(Convert a world-coordinate vector to an index-coordinate vector.

Args:
    world: Input vector in world coordinates.
    index: Output vector in index coordinates (filled in by the call).
)")
    .def("index_to_world", [](const BaseGeometry& g, const Point3D& i, Point3D& w) { return g.IndexToWorld(i, w); }, py::arg("index"), py::arg("world"),
      R"(Convert an index-coordinate point to a world-coordinate point.

Args:
    index: Input point in index coordinates.
    world: Output point in world coordinates (filled in by the call).
)")
    .def("index_to_world", [](const BaseGeometry& g, const Vector3D& i, Vector3D& w) { return g.IndexToWorld(i, w); }, py::arg("index"), py::arg("world"),
      R"(Convert an index-coordinate vector to a world-coordinate vector.

Args:
    index: Input vector in index coordinates.
    world: Output vector in world coordinates (filled in by the call).
)")
    .def("translate", &BaseGeometry::Translate, py::arg("vector"),
      R"(Translate the geometry in world space.

Args:
    vector: Translation vector as a :py:class:`Vector3D`.
)");
}

void InitGeometry3D(py::module_& m)
{
  py::class_<Geometry3D, BaseGeometry, Geometry3D::Pointer>(m, "Geometry3D",
    R"(Concrete 3D spatial geometry.

Inherits all methods of :py:class:`BaseGeometry`. The default subclass used
when MITK creates a fresh geometry for an :py:class:`Image`.
)")
    .def_static("new", py::overload_cast<>(&Geometry3D::New),
      "Construct a new identity-initialized 3D geometry.");
}

void InitPlaneGeometry(py::module_& m)
{
  py::class_<PlaneGeometry, BaseGeometry, PlaneGeometry::Pointer>(m, "PlaneGeometry",
    R"(Geometry of a 2D plane embedded in 3D world space.

Used for example to describe individual slices of a 3D image. Inherits from
:py:class:`BaseGeometry` and adds 2D index <-> world conversions plus a
``normal`` accessor.
)")
    .def_static("new", &PlaneGeometry::New,
      "Construct a new identity-initialized plane geometry.")
    .def_property_readonly("normal", &PlaneGeometry::GetNormal,
      "World-space normal of the plane as a :py:class:`Vector3D`.")
    .def("world_to_index", py::overload_cast<const Point2D&, Point2D&>(&PlaneGeometry::WorldToIndex, py::const_), py::arg("world"), py::arg("index"),
      "Convert a 2D world-coordinate point on the plane to a 2D index-coordinate point.")
    .def("world_to_index", py::overload_cast<const Vector2D&, Vector2D&>(&PlaneGeometry::WorldToIndex, py::const_), py::arg("world"), py::arg("index"),
      "Convert a 2D world-coordinate vector on the plane to a 2D index-coordinate vector.")
    .def("index_to_world", py::overload_cast<const Point2D&, Point2D&>(&PlaneGeometry::IndexToWorld, py::const_), py::arg("index"), py::arg("world"),
      "Convert a 2D index-coordinate point to a 2D world-coordinate point on the plane.")
    .def("index_to_world", py::overload_cast<const Vector2D&, Vector2D&>(&PlaneGeometry::IndexToWorld, py::const_), py::arg("index"), py::arg("world"),
      "Convert a 2D index-coordinate vector to a 2D world-coordinate vector on the plane.");
}

void InitSlicedGeometry3D(py::module_& m)
{
  py::class_<SlicedGeometry3D, BaseGeometry, SlicedGeometry3D::Pointer>(m, "SlicedGeometry3D",
    R"(Stack of :py:class:`PlaneGeometry` objects forming a 3D volume.

Used to describe the geometry of multi-slice acquisitions where each slice
may have its own plane geometry. Inherits the 3D world-space interface from
:py:class:`BaseGeometry`.
)")
    .def_static("new", &SlicedGeometry3D::New,
      "Construct a new identity-initialized sliced geometry.");
}

void InitTimeGeometry(py::module_& m)
{
  py::class_<TimeGeometry, TimeGeometry::Pointer>(m, "TimeGeometry",
    R"(Maps discrete time steps to floating-point time points.

A ``TimeGeometry`` is the temporal counterpart of :py:class:`BaseGeometry`.
It bridges integer time-step indices (used as array indices) and physical
time points (the time values a user sees). For static 3D data the time
geometry has a single time step that covers the full time range.

Subclasses:

- :py:class:`ProportionalTimeGeometry`: evenly spaced time points.
- :py:class:`ArbitraryTimeGeometry`: user-defined time points per step.
)")
    .def("count_time_steps", &TimeGeometry::CountTimeSteps,
      "Return the number of time steps.")
    .def("get_min_time_point", py::overload_cast<>(&TimeGeometry::GetMinimumTimePoint, py::const_),
      "Return the minimum (earliest) time point covered by this geometry.")
    .def("get_max_time_point", py::overload_cast<>(&TimeGeometry::GetMaximumTimePoint, py::const_),
      "Return the maximum (latest) time point covered by this geometry.")
    .def("get_min_time_point", py::overload_cast<TimeStepType>(&TimeGeometry::GetMinimumTimePoint, py::const_), py::arg("time_step"),
      R"(Return the minimum time point of a single time step.

Args:
    time_step: Time-step index.
)")
    .def("get_max_time_point", py::overload_cast<TimeStepType>(&TimeGeometry::GetMaximumTimePoint, py::const_), py::arg("time_step"),
      R"(Return the maximum time point of a single time step.

Args:
    time_step: Time-step index.
)")
    .def("get_time_bounds",
      [](const TimeGeometry& tg) {
        const auto b = tg.GetTimeBounds();
        return std::make_tuple(b[0], b[1]);
      },
      R"(Return the overall ``(min_time_point, max_time_point)`` tuple.

Returns:
    A 2-tuple of floats.
)")
    .def("get_time_bounds",
      [](const TimeGeometry& tg, TimeStepType t) {
        const auto b = tg.GetTimeBounds(t);
        return std::make_tuple(b[0], b[1]);
      },
      py::arg("time_step"),
      R"(Return the ``(min, max)`` time-point tuple for a single time step.

Args:
    time_step: Time-step index.

Returns:
    A 2-tuple of floats.
)")
    .def("time_step_to_time_point", &TimeGeometry::TimeStepToTimePoint, py::arg("time_step"),
      R"(Convert an integer time-step index to a floating-point time point.

Args:
    time_step: Time-step index.

Returns:
    The corresponding time point.
)")
    .def("time_point_to_time_step", &TimeGeometry::TimePointToTimeStep, py::arg("time_point"),
      R"(Convert a floating-point time point to a time-step index.

Args:
    time_point: Time point in physical units.

Returns:
    The matching time-step index. For out-of-range inputs, returns the
    nearest valid step (clamping behavior is defined by the subclass).
)")
    .def("is_valid_time_step", &TimeGeometry::IsValidTimeStep, py::arg("time_step"),
      R"(Return True if the index is a valid time step for this geometry.

Args:
    time_step: Candidate time-step index.
)")
    .def("is_valid_time_point", &TimeGeometry::IsValidTimePoint, py::arg("time_point"),
      R"(Return True if the time point is within the geometry's bounds.

Args:
    time_point: Candidate time point.
)")
    .def("get_geometry_for_time_step", &TimeGeometry::GetGeometryForTimeStep, py::arg("time_step"),
      R"(Return the :py:class:`BaseGeometry` associated with a time step.

Args:
    time_step: Time-step index.

Returns:
    The spatial geometry at that step.
)")
    .def("get_geometry_for_time_point", &TimeGeometry::GetGeometryForTimePoint, py::arg("time_point"),
      R"(Return the :py:class:`BaseGeometry` associated with a time point.

Args:
    time_point: Time point in physical units.

Returns:
    The spatial geometry at the matching time step.
)");
}

void InitArbitraryTimeGeometry(py::module_& m)
{
  py::class_<ArbitraryTimeGeometry, TimeGeometry, ArbitraryTimeGeometry::Pointer>(m, "ArbitraryTimeGeometry",
    R"(:py:class:`TimeGeometry` with user-defined time points per step.

Use when the time points are not evenly spaced (gated acquisitions,
re-binned reconstructions, sparse 4D series).
)")
    .def_static("new", &ArbitraryTimeGeometry::New,
      "Construct an empty arbitrary time geometry.");
}

void InitProportionalTimeGeometry(py::module_& m)
{
  py::class_<ProportionalTimeGeometry, TimeGeometry, ProportionalTimeGeometry::Pointer>(m, "ProportionalTimeGeometry",
    R"(:py:class:`TimeGeometry` with evenly spaced time points.

The default choice when a fixed time step covers the whole series.
)")
    .def_static("new", &ProportionalTimeGeometry::New,
      "Construct an empty proportional time geometry.");
}

void InitGeometries(py::module_& m)
{
  InitBaseGeometry(m);
  InitGeometry3D(m);
  InitPlaneGeometry(m);
  InitSlicedGeometry3D(m);
  InitTimeGeometry(m);
  InitArbitraryTimeGeometry(m);
  InitProportionalTimeGeometry(m);
}
