/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "GeometryHelpers.h"

#include <mitkExceptionMacro.h>

mitk::BaseGeometry::Pointer GetGeometryForTimeStep(mitk::TimeGeometry* tg, mitk::TimeStepType timeStep)
{
  if (tg == nullptr)
    mitkThrow() << "No time geometry available";

  auto geom = tg->GetGeometryForTimeStep(timeStep);

  if (geom.IsNull())
    mitkThrow() << "Invalid time step: " << timeStep;

  return geom;
}

mitk::BaseGeometry::ConstPointer GetConstGeometryForTimeStep(const mitk::TimeGeometry* tg, mitk::TimeStepType timeStep)
{
  if (tg == nullptr)
    mitkThrow() << "No time geometry available";

  auto geom = tg->GetGeometryForTimeStep(timeStep);

  if (geom.IsNull())
    mitkThrow() << "Invalid time step: " << timeStep;

  return mitk::BaseGeometry::ConstPointer(geom.GetPointer());
}

std::tuple<double, double, double> GetSpacing(const mitk::TimeGeometry* tg, mitk::TimeStepType t)
{
  auto geom = GetConstGeometryForTimeStep(tg, t);
  const auto s = geom->GetSpacing();
  return {s[0], s[1], s[2]};
}

void SetSpacing(mitk::TimeGeometry* tg, const std::array<double, 3>& spacing, mitk::TimeStepType t)
{
  auto geom = GetGeometryForTimeStep(tg, t);
  mitk::Vector3D v;
  v[0] = spacing[0];
  v[1] = spacing[1];
  v[2] = spacing[2];
  geom->SetSpacing(v);
}

std::tuple<double, double, double> GetOrigin(const mitk::TimeGeometry* tg, mitk::TimeStepType t)
{
  auto geom = GetConstGeometryForTimeStep(tg, t);
  const auto o = geom->GetOrigin();
  return {o[0], o[1], o[2]};
}

void SetOrigin(mitk::TimeGeometry* tg, const std::array<double, 3>& origin, mitk::TimeStepType t)
{
  auto geom = GetGeometryForTimeStep(tg, t);
  mitk::Point3D p;
  p[0] = origin[0];
  p[1] = origin[1];
  p[2] = origin[2];
  geom->SetOrigin(p);
}

py::array_t<double> ExtractDirection(const mitk::BaseGeometry& geom)
{
  const auto& matrix = geom.GetIndexToWorldTransform()->GetMatrix();
  const auto spacing = geom.GetSpacing();

  py::array_t<double> direction({static_cast<py::ssize_t>(3), static_cast<py::ssize_t>(3)});
  auto buf = direction.mutable_unchecked<2>();

  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      buf(row, col) = matrix[row][col] / spacing[col];

  return direction;
}

py::array_t<double> GetDirection(const mitk::TimeGeometry* tg, mitk::TimeStepType t)
{
  return ExtractDirection(*GetConstGeometryForTimeStep(tg, t));
}

void SetDirection(mitk::TimeGeometry* tg, py::array_t<double, py::array::c_style | py::array::forcecast> direction, mitk::TimeStepType t)
{
  if (direction.ndim() != 2 || direction.shape(0) != 3 || direction.shape(1) != 3)
    throw py::value_error("direction must be a 3x3 matrix");

  auto geom = GetGeometryForTimeStep(tg, t);
  auto buf = direction.unchecked<2>();
  const auto spacing = geom->GetSpacing();

  auto* transform = geom->GetIndexToWorldTransform();
  auto matrix = transform->GetMatrix();

  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      matrix[row][col] = buf(row, col) * spacing[col];

  transform->SetMatrix(matrix);
  geom->Modified();
}

std::array<double, 9> GetDirectionCosines(const mitk::TimeGeometry* tg, mitk::TimeStepType t)
{
  auto dir = GetDirection(tg, t);
  auto buf = dir.unchecked<2>();
  std::array<double, 9> flat{};

  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      flat[row * 3 + col] = buf(row, col);

  return flat;
}
