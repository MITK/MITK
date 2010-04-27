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


void mitk::PlanarFigure::PlaceFigure( const mitk::Point2D& point )
{
  for ( unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    m_ControlPoints->InsertElement( i, this->ApplyControlPointConstraints( i, point ) );
  }

  m_FigurePlaced = true;
  m_SelectedControlPoint = 1;
}


bool mitk::PlanarFigure::AddControlPoint( const mitk::Point2D& point )
{
  if ( m_NumberOfControlPoints < this->GetMaximumNumberOfControlPoints() )
  {
    m_ControlPoints->InsertElement( m_NumberOfControlPoints, 
      this->ApplyControlPointConstraints( m_NumberOfControlPoints, point ) );
    ++m_NumberOfControlPoints;
    ++m_SelectedControlPoint;
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::PlanarFigure::SetControlPoint( unsigned int index, const Point2D& point, bool createIfDoesNotExist )
{
  if (createIfDoesNotExist)
  {
    m_ControlPoints->InsertElement( index, this->ApplyControlPointConstraints( index, point ) );
    if ( m_NumberOfControlPoints <= index )
    {
      m_NumberOfControlPoints = index + 1;
    }   
    return true;
  }
  else if ( index < m_NumberOfControlPoints )
  {
    m_ControlPoints->InsertElement( index, this->ApplyControlPointConstraints( index, point ) );
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::PlanarFigure::SetCurrentControlPoint( const Point2D& point )
{
  if ( (m_SelectedControlPoint < 0) || (m_SelectedControlPoint >= (int)m_NumberOfControlPoints) )
  {
    return false;
  }

  return this->SetControlPoint(m_SelectedControlPoint, point, false);
}


unsigned int mitk::PlanarFigure::GetNumberOfControlPoints() const
{
  return m_NumberOfControlPoints;
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


mitk::Point2D& mitk::PlanarFigure::GetControlPoint( unsigned int index ) const
{
  if ( index < m_NumberOfControlPoints )
  {
    return m_ControlPoints->ElementAt( index );
  }

  itkExceptionMacro( << "GetControlPoint(): Invalid index!" );
}


mitk::Point3D mitk::PlanarFigure::GetWorldControlPoint( unsigned int index ) const
{
  Point3D point3D;
  if ( (m_Geometry2D != NULL) && (index < m_NumberOfControlPoints) )
  {
    m_Geometry2D->Map( m_ControlPoints->ElementAt( index ), point3D );
    return point3D;
  }

  itkExceptionMacro( << "GetWorldControlPoint(): Invalid index!" );
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


double mitk::PlanarFigure::GetQuantity( unsigned int index ) const
{
  if ( index < m_Features.size() )
  {
    return m_Features[index].Quantity;
  }
  else
  {
    return 0.0;
  }
}


bool mitk::PlanarFigure::IsFeatureActive( unsigned int index ) const
{
  if ( index < m_Features.size() )
  {
    return m_Features[index].Active;
  }
  else
  {
    return false;
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


void mitk::PlanarFigure::ResetNumberOfControlPoints( int numberOfControlPoints )
{
  m_NumberOfControlPoints = numberOfControlPoints;
}


mitk::Point2D mitk::PlanarFigure::ApplyControlPointConstraints( unsigned int index, const Point2D& point )
{
  if ( m_Geometry2D ==  NULL )
  {
    return point;
  }

  Point2D indexPoint;
  m_Geometry2D->WorldToIndex( point, indexPoint );

  BoundingBox::BoundsArrayType bounds = m_Geometry2D->GetBounds();
  if ( indexPoint[0] < bounds[0] ) { indexPoint[0] = bounds[0]; }
  if ( indexPoint[0] > bounds[1] ) { indexPoint[0] = bounds[1]; }
  if ( indexPoint[1] < bounds[2] ) { indexPoint[1] = bounds[2]; }
  if ( indexPoint[1] > bounds[3] ) { indexPoint[1] = bounds[3]; }
  
  Point2D constrainedPoint;
  m_Geometry2D->IndexToWorld( indexPoint, constrainedPoint );

  return constrainedPoint;
}


unsigned int mitk::PlanarFigure::AddFeature( const char *featureName, const char *unitName )
{
  unsigned int index = m_Features.size();
  
  Feature newFeature( featureName, unitName );
  m_Features.push_back( newFeature );

  return index;
}


void mitk::PlanarFigure::SetFeatureName( unsigned int index, const char *featureName )
{
  if ( index < m_Features.size() )
  {
    m_Features[index].Name = featureName;
  }
}


void mitk::PlanarFigure::SetFeatureUnit( unsigned int index, const char *unitName )
{
  if ( index < m_Features.size() )
  {
    m_Features[index].Unit = unitName;
  }
}


void mitk::PlanarFigure::SetQuantity( unsigned int index, double quantity )
{
  if ( index < m_Features.size() )
  {
    m_Features[index].Quantity = quantity;
  }
}


void mitk::PlanarFigure::ActivateFeature( unsigned int index )
{
  if ( index < m_Features.size() )
  {
    m_Features[index].Active = true;
  }
}


void mitk::PlanarFigure::DeactivateFeature( unsigned int index )
{
  if ( index < m_Features.size() )
  {
    m_Features[index].Active = false;
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
  for ( unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    os << indent.GetNextIndent() << i << ": " << m_ControlPoints->ElementAt( i ) << std::endl;
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


bool mitk::PlanarFigure::ResetOnPointSelect()
{
  return false;
}
