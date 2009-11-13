/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-08 11:04:08 +0200 (Mi, 08 Jul 2009) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigure.h"
#include "mitkGeometry2D.h"


mitk::PlanarFigure::PlanarFigure()
: m_FigurePlaced( false ),
  m_SelectedControlPoint( -1 ),
  m_Geometry2D( NULL ),
  m_FeaturesMTime( 0 )
{
  m_ControlPoints = VertexContainerType::New();
  m_PolyLines = VertexContainerVectorType::New();
  m_HelperPolyLines = VertexContainerVectorType::New();
  m_HelperPolyLinesToBePainted = BoolContainerType::New();

  // Currently only single-time-step geometries are supported
  this->InitializeTimeSlicedGeometry( 1 );
}


mitk::PlanarFigure::~PlanarFigure()
{
}


void mitk::PlanarFigure::SetGeometry2D( mitk::Geometry2D *geometry )
{
  this->SetGeometry( geometry );
  m_Geometry2D = geometry;
}


const mitk::Geometry2D *mitk::PlanarFigure::GetGeometry2D() const
{
  return m_Geometry2D;
}


void mitk::PlanarFigure::PlaceFigure( const mitk::Point2D &point )
{
  VertexContainerType::Iterator it;
  for ( it = m_ControlPoints->Begin(); it != m_ControlPoints->End(); ++it )
  {
    it->Value() = point;
  }

  m_FigurePlaced = true;
  m_SelectedControlPoint = 1;
}


bool mitk::PlanarFigure::AddControlPoint( const mitk::Point2D &point )
{
  int currentNumberOfControlPoints = m_ControlPoints->Size();

  LOG_INFO << "AddControlPoint()";
  LOG_INFO << "currentNumberOfControlPoints: " << currentNumberOfControlPoints;
  LOG_INFO << "maxNumber: " << this->GetMaximumNumberOfControlPoints();
  LOG_INFO << "selected point: " << m_SelectedControlPoint;
  if ( currentNumberOfControlPoints < this->GetMaximumNumberOfControlPoints() )
  {
    m_ControlPoints->InsertElement( currentNumberOfControlPoints, point );
    ++m_SelectedControlPoint;
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::PlanarFigure::SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist )
{
  if (createIfDoesNotExist)
  {
    m_ControlPoints->CreateIndex(index);
    m_ControlPoints->CreateElementAt(index) = point;
    return true;
  }
  else if ( index < m_ControlPoints->Size() )
  {
    m_ControlPoints->ElementAt( index ) = point;
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::PlanarFigure::SetCurrentControlPoint( const Point2D &point )
{
  if ( (m_SelectedControlPoint < 0) || (m_SelectedControlPoint >= m_ControlPoints->Size()) )
  {
    return false;
  }

  m_ControlPoints->ElementAt( m_SelectedControlPoint ) = point;
  return true;
}


unsigned int mitk::PlanarFigure::GetNumberOfControlPoints() const
{
  return m_ControlPoints->Size();
}


bool mitk::PlanarFigure::SelectControlPoint( unsigned int index )
{
  if ( index < this->GetNumberOfControlPoints() )
  {
    m_SelectedControlPoint = index;
    return true;
  }
  else
  {
    return false;
  }
}


void mitk::PlanarFigure::DeselectControlPoint()
{
  m_SelectedControlPoint = -1;
}


const mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetControlPoints() const
{
  return m_ControlPoints;
}


mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetControlPoints()
{
  return m_ControlPoints;
}


mitk::Point2D mitk::PlanarFigure::GetControlPoint( unsigned int index ) const
{
  Point2D point2D;

  if ( index < m_ControlPoints->Size() )
  {
    point2D = m_ControlPoints->ElementAt( index );
  }
  else
  {
    point2D.Fill( 0.0 );
  }

  return point2D;
}


mitk::Point3D mitk::PlanarFigure::GetWorldControlPoint( unsigned int index ) const
{
  Point3D point3D;

  if ( (m_Geometry2D != NULL) && (index < m_ControlPoints->Size()) )
  {
    Point2D point2D;
    m_Geometry2D->IndexToWorld( m_ControlPoints->ElementAt( index ), point2D );
    m_Geometry2D->Map( point2D, point3D );
  }
  else
  {
    point3D.Fill( 0.0 );
  }

  return point3D;
}


mitk::Point2D mitk::PlanarFigure::GetWorldControlPoint2D( unsigned int index ) const
{
  Point2D point2D;

  if ( (m_Geometry2D != NULL) && (index < m_ControlPoints->Size()) )
  {
    m_Geometry2D->IndexToWorld( m_ControlPoints->ElementAt( index ), point2D );
  }
  else
  {
    point2D.Fill( 0.0 );
  }

  return point2D;
}


mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetPolyLine(unsigned int index)
{
  if ((m_PolyLines->ElementAt( index )) && (m_PolyLines->ElementAt( index )->GetMTime() < m_ControlPoints->GetMTime()) )
  {
    this->GeneratePolyLine();
  }

  return m_PolyLines->ElementAt( index );
}


const mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetPolyLine(unsigned int index) const
{
  return m_PolyLines->ElementAt( index );
}


const mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetHelperPolyLine(unsigned int index, double mmPerDisplayUnit, unsigned int displayHeight)
{
  if ((m_HelperPolyLines->ElementAt( index )) && (m_HelperPolyLines->ElementAt( index )->GetMTime() < m_ControlPoints->GetMTime()) )
  {
    this->GenerateHelperPolyLine(mmPerDisplayUnit, displayHeight);
  }

  return m_HelperPolyLines->ElementAt( index );
}

/** \brief Returns the number of features available for this PlanarFigure
* (such as, radius, area, ...). */
unsigned int mitk::PlanarFigure::GetNumberOfFeatures() const
{
  return m_Features.size();
}


/** \brief Returns the name (identifier) of the specified features. */
const char *mitk::PlanarFigure::GetFeatureName( unsigned int index ) const
{
  if ( index < m_Features.size() )
  {
    return m_Features[index].Name.c_str();
  }
  else
  {
    return NULL;
  }
}


/** \brief Returns the physical unit of the specified features. */
const char *mitk::PlanarFigure::GetFeatureUnit( unsigned int index ) const
{
  if ( index < m_Features.size() )
  {
    return m_Features[index].Unit.c_str();
  }
  else
  {
    return NULL;
  }
}


/** Returns quantity of the specified feature (e.g., length, radius,
* area, ... ) */
double mitk::PlanarFigure::GetQuantity( unsigned int index ) const
{
  if ( index < m_Features.size() )
  {
    return m_Features[index].Quantity;
  }
  else
  {
    return NULL;
  }
}


void mitk::PlanarFigure::EvaluateFeatures()
{
  if ( m_FeaturesMTime < m_ControlPoints->GetMTime() )
  {
    this->EvaluateFeaturesInternal();

    m_FeaturesMTime = m_ControlPoints->GetMTime();
  }
}


void mitk::PlanarFigure::UpdateOutputInformation()
{
  // Bounds are NOT calculated here, since the Geometry2D defines a fixed
  // frame (= bounds) for the planar figure.
  Superclass::UpdateOutputInformation();
  this->GetTimeSlicedGeometry()->UpdateInformation();
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


void mitk::PlanarFigure::SetRequestedRegion( itk::DataObject * /*data*/ )
{

}


unsigned int mitk::PlanarFigure::AddFeature( const char *featureName, const char *unitName )
{
  unsigned int index = m_Features.size();
  
  Feature newFeature( featureName, unitName );
  m_Features.push_back( newFeature );

  return index;
}


void mitk::PlanarFigure::SetQuantity( unsigned int index, double quantity )
{
  if ( index < m_Features.size() )
  {
    m_Features[index].Quantity = quantity;
  }
}


void mitk::PlanarFigure::InitializeTimeSlicedGeometry( unsigned int timeSteps )
{
  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();

  mitk::Geometry2D::Pointer geometry2D = mitk::Geometry2D::New();
  geometry2D->Initialize();

  if ( timeSteps > 1 )
  {
    mitk::ScalarType timeBounds[] = {0.0, 1.0};
    geometry2D->SetTimeBounds( timeBounds );
  }

  // The geometry is propagated automatically to all time steps,
  // if EvenlyTimed is true...
  timeGeometry->InitializeEvenlyTimed( geometry2D, timeSteps );
}


void mitk::PlanarFigure::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << this->GetNameOfClass() << ":\n";
  
  if (this->IsClosed())
    os << indent << "This figure is closed\n";
  else
    os << indent << "This figure is not closed\n";
  os << indent << "Minimum number of control points: " << this->GetMinimumNumberOfControlPoints() << std::endl;
  os << indent << "Maximum number of control points: " << this->GetMaximumNumberOfControlPoints() << std::endl;
  os << indent << "Current number of control points: " << this->GetNumberOfControlPoints() << std::endl;
  os << indent << "Control points:" << std::endl;
  mitk::PlanarFigure::VertexContainerType::ConstIterator it;  
  unsigned int i;
  for ( it = m_ControlPoints->Begin(), i = 0;
        it != m_ControlPoints->End();
        ++it, ++i )
  {
    os << indent.GetNextIndent() << i << ": " << it.Value() << std::endl;
  }
  os << indent << "Geometry:\n";
  this->GetGeometry2D()->Print(os, indent.GetNextIndent());
}


unsigned short mitk::PlanarFigure::GetPolyLinesSize()
{
  return m_PolyLines->size();
}

unsigned short mitk::PlanarFigure::GetHelperPolyLinesSize()
{
  return m_HelperPolyLines->size();
}

bool mitk::PlanarFigure::IsHelperToBePainted(unsigned int index)
{
  return m_HelperPolyLinesToBePainted->GetElement( index );
}
