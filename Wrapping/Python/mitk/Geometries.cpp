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
  py::class_<BaseGeometry, BaseGeometry::Pointer>(m, "BaseGeometry")
    .def_static("new", &BaseGeometry::New)
    .def_property("image_geometry", &BaseGeometry::GetImageGeometry, &BaseGeometry::SetImageGeometry)
    .def_property("origin", &BaseGeometry::GetOrigin, &BaseGeometry::SetOrigin)
    .def_property("spacing", &BaseGeometry::GetSpacing, &BaseGeometry::SetSpacing)
    .def("enforce_spacing",
      [](BaseGeometry& g, const Vector3D& s) {
        g.SetSpacing(s, true);
      },
      py::arg("spacing"))
    .def_property("frame_of_reference_id", &BaseGeometry::GetFrameOfReferenceID, &BaseGeometry::SetFrameOfReferenceID)
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
      })
    .def_property_readonly("index_to_world_last_modified", &BaseGeometry::GetIndexToWorldTransformLastModified)
    .def_property_readonly("modified", &BaseGeometry::Modified)
    .def_property_readonly("center", &BaseGeometry::GetCenter)
    .def_property_readonly("diagonal_length", &BaseGeometry::GetDiagonalLength)
    .def("initialize", &BaseGeometry::Initialize)
    .def("initialize_geometry", &BaseGeometry::InitializeGeometry, py::arg("geometry"))
    .def("set_identity", &BaseGeometry::SetIdentity)
    .def("is_valid", &BaseGeometry::IsValid)
    .def("get_axis_vector", &BaseGeometry::GetAxisVector, py::arg("direction"))
    .def("get_extent", &BaseGeometry::GetExtent, py::arg("direction"))
    .def("get_extent_in_mm", &BaseGeometry::GetExtentInMM, py::arg("direction"))
    .def("set_extent_in_mm", &BaseGeometry::SetExtentInMM, py::arg("direction"), py::arg("extent"))
    .def("get_corner_point", py::overload_cast<int>(&BaseGeometry::GetCornerPoint, py::const_), py::arg("id"))
    .def("is_inside", &BaseGeometry::IsInside, py::arg("point"))
    .def("is_index_inside", [](const BaseGeometry& g, const Point3D& p) { return g.IsIndexInside(p); }, py::arg("index"))
    .def("clamp_point", &BaseGeometry::ClampPoint, py::arg("point"))
    .def("world_to_index", [](const BaseGeometry& g, const Point3D& w, Point3D& i) { return g.WorldToIndex(w, i); }, py::arg("world"), py::arg("index"))
    .def("world_to_index", [](const BaseGeometry& g, const Vector3D& w, Vector3D& i) { return g.WorldToIndex(w, i); }, py::arg("world"), py::arg("index"))
    .def("index_to_world", [](const BaseGeometry& g, const Point3D& i, Point3D& w) { return g.IndexToWorld(i, w); }, py::arg("index"), py::arg("world"))
    .def("index_to_world", [](const BaseGeometry& g, const Vector3D& i, Vector3D& w) { return g.IndexToWorld(i, w); }, py::arg("index"), py::arg("world"))
    .def("translate", &BaseGeometry::Translate, py::arg("vector"));
}

void InitGeometry3D(py::module_& m)
{
  py::class_<Geometry3D, BaseGeometry, Geometry3D::Pointer>(m, "Geometry3D")
    .def_static("new", py::overload_cast<>(&Geometry3D::New));
}

void InitPlaneGeometry(py::module_& m)
{
  py::class_<PlaneGeometry, BaseGeometry, PlaneGeometry::Pointer>(m, "PlaneGeometry")
    .def_static("new", &PlaneGeometry::New)
    .def_property_readonly("normal", &PlaneGeometry::GetNormal)
    .def("world_to_index", py::overload_cast<const Point2D&, Point2D&>(&PlaneGeometry::WorldToIndex, py::const_), py::arg("world"), py::arg("index"))
    .def("world_to_index", py::overload_cast<const Vector2D&, Vector2D&>(&PlaneGeometry::WorldToIndex, py::const_), py::arg("world"), py::arg("index"))
    .def("index_to_world", py::overload_cast<const Point2D&, Point2D&>(&PlaneGeometry::IndexToWorld, py::const_), py::arg("index"), py::arg("world"))
    .def("index_to_world", py::overload_cast<const Vector2D&, Vector2D&>(&PlaneGeometry::IndexToWorld, py::const_), py::arg("index"), py::arg("world"));
}

void InitSlicedGeometry3D(py::module_& m)
{
  py::class_<SlicedGeometry3D, BaseGeometry, SlicedGeometry3D::Pointer>(m, "SlicedGeometry3D")
    .def_static("new", &SlicedGeometry3D::New);
}

void InitTimeGeometry(py::module_& m)
{
  py::class_<TimeGeometry, TimeGeometry::Pointer>(m, "TimeGeometry")
    .def_static("new", &TimeGeometry::New);
}

void InitArbitratyTimeGeometry(py::module_& m)
{
  py::class_<ArbitraryTimeGeometry, TimeGeometry, ArbitraryTimeGeometry::Pointer>(m, "ArbitraryTimeGeometry")
    .def_static("new", &ArbitraryTimeGeometry::New);
}

void InitProportionalTimeGeometry(py::module_& m)
{
  py::class_<ProportionalTimeGeometry, TimeGeometry, ProportionalTimeGeometry::Pointer>(m, "ProportionalTimeGeometry")
    .def_static("new", &ProportionalTimeGeometry::New);
}

void InitGeometries(py::module_& m)
{
  InitBaseGeometry(m);
  InitGeometry3D(m);
  InitPlaneGeometry(m);
  InitSlicedGeometry3D(m);
  InitTimeGeometry(m);
  InitArbitratyTimeGeometry(m);
  InitProportionalTimeGeometry(m);
}
