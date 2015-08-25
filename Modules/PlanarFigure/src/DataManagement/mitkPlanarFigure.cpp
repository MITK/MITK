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
#include "mitkPlaneGeometry.h"
#include <mitkProperties.h>
#include "mitkPlanarFigureOperation.h"
#include "mitkInteractionConst.h"
#include <mitkProportionalTimeGeometry.h>

#include <algorithm>

mitk::PlanarFigure::PlanarFigure()
: m_SelectedControlPoint( -1 ),
  m_PreviewControlPointVisible( false ),
  m_FigurePlaced( false ),
  m_FigureFinalized( false ),
  m_PlaneGeometry( nullptr ),
  m_PolyLineUpToDate(false),
  m_HelperLinesUpToDate(false),
  m_FeaturesUpToDate(false),
  m_FeaturesMTime( 0 )
{
  m_HelperPolyLinesToBePainted = BoolContainerType::New();

  m_DisplaySize.first = 0.0;
  m_DisplaySize.second = 0;

  this->SetProperty( "closed", mitk::BoolProperty::New( false ) );

  // Currently only single-time-step geometries are supported
  this->InitializeTimeGeometry( 1 );
}


mitk::PlanarFigure::PlanarFigure(const Self& other)
  : BaseData(other),
    m_ControlPoints(other.m_ControlPoints),
    m_SelectedControlPoint(other.m_SelectedControlPoint),
    m_PreviewControlPoint(other.m_PreviewControlPoint),
    m_PreviewControlPointVisible(other.m_PreviewControlPointVisible),
    m_FigurePlaced(other.m_FigurePlaced),
    m_FigureFinalized(other.m_FigureFinalized),
    m_HelperPolyLinesToBePainted(other.m_HelperPolyLinesToBePainted->Clone()),
    m_PlaneGeometry(other.m_PlaneGeometry),
    m_PolyLineUpToDate(other.m_PolyLineUpToDate),
    m_HelperLinesUpToDate(other.m_HelperLinesUpToDate), // do not clone since SetPlaneGeometry() doesn't clone either
    m_FeaturesUpToDate(other.m_FeaturesUpToDate),
    m_PolyLines(other.m_PolyLines),
    m_HelperPolyLines(other.m_HelperPolyLines),
    m_Features(other.m_Features),
    m_FeaturesMTime(other.m_FeaturesMTime),
    m_DisplaySize(other.m_DisplaySize)
{
}


void mitk::PlanarFigure::SetPlaneGeometry( mitk::PlaneGeometry *geometry )
{
  this->SetGeometry( geometry );
  m_PlaneGeometry = dynamic_cast<PlaneGeometry *>(GetGeometry(0));//geometry;
}


const mitk::PlaneGeometry *mitk::PlanarFigure::GetPlaneGeometry() const
{
  return m_PlaneGeometry;
}


bool mitk::PlanarFigure::IsClosed() const
{
  mitk::BoolProperty* closed = dynamic_cast< mitk::BoolProperty* >( this->GetProperty( "closed" ).GetPointer() );
  if ( closed != nullptr )
  {
    return closed->GetValue();
  }
  return false;
}


void mitk::PlanarFigure::PlaceFigure( const mitk::Point2D& point )
{
  for (unsigned int i = 0; i < this->GetPlacementNumberOfControlPoints(); ++i)
  {
    AddControlPoint(this->ApplyControlPointConstraints(i, point), i);
  }

  m_FigurePlaced = true;
  SelectControlPoint(this->GetPlacementSelectedPointId());
  this->InvokeEvent(EndPlacementPlanarFigureEvent());
  this->Modified();
}

void mitk::PlanarFigure::CancelPlaceFigure()
{
  m_ControlPoints.resize(0);
  m_FigurePlaced = false;
  m_SelectedControlPoint = -1;
  this->InvokeEvent(CancelPlacementPlanarFigureEvent());
  this->Modified();
}

void mitk::PlanarFigure::SetFinalized(bool finalized)
{ 
    m_FigureFinalized = finalized; 
    if (finalized)  
    {
        this->InvokeEvent(FinalizedPlanarFigureEvent());
    }
    this->Modified(); 
}

bool mitk::PlanarFigure::AddControlPoint(const mitk::Point2D& point, int position)
{
    // if we already have the maximum number of control points, do nothing
    if (GetNumberOfControlPoints() < this->GetMaximumNumberOfControlPoints())
    {
        // if position has not been defined or position would be the last control point, just append the new one
        // we also append a new point if we click onto the line between the first two control-points if the second control-point is selected
        // -> special case for PlanarCross
        if (position == -1 || position > (int)GetNumberOfControlPoints() - 1 || (position == 1 && m_SelectedControlPoint == 2))
        {
            m_ControlPoints.push_back(this->ApplyControlPointConstraints(m_ControlPoints.size(), point));
            SelectControlPoint(m_ControlPoints.size() - 1);
        }
        else
        {
            // insert the point at the given position and set it as selected point
            ControlPointListType::iterator iter = m_ControlPoints.begin() + position;
            m_ControlPoints.insert(iter, this->ApplyControlPointConstraints(position, point));
            SelectControlPoint(position);
        }

        this->Modified();
        return true;
    }
    else
    {
        return false;
    }
}


bool mitk::PlanarFigure::SetControlPoint(unsigned int index, const Point2D& point, bool createIfDoesNotExist)
{
    bool controlPointSetCorrectly = false;
    if (createIfDoesNotExist)
    {
        if (GetNumberOfControlPoints() <= index)
        {
            m_ControlPoints.push_back(this->ApplyControlPointConstraints(index, point));
        }
        else
        {
            m_ControlPoints.at(index) = this->ApplyControlPointConstraints(index, point);
        }
        controlPointSetCorrectly = true;
    }
    else if (index < GetNumberOfControlPoints())
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
        this->Modified();
    }

    return controlPointSetCorrectly;
}


bool mitk::PlanarFigure::SetCurrentControlPoint(const Point2D& point)
{
    if ((m_SelectedControlPoint < 0) || (m_SelectedControlPoint >= (int)GetNumberOfControlPoints()))
    {
        return false;
    }

    return this->SetControlPoint(m_SelectedControlPoint, point, false);
}


unsigned int mitk::PlanarFigure::GetNumberOfControlPoints() const
{
    return m_ControlPoints.size();
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

void mitk::PlanarFigure::SetPreviewControlPoint(const Point2D& point)
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
    if (index < GetNumberOfControlPoints())
    {
        return m_ControlPoints.at(index);
    }

    itkExceptionMacro(<< "GetControlPoint(): Invalid index!");
}

bool mitk::PlanarFigure::IsPointNearLine(
    const mitk::Point2D& point,
    const mitk::Point2D& startPoint,
    const mitk::Point2D& endPoint,
    double maxDistance,
    mitk::Point2D& projectedPoint
    ) 
{
    mitk::Vector2D n1 = endPoint - startPoint;
    n1.Normalize();

    // Determine dot products between line vector and startpoint-point / endpoint-point vectors
    double l1 = n1 * (point - startPoint);
    double l2 = -n1 * (point - endPoint);

    // Determine projection of specified point onto line defined by start / end point
    mitk::Point2D crossPoint = startPoint + n1 * l1;
    projectedPoint = crossPoint;

    float dist1 = crossPoint.SquaredEuclideanDistanceTo(point);
    float dist2 = endPoint.SquaredEuclideanDistanceTo(point);
    float dist3 = startPoint.SquaredEuclideanDistanceTo(point);

    // Point is inside encompassing rectangle IF
    // - its distance to its projected point is small enough
    // - it is not further outside of the line than the defined tolerance
    if (((dist1 < maxDistance * maxDistance) && (l1 > 0.0) && (l2 > 0.0))
        || dist2 < maxDistance * maxDistance
        || dist3 < maxDistance * maxDistance)
    {
        return true;
    }

    return false;
}

std::tuple<int, int, mitk::Point2D> mitk::PlanarFigure::FindClosestPolyLinePoint( const mitk::Point2D& point, double maxDistance ) const
{
    mitk::Point2D pointProjectedOntoLine;

    for (unsigned short polyLineIndex = 0; polyLineIndex < GetPolyLinesSize(); ++polyLineIndex)
    {
        auto polyLine = GetPolyLine(polyLineIndex);

        for (std::size_t polyLinePointIndex = 1; polyLinePointIndex < polyLine.size(); ++polyLinePointIndex)
        {
            if (IsPointNearLine(point, polyLine[polyLinePointIndex - 1], polyLine[polyLinePointIndex], maxDistance, pointProjectedOntoLine)) {
                return std::make_tuple(static_cast<int>(polyLineIndex), static_cast<int>(polyLinePointIndex), pointProjectedOntoLine);
            }
        }

        // For closed figures, also check last line segment
        if (IsClosed()
            && IsPointNearLine(point, *polyLine.rbegin(), polyLine[0], maxDistance, pointProjectedOntoLine))
        {
            return std::make_tuple(static_cast<int>(polyLineIndex), static_cast<int>(polyLine.size()), pointProjectedOntoLine);
        }
    }
    return std::make_tuple(-1, -1, mitk::Point2D{});
}

int mitk::PlanarFigure::FindClosestControlPoint(const mitk::Point2D & point, double maxDistance) const
{
    int numberOfControlPoints = GetNumberOfControlPoints();
    for (auto i = 0u; i < GetNumberOfControlPoints(); ++i)
    {
        if (point.SquaredEuclideanDistanceTo(GetControlPoint(i)) < maxDistance * maxDistance)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

mitk::Point3D mitk::PlanarFigure::GetWorldControlPoint(unsigned int index) const
{
    Point3D point3D;
    if ((m_PlaneGeometry != nullptr) && (index < GetNumberOfControlPoints()))
    {
    m_PlaneGeometry->Map( m_ControlPoints.at( index ), point3D );
        return point3D;
    }

    itkExceptionMacro(<< "GetWorldControlPoint(): Invalid index!");
}


const mitk::PlanarFigure::PolyLineType
mitk::PlanarFigure::GetPolyLine(unsigned int index) const
{
    mitk::PlanarFigure::PolyLineType polyLine;
    if (index > m_PolyLines.size() || !m_PolyLineUpToDate)
    {
        const_cast<mitk::PlanarFigure*>(this)->GeneratePolyLine();
        m_PolyLineUpToDate = true;
    }

  return m_PolyLines.at( index );
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
    unsigned int displayHeight) const
{
    mitk::PlanarFigure::PolyLineType helperPolyLine;
    if (index < m_HelperPolyLines.size())
    {
        // m_HelperLinesUpToDate does not cover changes in zoom-level, so we have to check previous values of the
        // two parameters as well
        if (!m_HelperLinesUpToDate || m_DisplaySize.first != mmPerDisplayUnit || m_DisplaySize.second != displayHeight)
        {
            const_cast<mitk::PlanarFigure*>(this)->GenerateHelperPolyLine(mmPerDisplayUnit, displayHeight);
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
    if (m_FigurePlaced && !m_FeaturesUpToDate) {
        const_cast<mitk::PlanarFigure*>(this)->EvaluateFeaturesInternal();
        m_FeaturesUpToDate = true;
    }

    return m_Features.size();
}


const char *mitk::PlanarFigure::GetFeatureName( unsigned int index ) const
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
    if (!m_FigurePlaced) {
        return 0;
    }

    if (!m_FeaturesUpToDate) {
        const_cast<mitk::PlanarFigure*>(this)->EvaluateFeaturesInternal();
        m_FeaturesUpToDate = true;
    }

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

void mitk::PlanarFigure::Modified() const
{
    this->m_FeaturesUpToDate = false;
    this->m_PolyLineUpToDate = false;
    this->m_HelperLinesUpToDate = false;
    Superclass::Modified();
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


mitk::Point2D mitk::PlanarFigure::ApplyControlPointConstraints(unsigned int /*index*/, const Point2D& point)
{
  if ( m_PlaneGeometry ==  nullptr )
    {
        return point;
    }

    Point2D indexPoint;
  m_PlaneGeometry->WorldToIndex( point, indexPoint );

  BoundingBox::BoundsArrayType bounds = m_PlaneGeometry->GetBounds();
    if (indexPoint[0] < bounds[0]) { indexPoint[0] = bounds[0]; }
    if (indexPoint[0] > bounds[1]) { indexPoint[0] = bounds[1]; }
    if (indexPoint[1] < bounds[2]) { indexPoint[1] = bounds[2]; }
    if (indexPoint[1] > bounds[3]) { indexPoint[1] = bounds[3]; }

    Point2D constrainedPoint;
  m_PlaneGeometry->IndexToWorld( indexPoint, constrainedPoint );

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


void mitk::PlanarFigure::PrintSelf(std::ostream& os, itk::Indent indent) const
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
        //os << indent.GetNextIndent() << i << ": " << m_ControlPoints->ElementAt( i ) << std::endl;
        os << indent.GetNextIndent() << i << ": " << m_ControlPoints.at(i) << std::endl;
    }
    os << indent << "Geometry:\n";
  this->GetPlaneGeometry()->Print(os, indent.GetNextIndent());
}


unsigned short mitk::PlanarFigure::GetPolyLinesSize() const
{
    if (!m_PolyLineUpToDate)
    {
        const_cast<mitk::PlanarFigure*>(this)->GeneratePolyLine();
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

void mitk::PlanarFigure::RemoveControlPoint( unsigned int index )
{
    if (index >= GetNumberOfControlPoints())
        return;

    if ((m_ControlPoints.size() - 1) < this->GetMinimumNumberOfControlPoints())
        return;

    ControlPointListType::iterator iter;
    iter = m_ControlPoints.begin() + index;

    m_ControlPoints.erase(iter);

    this->Modified();
}

void mitk::PlanarFigure::RemoveLastControlPoint()
{
    RemoveControlPoint(GetNumberOfControlPoints() - 1);
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
        MITK_ERROR << "Tried to add point to PolyLine " << index + 1 << ", although only " << m_PolyLines.size() << " exists";
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
        MITK_ERROR << "Tried to add point to HelperPolyLine " << index + 1 << ", although only " << m_HelperPolyLines.size() << " exists";
    }
}

void mitk::PlanarFigure::ExecuteOperation(Operation* operation)
{
    mitkCheckOperationTypeMacro(PlanarFigureOperation, operation, planarFigureOp);

    switch (operation->GetOperationType())
    {
    case OpNOTHING:
        break;

    case OpINSERT://inserts the point at the given position and selects it.
        this->AddControlPoint(planarFigureOp->GetPoint(), planarFigureOp->GetIndex());
        break;

    case OpMOVE://moves the point given by index
        this->SetControlPoint(planarFigureOp->GetIndex(), planarFigureOp->GetPoint());
        break;

    case OpREMOVE://removes the point at given by position
        this->RemoveControlPoint(planarFigureOp->GetIndex());
        break;

    case OpSELECTPOINT://select the given point
        this->SelectControlPoint(planarFigureOp->GetIndex());
        break;

    case OpDESELECTPOINT://unselect the given point
        this->DeselectControlPoint();
        break;

    case OpADD:
        this->PlaceFigure(planarFigureOp->GetPoint());
        break;

    case OpUNDOADD:
        this->CancelPlaceFigure();
        break;

    case OpCLOSECELL:
        this->SetFinalized(true);
        break;

    case OpOPENCELL:
        this->SetFinalized(false);
        break;

    default:
        itkWarningMacro("mitkPlanarFigure could not understrand the operation. Please check!");
        break;
    }
}
bool mitk::PlanarFigure::Equals(const mitk::PlanarFigure& other) const
{
  //check geometries
  if ( this->GetPlaneGeometry() && other.GetPlaneGeometry() )
  {
    if( !Equal(*(this->GetPlaneGeometry()), *(other.GetPlaneGeometry()), mitk::eps, true))
    {
      return false;
    }
  }
  else
  {
    MITK_ERROR << "Geometry is not equal";
    return false;
  }

  //check isPlaced member
  if ( this->m_FigurePlaced != other.m_FigurePlaced)
  {
    MITK_ERROR << "Is_Placed is not equal";
    return false;
  }

  //check closed property
  if (this->IsClosed() != other.IsClosed())
  {
    MITK_ERROR << "Is_closed is not equal";
    return false;
  }

  //check poly lines
  if (this->GetPolyLinesSize() != other.GetPolyLinesSize())
  {
    return false;
  }
  else
  {
    auto itThis = this->m_PolyLines.begin();
    auto itEnd = this->m_PolyLines.end();
    auto itOther = other.m_PolyLines.begin();

    while( itThis != itEnd )
    {
      if(itThis->size() != itOther->size())
        return false;
      else
      {
        auto itLineThis = itThis->begin();
        auto itLineEnd = itThis->end();
        auto itLineOther = itOther->begin();

        while(itLineThis != itLineEnd)
        {
          Point2D p1 = *itLineThis;
          Point2D p2 = *itLineOther;
          ScalarType delta = fabs(p1[0]-p2[0])+fabs(p1[1]-p2[1]);
          if(delta > .001)
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

  //check features
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

    while(itThis != itEnd)
    {
      if(( itThis->Quantity - itOther->Quantity) > .001 )
      {
        MITK_ERROR << "Quantity is Different" << itThis->Quantity << "/" << itOther->Quantity;
        return false;
      }
      if( itThis->Unit.compare(itOther->Unit) != 0 )
      {
        MITK_ERROR << "Unit is Different" << itThis->Unit << "/" << itOther->Unit;
        return false;
      }
      if( itThis->Name.compare(itOther->Name) != 0 )
      {
        MITK_ERROR << "Name of Measure is Different " << itThis->Name << "/ " << itOther->Name;;
        return false;
      }

      ++itThis;
      ++itOther;
    }
  }

  return true;
}

bool mitk::Equal( const mitk::PlanarFigure& leftHandSide, const mitk::PlanarFigure& rightHandSide, ScalarType /*eps*/, bool /*verbose*/ )
{
  // FIXME: use eps and verbose
  return leftHandSide.Equals(rightHandSide);
}
