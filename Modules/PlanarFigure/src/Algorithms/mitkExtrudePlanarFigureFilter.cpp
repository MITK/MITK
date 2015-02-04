/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPlanarFigure.h"
#include <mitkExtrudePlanarFigureFilter.h>
#include <mitkPlaneGeometry.h>
#include <mitkSurface.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

static mitk::Point2D GetCenterPoint(const mitk::PlanarFigure::PolyLineType& polyLine)
{
  mitk::Point2D centerPoint;

  centerPoint[0] = 0;
  centerPoint[1] = 0;

  mitk::PlanarFigure::PolyLineType::const_iterator polyLineEnd = polyLine.end();

  for (mitk::PlanarFigure::PolyLineType::const_iterator polyLineIter = polyLine.begin(); polyLineIter != polyLineEnd; ++polyLineIter)
  {
    centerPoint[0] += static_cast<mitk::Point2D>(*polyLineIter)[0];
    centerPoint[1] += static_cast<mitk::Point2D>(*polyLineIter)[1];
  }

  size_t numPoints = polyLine.size();

  centerPoint[0] /= numPoints;
  centerPoint[1] /= numPoints;

  return centerPoint;
}

static mitk::Point2D GetCenterPoint(mitk::PlanarFigure* planarFigure)
{
  mitk::Point2D centerPoint;

  centerPoint[0] = 0;
  centerPoint[1] = 0;

  size_t numPolyLines = planarFigure->GetPolyLinesSize();

  for (size_t i = 0; i < numPolyLines; ++i)
  {
    mitk::Point2D polyLineCenterPoint = GetCenterPoint(planarFigure->GetPolyLine(i));

    centerPoint[0] += polyLineCenterPoint[0];
    centerPoint[1] += polyLineCenterPoint[1];
  }

  centerPoint[0] /= numPolyLines;
  centerPoint[1] /= numPolyLines;

  return centerPoint;
}

static mitk::Vector3D GetBendDirection(const mitk::PlaneGeometry* planeGeometry, const mitk::Point2D& centerPoint2d, const mitk::Vector2D& bendDirection2d)
{
  mitk::Point2D point2d = centerPoint2d + bendDirection2d;

  mitk::Point3D point3d;
  planeGeometry->Map(point2d, point3d);

  mitk::Point3D centerPoint3d;
  planeGeometry->Map(centerPoint2d, centerPoint3d);

  mitk::Vector3D bendDirection3d = point3d - centerPoint3d;
  bendDirection3d.Normalize();

  return bendDirection3d;
}

mitk::ExtrudePlanarFigureFilter::ExtrudePlanarFigureFilter()
  : m_Length(1),
    m_NumberOfSegments(1),
    m_TwistAngle(0),
    m_BendAngle(0),
    m_FlipDirection(false),
    m_FlipNormals(false)
{
  m_BendDirection[0] = 0;
  m_BendDirection[1] = 0;

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);

  this->SetNthOutput(0, this->MakeOutput(0));
}

mitk::ExtrudePlanarFigureFilter::~ExtrudePlanarFigureFilter()
{
}

void mitk::ExtrudePlanarFigureFilter::GenerateData()
{
  typedef PlanarFigure::PolyLineType PolyLine;
  typedef PolyLine::const_iterator PolyLineConstIter;

  if (m_Length <= 0)
    mitkThrow() << "Length is not positive!";

  if (m_NumberOfSegments == 0)
    mitkThrow() << "Number of segments is zero!";

  if (m_BendAngle != 0 && m_BendDirection[0] == 0 && m_BendDirection[1] == 0)
    mitkThrow() << "Bend direction is zero-length vector!";

  PlanarFigure* input = dynamic_cast<PlanarFigure*>(this->GetPrimaryInput());

  if (input == NULL)
    mitkThrow() << "Primary input is not a planar figure!";

  size_t numPolyLines = input->GetPolyLinesSize();

  if (numPolyLines == 0)
    mitkThrow() << "Primary input does not contain any poly lines!";

  const PlaneGeometry* planeGeometry = dynamic_cast<const PlaneGeometry*>(input->GetPlaneGeometry());

  if (planeGeometry == NULL)
    mitkThrow() << "Could not get plane geometry from primary input!";

  Vector3D planeNormal = planeGeometry->GetNormal();
  planeNormal.Normalize();

  Point2D centerPoint2d = GetCenterPoint(input);

  Point3D centerPoint3d;
  planeGeometry->Map(centerPoint2d, centerPoint3d);

  Vector3D bendDirection3d = m_BendAngle != 0
    ? ::GetBendDirection(planeGeometry, centerPoint2d, m_BendDirection)
    : Vector3D();

  ScalarType radius = m_Length * (360 / m_BendAngle) / (2 * vnl_math::pi);
  Vector3D scaledBendDirection3d = bendDirection3d * radius;

  Vector3D bendAxis = itk::CrossProduct(planeNormal, bendDirection3d);
  bendAxis.Normalize();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  vtkIdType baseIndex = 0;

  for (size_t i = 0; i < numPolyLines; ++i)
  {
    PolyLine polyLine = input->GetPolyLine(i);
    size_t numPoints = polyLine.size();

    if (numPoints < 2)
      mitkThrow() << "Poly line " << i << " of primary input consists of less than two points!";

    std::vector<mitk::Point3D> crossSection;

    PolyLineConstIter polyLineEnd = polyLine.end();

    for (PolyLineConstIter polyLineIter = polyLine.begin(); polyLineIter != polyLineEnd; ++polyLineIter)
    {
      Point3D point;
      planeGeometry->Map(*polyLineIter, point);
      crossSection.push_back(point);
    }

    ScalarType segmentLength = m_Length / m_NumberOfSegments;
    Vector3D translation = planeNormal * segmentLength;

    bool bend = std::abs(m_BendAngle) > mitk::eps;
    bool twist = std::abs(m_TwistAngle) > mitk::eps;

    ScalarType twistAngle = twist
      ? m_TwistAngle / m_NumberOfSegments * vnl_math::pi / 180
      : 0;

    ScalarType bendAngle = bend
      ? m_BendAngle / m_NumberOfSegments * vnl_math::pi / 180
      : 0;

    if (m_FlipDirection)
    {
      translation *= -1;
      bendAngle *= -1;
    }

    for (size_t k = 0; k < numPoints; ++k)
      points->InsertNextPoint(crossSection[k].GetDataPointer());

    for (size_t j = 1; j <= m_NumberOfSegments; ++j)
    {
      mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();

      if (bend || twist)
        transform->Translate(centerPoint3d.GetVectorFromOrigin(), true);

      if (bend)
      {
        transform->Translate(scaledBendDirection3d, true);
        transform->Rotate3D(bendAxis, bendAngle * j, true);
        transform->Translate(-scaledBendDirection3d, true);
      }
      else
      {
        transform->Translate(translation * j, true);
      }

      if (twist)
        transform->Rotate3D(planeNormal, twistAngle * j, true);

      if (bend || twist)
        transform->Translate(-centerPoint3d.GetVectorFromOrigin(), true);

      for (size_t k = 0; k < numPoints; ++k)
      {
        mitk::Point3D transformedPoint = transform->TransformPoint(crossSection[k]);
        points->InsertNextPoint(transformedPoint.GetDataPointer());
      }
    }

    for (size_t j = 0; j < m_NumberOfSegments; ++j)
    {
      for (size_t k = 1; k < numPoints; ++k)
      {
        vtkIdType cell[3];
        cell[0] = baseIndex + j * numPoints + (k - 1);
        cell[1] = baseIndex + (j + 1) * numPoints + (k - 1);
        cell[2] = baseIndex + j * numPoints + k;

        cells->InsertNextCell(3, cell);

        cell[0] = cell[1];
        cell[1] = baseIndex + (j + 1) * numPoints + k;

        cells->InsertNextCell(3, cell);
      }

      if (input->IsClosed() && numPoints > 2)
      {
        vtkIdType cell[3];
        cell[0] = baseIndex + j * numPoints + (numPoints - 1);
        cell[1] = baseIndex + (j + 1) * numPoints + (numPoints - 1);
        cell[2] = baseIndex + j * numPoints;

        cells->InsertNextCell(3, cell);

        cell[0] = cell[1];
        cell[1] = baseIndex + (j + 1) * numPoints;

        cells->InsertNextCell(3, cell);
      }
    }

    baseIndex += points->GetNumberOfPoints();
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetPolys(cells);

  vtkSmartPointer<vtkPolyDataNormals> polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
  polyDataNormals->SetFlipNormals(m_FlipNormals);
  polyDataNormals->SetInputData(polyData);
  polyDataNormals->SplittingOff();

  polyDataNormals->Update();

  Surface* output = static_cast<Surface*>(this->GetPrimaryOutput());
  output->SetVtkPolyData(polyDataNormals->GetOutput());
}

void mitk::ExtrudePlanarFigureFilter::GenerateOutputInformation()
{
}

itk::ProcessObject::DataObjectPointer mitk::ExtrudePlanarFigureFilter::MakeOutput(DataObjectPointerArraySizeType idx)
{
  return idx == 0
    ? Surface::New().GetPointer()
    : NULL;
}

itk::ProcessObject::DataObjectPointer mitk::ExtrudePlanarFigureFilter::MakeOutput(const DataObjectIdentifierType& name)
{
  return this->IsIndexedOutputName(name)
    ? this->MakeOutput(this->MakeIndexFromOutputName(name))
    : NULL;
}

void mitk::ExtrudePlanarFigureFilter::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Length: " << m_Length << std::endl;
  os << indent << "Number of Segments: " << m_NumberOfSegments << std::endl;
  os << indent << "Twist Angle: " << m_TwistAngle << std::endl;
  os << indent << "Bend Angle: " << m_BendAngle << std::endl;
  os << indent << "Bend Direction: " << m_BendDirection << std::endl;
  os << indent << "Flip Normals: " << m_FlipNormals << std::endl;
}

void mitk::ExtrudePlanarFigureFilter::SetInput(PlanarFigure* planarFigure)
{
  this->SetPrimaryInput(planarFigure);
}

mitk::Surface* mitk::ExtrudePlanarFigureFilter::GetOutput()
{
  return static_cast<Surface*>(this->GetPrimaryOutput());
}
