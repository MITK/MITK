/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureTessellator.h"

#include <vtkBox.h>
#include <vtkPointData.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
  // Joins whose miter would exceed this multiple of the half width are beveled.
  constexpr double MiterLimit = 4.0;
  constexpr double Epsilon = 1e-9;

  mitk::Vector2D Normalized(const mitk::Vector2D &vector)
  {
    const auto norm = vector.GetNorm();

    if (norm <= Epsilon)
      return vector;

    mitk::Vector2D result = vector;
    result /= norm;
    return result;
  }

  mitk::Vector2D LeftNormal(const mitk::Vector2D &direction)
  {
    mitk::Vector2D normal;
    normal[0] = -direction[1];
    normal[1] = direction[0];
    return normal;
  }

  double Cross(const mitk::Vector2D &a, const mitk::Vector2D &b)
  {
    return a[0] * b[1] - a[1] * b[0];
  }

  std::vector<mitk::Point2D> WithoutDuplicates(const std::vector<mitk::Point2D> &points, bool closed)
  {
    std::vector<mitk::Point2D> result;
    result.reserve(points.size());

    for (const auto &point : points)
    {
      if (result.empty() || result.back().EuclideanDistanceTo(point) > Epsilon)
        result.push_back(point);
    }

    if (closed && result.size() > 1 && result.front().EuclideanDistanceTo(result.back()) <= Epsilon)
      result.pop_back();

    return result;
  }
}

mitk::PlanarFigureTessellator::PlanarFigureTessellator()
  : m_PolyData(vtkSmartPointer<vtkPolyData>::New()),
    m_Points(vtkSmartPointer<vtkPoints>::New()),
    m_Colors(vtkSmartPointer<vtkUnsignedCharArray>::New()),
    m_Triangles(vtkSmartPointer<vtkCellArray>::New())
{
  m_Colors->SetNumberOfComponents(4);
  m_PolyData->SetPoints(m_Points);
  m_PolyData->SetPolys(m_Triangles);
  m_PolyData->GetPointData()->SetScalars(m_Colors);
}

vtkPolyData *mitk::PlanarFigureTessellator::GetPolyData() const
{
  return m_PolyData;
}

vtkIdType mitk::PlanarFigureTessellator::AddVertex(const Point2D &position, const Color &color, double alpha)
{
  const auto toByte = [](double value) {
    return static_cast<unsigned char>(std::lround(std::clamp(value, 0.0, 1.0) * 255.0));
  };

  const unsigned char rgba[4] = { toByte(color[0]), toByte(color[1]), toByte(color[2]), toByte(alpha) };
  m_Colors->InsertNextTypedTuple(rgba);

  return m_Points->InsertNextPoint(position[0], position[1], 0.0);
}

void mitk::PlanarFigureTessellator::AddTriangle(vtkIdType a, vtkIdType b, vtkIdType c)
{
  const vtkIdType ids[3] = { a, b, c };
  m_Triangles->InsertNextCell(3, ids);
}

void mitk::PlanarFigureTessellator::AddQuad(vtkIdType a, vtkIdType b, vtkIdType c, vtkIdType d)
{
  this->AddTriangle(a, b, c);
  this->AddTriangle(a, c, d);
}

void mitk::PlanarFigureTessellator::ConnectRows(const Row &from, const Row &to)
{
  this->AddQuad(from.LeftOuter, from.LeftInner, to.LeftInner, to.LeftOuter);
  this->AddQuad(from.LeftInner, from.RightInner, to.RightInner, to.LeftInner);
  this->AddQuad(from.RightInner, from.RightOuter, to.RightOuter, to.RightInner);
}

mitk::PlanarFigureTessellator::Row mitk::PlanarFigureTessellator::AddRow(
  const Point2D &center, const Vector2D &normal, double innerOffset, double outerOffset, const Color &color, double alpha)
{
  Row row;
  row.LeftOuter = this->AddVertex(center + normal * outerOffset, color, 0.0);
  row.LeftInner = this->AddVertex(center + normal * innerOffset, color, alpha);
  row.RightInner = this->AddVertex(center - normal * innerOffset, color, alpha);
  row.RightOuter = this->AddVertex(center - normal * outerOffset, color, 0.0);
  return row;
}

void mitk::PlanarFigureTessellator::AddStroke(
  const std::vector<Point2D> &points, bool closed, double width, const Color &color, double feather)
{
  auto vertices = WithoutDuplicates(points, closed);

  if (closed && vertices.size() < 3)
    closed = false;

  if (vertices.size() < 2 || width <= 0.0 || color[3] <= 0.0 || feather <= 0.0)
    return;

  const auto innerOffset = std::max(width - feather, 0.0) / 2.0;
  const auto outerOffset = innerOffset + feather;
  const auto alpha = color[3] * std::min(width / feather, 1.0);

  const auto numberOfVertices = vertices.size();
  const auto numberOfSegments = closed ? numberOfVertices : numberOfVertices - 1;

  std::vector<Vector2D> directions(numberOfSegments);

  for (std::size_t i = 0; i < numberOfSegments; ++i)
    directions[i] = Normalized(vertices[(i + 1) % numberOfVertices] - vertices[i]);

  // The coverage ramp of a cap is centered on the end point, so the visible
  // length matches the polyline.
  if (!closed)
  {
    const auto halfFeather = feather / 2.0;

    if (vertices[0].EuclideanDistanceTo(vertices[1]) > feather)
      vertices[0] += directions.front() * halfFeather;

    if (vertices[numberOfVertices - 1].EuclideanDistanceTo(vertices[numberOfVertices - 2]) > feather)
      vertices[numberOfVertices - 1] -= directions.back() * halfFeather;
  }

  std::vector<Row> incomingRows(numberOfVertices);
  std::vector<Row> outgoingRows(numberOfVertices);

  for (std::size_t i = 0; i < numberOfVertices; ++i)
  {
    const bool hasIncoming = closed || i > 0;
    const bool hasOutgoing = closed || i + 1 < numberOfVertices;

    if (!hasIncoming)
    {
      outgoingRows[i] = this->AddRow(vertices[i], LeftNormal(directions.front()), innerOffset, outerOffset, color, alpha);
      continue;
    }

    if (!hasOutgoing)
    {
      incomingRows[i] = this->AddRow(vertices[i], LeftNormal(directions.back()), innerOffset, outerOffset, color, alpha);
      continue;
    }

    const auto &incoming = directions[(i + numberOfSegments - 1) % numberOfSegments];
    const auto &outgoing = directions[i % numberOfSegments];
    const auto incomingNormal = LeftNormal(incoming);
    const auto outgoingNormal = LeftNormal(outgoing);

    auto miter = incomingNormal + outgoingNormal;
    const auto miterNorm = miter.GetNorm();

    if (miterNorm > Epsilon)
    {
      miter /= miterNorm;
      const auto cosine = miter * outgoingNormal;

      if (cosine > 1.0 / MiterLimit)
      {
        incomingRows[i] = this->AddRow(vertices[i], miter, innerOffset / cosine, outerOffset / cosine, color, alpha);
        outgoingRows[i] = incomingRows[i];
        continue;
      }
    }

    incomingRows[i] = this->AddRow(vertices[i], incomingNormal, innerOffset, outerOffset, color, alpha);
    outgoingRows[i] = this->AddRow(vertices[i], outgoingNormal, innerOffset, outerOffset, color, alpha);

    // The bevel fills the wedge that opens on the outer side of the turn.
    const auto center = this->AddVertex(vertices[i], color, alpha);
    const auto &in = incomingRows[i];
    const auto &out = outgoingRows[i];

    if (Cross(incoming, outgoing) > 0.0)
    {
      this->AddTriangle(center, in.RightInner, out.RightInner);
      this->AddQuad(in.RightInner, in.RightOuter, out.RightOuter, out.RightInner);
    }
    else
    {
      this->AddTriangle(center, out.LeftInner, in.LeftInner);
      this->AddQuad(out.LeftInner, out.LeftOuter, in.LeftOuter, in.LeftInner);
    }
  }

  for (std::size_t i = 0; i < numberOfSegments; ++i)
    this->ConnectRows(outgoingRows[i], incomingRows[(i + 1) % numberOfVertices]);

  if (!closed)
  {
    const auto startCap = this->AddRow(vertices.front() - directions.front() * feather,
      LeftNormal(directions.front()), innerOffset, outerOffset, color, 0.0);
    this->ConnectRows(startCap, outgoingRows.front());

    const auto endCap = this->AddRow(vertices.back() + directions.back() * feather,
      LeftNormal(directions.back()), innerOffset, outerOffset, color, 0.0);
    this->ConnectRows(incomingRows.back(), endCap);
  }
}

void mitk::PlanarFigureTessellator::AddDashedStroke(const std::vector<Point2D> &points, bool closed, double width,
  const Color &color, double dash, double gap, const Point2D &clipLowerLeft, const Point2D &clipUpperRight)
{
  if (dash <= 0.0 || gap <= 0.0)
  {
    this->AddStroke(points, closed, width, color);
    return;
  }

  // A dash reaches less than half its width plus two feathers beyond its center
  // line, so dashes outside this margin around the clip rectangle cannot touch it.
  constexpr double feather = 1.0;
  const auto margin = width / 2.0 + 2.0 * feather;

  const double bounds[6] = { clipLowerLeft[0] - margin, clipUpperRight[0] + margin,
    clipLowerLeft[1] - margin, clipUpperRight[1] + margin, 0.0, 0.0 };

  auto vertices = points;

  if (closed && vertices.size() > 1)
    vertices.push_back(vertices.front());

  std::vector<Point2D> piece;
  bool isDash = true;
  double phase = 0.0;

  const auto endPiece = [&]() {
    if (piece.size() > 1)
      this->AddStroke(piece, false, width, color, feather);

    piece.clear();
  };

  // Moves the pattern along the clipped parts, so it stays anchored to the start of the polyline.
  const auto advance = [&](double length) {
    const auto position = std::fmod((isDash ? phase : dash + phase) + length, dash + gap);
    isDash = position < dash;
    phase = isDash ? position : position - dash;
  };

  for (std::size_t i = 0; i + 1 < vertices.size(); ++i)
  {
    const auto &segmentStart = vertices[i];
    const auto segment = vertices[i + 1] - segmentStart;
    const auto segmentLength = segment.GetNorm();

    if (!std::isfinite(segmentLength))
    {
      endPiece();
      continue;
    }

    if (segmentLength <= Epsilon)
      continue;

    // The clipped ends are clamped into the box, which keeps the visible part short
    // also where huge coordinates leave no precision for the clipping.
    const double from[3] = { segmentStart[0], segmentStart[1], 0.0 };
    const double to[3] = { vertices[i + 1][0], vertices[i + 1][1], 0.0 };
    double lower = 0.0;
    double upper = 1.0;
    double clippedFrom[3];
    double clippedTo[3];
    int fromPlane = -1;
    int toPlane = -1;

    if (0 == vtkBox::IntersectWithLine(bounds, from, to, lower, upper, clippedFrom, clippedTo, fromPlane, toPlane) || lower >= upper)
    {
      endPiece();
      advance(segmentLength);
      continue;
    }

    if (lower > 0.0)
    {
      endPiece();
      advance(lower * segmentLength);
    }

    Point2D start;
    start[0] = clippedFrom[0];
    start[1] = clippedFrom[1];

    Vector2D visible;
    visible[0] = clippedTo[0] - clippedFrom[0];
    visible[1] = clippedTo[1] - clippedFrom[1];

    const auto length = visible.GetNorm();

    if (length > Epsilon)
    {
      const auto direction = visible / length;
      double position = 0.0;

      while (position < length)
      {
        const auto stateLength = isDash ? dash : gap;
        const auto step = std::min(stateLength - phase, length - position);

        if (isDash)
        {
          if (piece.empty())
            piece.push_back(start + direction * position);

          piece.push_back(start + direction * (position + step));
        }

        position += step;
        phase += step;

        if (phase >= stateLength - Epsilon)
        {
          if (isDash)
            endPiece();

          isDash = !isDash;
          phase = 0.0;
        }
      }
    }

    if (upper < 1.0)
    {
      endPiece();
      advance((1.0 - upper) * segmentLength);
    }
  }

  endPiece();
}

void mitk::PlanarFigureTessellator::AddCircularBands(
  const Point2D &center, const std::vector<double> &radii, const std::vector<double> &alphas, const Color &color)
{
  const auto maximumRadius = radii.back();
  const auto numberOfSegments = std::clamp(static_cast<int>(std::ceil(std::numbers::pi * maximumRadius)), 16, 128);

  std::vector<std::vector<vtkIdType>> rings;
  vtkIdType centerId = -1;

  for (std::size_t k = 0; k < radii.size(); ++k)
  {
    if (radii[k] <= Epsilon)
    {
      centerId = this->AddVertex(center, color, alphas[k]);
      continue;
    }

    std::vector<vtkIdType> ring(numberOfSegments);

    for (int j = 0; j < numberOfSegments; ++j)
    {
      const auto angle = 2.0 * std::numbers::pi * j / numberOfSegments;
      Vector2D offset;
      offset[0] = std::cos(angle) * radii[k];
      offset[1] = std::sin(angle) * radii[k];
      ring[j] = this->AddVertex(center + offset, color, alphas[k]);
    }

    rings.push_back(std::move(ring));
  }

  if (rings.empty())
    return;

  if (centerId >= 0)
  {
    for (int j = 0; j < numberOfSegments; ++j)
      this->AddTriangle(centerId, rings.front()[j], rings.front()[(j + 1) % numberOfSegments]);
  }

  for (std::size_t k = 0; k + 1 < rings.size(); ++k)
  {
    for (int j = 0; j < numberOfSegments; ++j)
    {
      const auto next = (j + 1) % numberOfSegments;
      this->AddQuad(rings[k][j], rings[k + 1][j], rings[k + 1][next], rings[k][next]);
    }
  }
}

void mitk::PlanarFigureTessellator::AddDisc(const Point2D &center, double radius, const Color &color)
{
  if (radius <= 0.0 || color[3] <= 0.0)
    return;

  const auto innerRadius = std::max(radius - 0.5, 0.0);
  const auto alpha = color[3] * std::min(2.0 * radius, 1.0);

  if (innerRadius > 0.0)
    this->AddCircularBands(center, { 0.0, innerRadius, radius + 0.5 }, { alpha, alpha, 0.0 }, color);
  else
    this->AddCircularBands(center, { 0.0, radius + 0.5 }, { alpha, 0.0 }, color);
}

void mitk::PlanarFigureTessellator::AddRoundedRectangle(
  const Point2D &lowerLeft, const Point2D &upperRight, double cornerRadius, const Color &color)
{
  const auto width = upperRight[0] - lowerLeft[0];
  const auto height = upperRight[1] - lowerLeft[1];

  if (width < 1.0 || height < 1.0 || color[3] <= 0.0)
    return;

  // The outline is inset and outset by half a pixel for the feather, which needs a radius of at least that.
  const auto radius = std::clamp(cornerRadius, 0.5, std::min(width, height) / 2.0);
  const auto segmentsPerCorner = std::max(4, static_cast<int>(std::ceil(radius)));

  // Counterclockwise, starting with the lower right corner, which turns from -90 to 0 degrees.
  std::array<Point2D, 4> cornerCenters;
  cornerCenters[0][0] = upperRight[0] - radius;
  cornerCenters[0][1] = lowerLeft[1] + radius;
  cornerCenters[1][0] = upperRight[0] - radius;
  cornerCenters[1][1] = upperRight[1] - radius;
  cornerCenters[2][0] = lowerLeft[0] + radius;
  cornerCenters[2][1] = upperRight[1] - radius;
  cornerCenters[3][0] = lowerLeft[0] + radius;
  cornerCenters[3][1] = lowerLeft[1] + radius;

  const auto addOutline = [&](double offset, double alpha) {
    std::vector<vtkIdType> ids;

    for (int corner = 0; corner < 4; ++corner)
    {
      for (int j = 0; j <= segmentsPerCorner; ++j)
      {
        const auto angle = (corner - 1 + static_cast<double>(j) / segmentsPerCorner) * std::numbers::pi / 2.0;

        Vector2D direction;
        direction[0] = std::cos(angle);
        direction[1] = std::sin(angle);

        ids.push_back(this->AddVertex(cornerCenters[corner] + direction * (radius + offset), color, alpha));
      }
    }

    return ids;
  };

  const auto inner = addOutline(-0.5, color[3]);
  const auto outer = addOutline(0.5, 0.0);

  Point2D center;
  center[0] = (lowerLeft[0] + upperRight[0]) / 2.0;
  center[1] = (lowerLeft[1] + upperRight[1]) / 2.0;

  const auto centerId = this->AddVertex(center, color, color[3]);
  const auto numberOfVertices = inner.size();

  for (std::size_t i = 0; i < numberOfVertices; ++i)
  {
    const auto next = (i + 1) % numberOfVertices;
    this->AddTriangle(centerId, inner[i], inner[next]);
    this->AddQuad(inner[i], outer[i], outer[next], inner[next]);
  }
}

void mitk::PlanarFigureTessellator::AddRing(const Point2D &center, double innerRadius, double outerRadius, const Color &color)
{
  const auto thickness = outerRadius - innerRadius;

  if (innerRadius < 0.0 || thickness <= 0.0 || color[3] <= 0.0)
    return;

  if (thickness >= 1.0)
  {
    this->AddCircularBands(center,
      { std::max(innerRadius - 0.5, 0.0), innerRadius + 0.5, outerRadius - 0.5, outerRadius + 0.5 },
      { 0.0, color[3], color[3], 0.0 }, color);
  }
  else
  {
    const auto middle = (innerRadius + outerRadius) / 2.0;

    this->AddCircularBands(center,
      { std::max(middle - 0.5, 0.0), middle, middle + 0.5 },
      { 0.0, color[3] * thickness, 0.0 }, color);
  }
}
