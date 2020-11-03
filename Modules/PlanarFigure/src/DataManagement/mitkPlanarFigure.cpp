/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigure.h"
#include "mitkPlaneGeometry.h"
#include <mitkProperties.h>
#include <mitkProportionalTimeGeometry.h>

#include <algorithm>

mitk::PlanarFigure::PlanarFigure()
  : m_SelectedControlPoint(-1),
    m_PreviewControlPointVisible(false),
    m_FigurePlaced(false),
    m_PolyLineUpToDate(false),
    m_HelperLinesUpToDate(false),
    m_FeaturesUpToDate(false),
    m_FeaturesMTime(0)
{
  m_HelperPolyLinesToBePainted = BoolContainerType::New();

  m_DisplaySize.first = 0.0;
  m_DisplaySize.second = 0;

  this->SetProperty("closed", mitk::BoolProperty::New(false));

  // Currently only single-time-step geometries are supported
  this->InitializeTimeGeometry(1);
}

mitk::PlanarFigure::PlanarFigure(const Self &other)
  : BaseData(other),
    m_ControlPoints(other.m_ControlPoints),
    m_NumberOfControlPoints(other.m_NumberOfControlPoints),
    m_SelectedControlPoint(other.m_SelectedControlPoint),
    m_PolyLines(other.m_PolyLines),
    m_HelperPolyLines(other.m_HelperPolyLines),
    m_PreviewControlPoint(other.m_PreviewControlPoint),
    m_PreviewControlPointVisible(other.m_PreviewControlPointVisible),
    m_FigurePlaced(other.m_FigurePlaced),
    m_PolyLineUpToDate(other.m_PolyLineUpToDate),
    m_HelperLinesUpToDate(other.m_HelperLinesUpToDate),
    m_FeaturesUpToDate(other.m_FeaturesUpToDate),
    m_Features(other.m_Features),
    m_FeaturesMTime(other.m_FeaturesMTime),
    m_DisplaySize(other.m_DisplaySize)
{
  m_HelperPolyLinesToBePainted = BoolContainerType::New();
  for (unsigned long i = 0; i < other.m_HelperPolyLinesToBePainted->Size(); ++i)
  {
    m_HelperPolyLinesToBePainted->InsertElement(i, other.m_HelperPolyLinesToBePainted->GetElement(i));
  }
}

void mitk::PlanarFigure::SetPlaneGeometry(mitk::PlaneGeometry *geometry)
{
  this->SetGeometry(geometry);

}

const mitk::PlaneGeometry *mitk::PlanarFigure::GetPlaneGeometry() const
{
  return dynamic_cast<PlaneGeometry*>(GetGeometry(0));
}

bool mitk::PlanarFigure::IsClosed() const
{
  mitk::BoolProperty *closed = dynamic_cast<mitk::BoolProperty *>(this->GetProperty("closed").GetPointer());
  if (closed != nullptr)
  {
    return closed->GetValue();
  }
  return false;
}

void mitk::PlanarFigure::PlaceFigure(const mitk::Point2D &point)
{
  for (unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i)
  {
    m_ControlPoints.push_back(this->ApplyControlPointConstraints(i, point));
  }

  m_FigurePlaced = true;
  m_SelectedControlPoint = 1;
}

bool mitk::PlanarFigure::AddControlPoint(const mitk::Point2D &point, int position)
{
  // if we already have the maximum number of control points, do nothing
  if (m_NumberOfControlPoints < this->GetMaximumNumberOfControlPoints())
  {
    // if position has not been defined or position would be the last control point, just append the new one
    // we also append a new point if we click onto the line between the first two control-points if the second
    // control-point is selected
    // -> special case for PlanarCross
    if (position == -1 || position > (int)m_NumberOfControlPoints - 1 || (position == 1 && m_SelectedControlPoint == 2))
    {
      if (m_ControlPoints.size() > this->GetMaximumNumberOfControlPoints() - 1)
      {
        // get rid of deprecated control points in the list. This is necessary
        // as ::ResetNumberOfControlPoints() only sets the member, does not resize the list!
        m_ControlPoints.resize(this->GetNumberOfControlPoints());
      }

      m_ControlPoints.push_back(this->ApplyControlPointConstraints(m_NumberOfControlPoints, point));
      m_SelectedControlPoint = m_NumberOfControlPoints;
    }
    else
    {
      // insert the point at the given position and set it as selected point
      auto iter = m_ControlPoints.begin() + position;
      m_ControlPoints.insert(iter, this->ApplyControlPointConstraints(position, point));
      for (unsigned int i = 0; i < m_ControlPoints.size(); ++i)
      {
        if (point == m_ControlPoints.at(i))
        {
          m_SelectedControlPoint = i;
        }
      }
    }

    // polylines & helperpolylines need to be repainted
    m_PolyLineUpToDate = false;
    m_HelperLinesUpToDate = false;
    m_FeaturesUpToDate = false;

    // one control point more
    ++m_NumberOfControlPoints;
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PlanarFigure::SetControlPoint(unsigned int index, const Point2D &point, bool createIfDoesNotExist)
{
  bool controlPointSetCorrectly = false;
  if (createIfDoesNotExist)
  {
    if (m_NumberOfControlPoints <= index)
    {
      m_ControlPoints.push_back(this->ApplyControlPointConstraints(index, point));
      m_NumberOfControlPoints++;
    }
    else
    {
      m_ControlPoints.at(index) = this->ApplyControlPointConstraints(index, point);
    }
    controlPointSetCorrectly = true;
  }
  else if (index < m_NumberOfControlPoints)
  {
    m_ControlPoints.at(index) = this->ApplyControlPointConstraints(index, point);
    controlPointSetCorrectly = true;
  }
  else
  {
    return false;
  }

  if (controlPointSetCorrectly)
  {
    m_PolyLineUpToDate = false;
    m_HelperLinesUpToDate = false;
    m_FeaturesUpToDate = false;
  }

  return controlPointSetCorrectly;
}

bool mitk::PlanarFigure::SetCurrentControlPoint(const Point2D &point)
{
  if ((m_SelectedControlPoint < 0) || (m_SelectedControlPoint >= (int)m_NumberOfControlPoints))
  {
    return false;
  }

  return this->SetControlPoint(m_SelectedControlPoint, point, false);
}

unsigned int mitk::PlanarFigure::GetNumberOfControlPoints() const
{
  return m_NumberOfControlPoints;
}

bool mitk::PlanarFigure::SelectControlPoint(unsigned int index)
{
  if (index < this->GetNumberOfControlPoints())
  {
    m_SelectedControlPoint = index;
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PlanarFigure::DeselectControlPoint()
{
  bool wasSelected = (m_SelectedControlPoint != -1);

  m_SelectedControlPoint = -1;

  return wasSelected;
}

void mitk::PlanarFigure::SetPreviewControlPoint(const Point2D &point)
{
  m_PreviewControlPoint = point;
  m_PreviewControlPointVisible = true;
}

void mitk::PlanarFigure::ResetPreviewContolPoint()
{
  m_PreviewControlPointVisible = false;
}

mitk::Point2D mitk::PlanarFigure::GetPreviewControlPoint() const
{
  return m_PreviewControlPoint;
}

bool mitk::PlanarFigure::IsPreviewControlPointVisible() const
{
  return m_PreviewControlPointVisible;
}

mitk::Point2D mitk::PlanarFigure::GetControlPoint(unsigned int index) const
{
  if (index < m_NumberOfControlPoints)
  {
    return m_ControlPoints.at(index);
  }

  itkExceptionMacro(<< "GetControlPoint(): Invalid index!");
}

mitk::Point3D mitk::PlanarFigure::GetWorldControlPoint(unsigned int index) const
{
  Point3D point3D;
  const auto planeGeometry = this->GetPlaneGeometry();
  if ((nullptr != planeGeometry) && (index < m_NumberOfControlPoints))
  {
    planeGeometry->Map(m_ControlPoints.at(index), point3D);
    return point3D;
  }

  itkExceptionMacro(<< "GetWorldControlPoint(): Invalid plane geometry or index!");
}

const mitk::PlanarFigure::PolyLineType mitk::PlanarFigure::GetPolyLine(unsigned int index)
{
  mitk::PlanarFigure::PolyLineType polyLine;
  if (index > m_PolyLines.size() || !m_PolyLineUpToDate)
  {
    this->GeneratePolyLine();
    m_PolyLineUpToDate = true;
  }

  return m_PolyLines.at(index);
}

const mitk::PlanarFigure::PolyLineType mitk::PlanarFigure::GetPolyLine(unsigned int index) const
{
  return m_PolyLines.at(index);
}

void mitk::PlanarFigure::ClearPolyLines()
{
  for (std::vector<PolyLineType>::size_type i = 0; i < m_PolyLines.size(); i++)
  {
    m_PolyLines.at(i).clear();
  }
  m_PolyLineUpToDate = false;
}

const mitk::PlanarFigure::PolyLineType mitk::PlanarFigure::GetHelperPolyLine(unsigned int index,
                                                                             double mmPerDisplayUnit,
                                                                             unsigned int displayHeight)
{
  mitk::PlanarFigure::PolyLineType helperPolyLine;
  if (index < m_HelperPolyLines.size())
  {
    // m_HelperLinesUpToDate does not cover changes in zoom-level, so we have to check previous values of the
    // two parameters as well
    if (!m_HelperLinesUpToDate || m_DisplaySize.first != mmPerDisplayUnit || m_DisplaySize.second != displayHeight)
    {
      this->GenerateHelperPolyLine(mmPerDisplayUnit, displayHeight);
      m_HelperLinesUpToDate = true;

      // store these parameters to be able to check next time if somebody zoomed in or out
      m_DisplaySize.first = mmPerDisplayUnit;
      m_DisplaySize.second = displayHeight;
    }

    helperPolyLine = m_HelperPolyLines.at(index);
  }

  return helperPolyLine;
}

void mitk::PlanarFigure::ClearHelperPolyLines()
{
  for (std::vector<PolyLineType>::size_type i = 0; i < m_HelperPolyLines.size(); i++)
  {
    m_HelperPolyLines.at(i).clear();
  }
  m_HelperLinesUpToDate = false;
}

/** \brief Returns the number of features available for this PlanarFigure
* (such as, radius, area, ...). */
unsigned int mitk::PlanarFigure::GetNumberOfFeatures() const
{
  return m_Features.size();
}

int mitk::PlanarFigure::GetControlPointForPolylinePoint(int indexOfPolylinePoint, int /*polyLineIndex*/) const
{
  return indexOfPolylinePoint;
}

const char *mitk::PlanarFigure::GetFeatureName(unsigned int index) const
{
  if (index < m_Features.size())
  {
    return m_Features[index].Name.c_str();
  }
  else
  {
    return nullptr;
  }
}

const char *mitk::PlanarFigure::GetFeatureUnit(unsigned int index) const
{
  if (index < m_Features.size())
  {
    return m_Features[index].Unit.c_str();
  }
  else
  {
    return nullptr;
  }
}

double mitk::PlanarFigure::GetQuantity(unsigned int index) const
{
  if (index < m_Features.size())
  {
    return m_Features[index].Quantity;
  }
  else
  {
    return 0.0;
  }
}

bool mitk::PlanarFigure::IsFeatureActive(unsigned int index) const
{
  if (index < m_Features.size())
  {
    return m_Features[index].Active;
  }
  else
  {
    return false;
  }
}

bool mitk::PlanarFigure::IsFeatureVisible(unsigned int index) const
{
  if (index < m_Features.size())
  {
    return m_Features[index].Visible;
  }
  else
  {
    return false;
  }
}

void mitk::PlanarFigure::SetFeatureVisible(unsigned int index, bool visible)
{
  if (index < m_Features.size())
  {
    m_Features[index].Visible = visible;
  }
}

void mitk::PlanarFigure::EvaluateFeatures()
{
  if (!m_FeaturesUpToDate || !m_PolyLineUpToDate)
  {
    if (!m_PolyLineUpToDate)
    {
      this->GeneratePolyLine();
    }

    this->EvaluateFeaturesInternal();

    m_FeaturesUpToDate = true;
  }
}

void mitk::PlanarFigure::UpdateOutputInformation()
{
  // Bounds are NOT calculated here, since the PlaneGeometry defines a fixed
  // frame (= bounds) for the planar figure.
  Superclass::UpdateOutputInformation();
  this->GetTimeGeometry()->Update();
}

void mitk::PlanarFigure::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::PlanarFigure::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::PlanarFigure::VerifyRequestedRegion()
{
  return true;
}

void mitk::PlanarFigure::SetRequestedRegion(const itk::DataObject * /*data*/)
{
}

void mitk::PlanarFigure::ResetNumberOfControlPoints(int numberOfControlPoints)
{
  // DO NOT resize the list here, will cause crash!!
  m_NumberOfControlPoints = numberOfControlPoints;
}

mitk::Point2D mitk::PlanarFigure::ApplyControlPointConstraints(unsigned int /*index*/, const Point2D &point)
{
  const auto planeGeometry = this->GetPlaneGeometry();
  if (nullptr == planeGeometry)
  {
    return point;
  }

  Point2D indexPoint;
  planeGeometry->WorldToIndex(point, indexPoint);

  BoundingBox::BoundsArrayType bounds = planeGeometry->GetBounds();
  if (indexPoint[0] < bounds[0])
  {
    indexPoint[0] = bounds[0];
  }
  if (indexPoint[0] > bounds[1])
  {
    indexPoint[0] = bounds[1];
  }
  if (indexPoint[1] < bounds[2])
  {
    indexPoint[1] = bounds[2];
  }
  if (indexPoint[1] > bounds[3])
  {
    indexPoint[1] = bounds[3];
  }

  Point2D constrainedPoint;
  planeGeometry->IndexToWorld(indexPoint, constrainedPoint);

  return constrainedPoint;
}

unsigned int mitk::PlanarFigure::AddFeature(const char *featureName, const char *unitName)
{
  unsigned int index = m_Features.size();

  Feature newFeature(featureName, unitName);
  m_Features.push_back(newFeature);

  return index;
}

void mitk::PlanarFigure::SetFeatureName(unsigned int index, const char *featureName)
{
  if (index < m_Features.size())
  {
    m_Features[index].Name = featureName;
  }
}

void mitk::PlanarFigure::SetFeatureUnit(unsigned int index, const char *unitName)
{
  if (index < m_Features.size())
  {
    m_Features[index].Unit = unitName;
  }
}

void mitk::PlanarFigure::SetQuantity(unsigned int index, double quantity)
{
  if (index < m_Features.size())
  {
    m_Features[index].Quantity = quantity;
  }
}

void mitk::PlanarFigure::ActivateFeature(unsigned int index)
{
  if (index < m_Features.size())
  {
    m_Features[index].Active = true;
  }
}

void mitk::PlanarFigure::DeactivateFeature(unsigned int index)
{
  if (index < m_Features.size())
  {
    m_Features[index].Active = false;
  }
}

void mitk::PlanarFigure::InitializeTimeGeometry(unsigned int timeSteps)
{
  mitk::PlaneGeometry::Pointer geometry2D = mitk::PlaneGeometry::New();
  geometry2D->Initialize();

  // The geometry is propagated automatically to all time steps,
  // if EvenlyTimed is true...
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(geometry2D, timeSteps);
  SetTimeGeometry(timeGeometry);
}

void mitk::PlanarFigure::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << this->GetNameOfClass() << ":\n";

  if (this->IsClosed())
    os << indent << "This figure is closed\n";
  else
    os << indent << "This figure is not closed\n";
  os << indent << "Minimum number of control points: " << this->GetMinimumNumberOfControlPoints() << std::endl;
  os << indent << "Maximum number of control points: " << this->GetMaximumNumberOfControlPoints() << std::endl;
  os << indent << "Current number of control points: " << this->GetNumberOfControlPoints() << std::endl;
  os << indent << "Control points:" << std::endl;

  for (unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i)
  {
    // os << indent.GetNextIndent() << i << ": " << m_ControlPoints->ElementAt( i ) << std::endl;
    os << indent.GetNextIndent() << i << ": " << m_ControlPoints.at(i) << std::endl;
  }
  os << indent << "Geometry:\n";
  this->GetPlaneGeometry()->Print(os, indent.GetNextIndent());
}

unsigned short mitk::PlanarFigure::GetPolyLinesSize()
{
  if (!m_PolyLineUpToDate)
  {
    this->GeneratePolyLine();
    m_PolyLineUpToDate = true;
  }
  return m_PolyLines.size();
}

unsigned short mitk::PlanarFigure::GetHelperPolyLinesSize() const
{
  return m_HelperPolyLines.size();
}

bool mitk::PlanarFigure::IsHelperToBePainted(unsigned int index) const
{
  return m_HelperPolyLinesToBePainted->GetElement(index);
}

bool mitk::PlanarFigure::ResetOnPointSelect()
{
  return false;
}

bool mitk::PlanarFigure::ResetOnPointSelectNeeded() const
{
  return false;
}

void mitk::PlanarFigure::RemoveControlPoint(unsigned int index)
{
  if (index > m_ControlPoints.size())
    return;

  if ((m_ControlPoints.size() - 1) < this->GetMinimumNumberOfControlPoints())
    return;

  ControlPointListType::iterator iter;
  iter = m_ControlPoints.begin() + index;

  m_ControlPoints.erase(iter);

  m_PolyLineUpToDate = false;
  m_HelperLinesUpToDate = false;
  m_FeaturesUpToDate = false;

  --m_NumberOfControlPoints;
}

void mitk::PlanarFigure::RemoveLastControlPoint()
{
  RemoveControlPoint(m_ControlPoints.size() - 1);
}

void mitk::PlanarFigure::SetNumberOfPolyLines(unsigned int numberOfPolyLines)
{
  m_PolyLines.resize(numberOfPolyLines);
}

void mitk::PlanarFigure::SetNumberOfHelperPolyLines(unsigned int numberOfHerlperPolyLines)
{
  m_HelperPolyLines.resize(numberOfHerlperPolyLines);
}

void mitk::PlanarFigure::AppendPointToPolyLine(unsigned int index, PolyLineElement element)
{
  if (index < m_PolyLines.size())
  {
    m_PolyLines[index].push_back(element);
    m_PolyLineUpToDate = false;
  }
  else
  {
    MITK_ERROR << "Tried to add point to PolyLine " << index + 1 << ", although only " << m_PolyLines.size()
               << " exists";
  }
}

void mitk::PlanarFigure::AppendPointToHelperPolyLine(unsigned int index, PolyLineElement element)
{
  if (index < m_HelperPolyLines.size())
  {
    m_HelperPolyLines[index].push_back(element);
    m_HelperLinesUpToDate = false;
  }
  else
  {
    MITK_ERROR << "Tried to add point to HelperPolyLine " << index + 1 << ", although only " << m_HelperPolyLines.size()
               << " exists";
  }
}

bool mitk::PlanarFigure::Equals(const mitk::PlanarFigure &other) const
{
  // check geometries
  if (this->GetPlaneGeometry() && other.GetPlaneGeometry())
  {
    if (!Equal(*(this->GetPlaneGeometry()), *(other.GetPlaneGeometry()), mitk::eps, true))
    {
      return false;
    }
  }
  else
  {
    MITK_ERROR << "Geometry is not equal";
    return false;
  }

  // check isPlaced member
  if (this->m_FigurePlaced != other.m_FigurePlaced)
  {
    MITK_ERROR << "Is_Placed is not equal";
    return false;
  }

  // check closed property
  if (this->IsClosed() != other.IsClosed())
  {
    MITK_ERROR << "Is_closed is not equal";
    return false;
  }

  // check poly lines
  if (this->m_PolyLines.size() != other.m_PolyLines.size())
  {
    return false;
  }
  else
  {
    auto itThis = this->m_PolyLines.begin();
    auto itEnd = this->m_PolyLines.end();
    auto itOther = other.m_PolyLines.begin();

    while (itThis != itEnd)
    {
      if (itThis->size() != itOther->size())
        return false;
      else
      {
        auto itLineThis = itThis->begin();
        auto itLineEnd = itThis->end();
        auto itLineOther = itOther->begin();

        while (itLineThis != itLineEnd)
        {
          Point2D p1 = *itLineThis;
          Point2D p2 = *itLineOther;
          ScalarType delta = fabs(p1[0] - p2[0]) + fabs(p1[1] - p2[1]);
          if (delta > .001)
          {
            MITK_ERROR << "Poly line is not equal";
            MITK_ERROR << p1 << "/" << p2;
            return false;
          }

          ++itLineThis;
          ++itLineOther;
        }
      }
      ++itThis;
      ++itOther;
    }
  }

  // check features
  if (this->GetNumberOfFeatures() != other.GetNumberOfFeatures())
  {
    MITK_ERROR << "Number of Features is Different";
    return false;
  }
  else
  {
    auto itThis = m_Features.begin();
    auto itEnd = m_Features.end();
    auto itOther = other.m_Features.begin();

    while (itThis != itEnd)
    {
      if ((itThis->Quantity - itOther->Quantity) > .001)
      {
        MITK_ERROR << "Quantity is Different" << itThis->Quantity << "/" << itOther->Quantity;
        return false;
      }
      if (itThis->Unit.compare(itOther->Unit) != 0)
      {
        MITK_ERROR << "Unit is Different" << itThis->Unit << "/" << itOther->Unit;
        return false;
      }
      if (itThis->Name.compare(itOther->Name) != 0)
      {
        MITK_ERROR << "Name of Measure is Different " << itThis->Name << "/ " << itOther->Name;
        ;
        return false;
      }

      ++itThis;
      ++itOther;
    }
  }

  return true;
}

bool mitk::Equal(const mitk::PlanarFigure &leftHandSide,
                 const mitk::PlanarFigure &rightHandSide,
                 ScalarType /*eps*/,
                 bool /*verbose*/)
{
  // FIXME: use eps and verbose
  return leftHandSide.Equals(rightHandSide);
}
