/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkPointSetToCurvedGeometryFilter.h"
#include "mitkThinPlateSplineCurvedGeometry.h"
#include "mitkPlaneGeometry.h"
#include "mitkImage.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkDataTreeNode.h"
#include "mitkGeometryData.h"
#include "mitkGeometry2DData.h"
#include "mitkProperties.h"
#include "itkMesh.h"
#include "itkPointSet.h"

mitk::PointSetToCurvedGeometryFilter::PointSetToCurvedGeometryFilter()
{
  m_ProjectionMode = YZPlane;
  m_PCAPlaneCalculator = mitk::PlaneFit::New();
  m_ImageToBeMapped = NULL;
  m_Sigma = 1000;
  mitk::Geometry2DData::Pointer output = static_cast<mitk::Geometry2DData*> ( this->MakeOutput ( 0 ).GetPointer() );
  output->Initialize();
  Superclass::SetNumberOfRequiredOutputs ( 1 );
  Superclass::SetNthOutput ( 0, output.GetPointer() );
}



mitk::PointSetToCurvedGeometryFilter::~PointSetToCurvedGeometryFilter()
{}

void mitk::PointSetToCurvedGeometryFilter::GenerateOutputInformation()
{
  mitk::PointSet::ConstPointer input  = this->GetInput();
  mitk::Geometry2DData::Pointer output  = dynamic_cast<mitk::Geometry2DData*> ( this->GetOutput() );

  if ( input.IsNull() )
    itkGenericExceptionMacro ( "Input point set is NULL!" );

  if ( input->GetTimeSlicedGeometry()->GetTimeSteps() != 1 )
    itkWarningMacro ( "More than one time step is not yet supported!" );

  if ( output.IsNull() )
    itkGenericExceptionMacro ( "Output is NULL!" );
  
  if ( m_ImageToBeMapped.IsNull() )
    itkGenericExceptionMacro ( "Image to be mapped is NULL!" );

  bool update = false;
  if ( output->GetGeometry() == NULL || output->GetGeometry2D() == NULL || output->GetTimeSlicedGeometry() == NULL )
    update = true;
  if ( ( ! update ) && ( output->GetTimeSlicedGeometry()->GetTimeSteps() != input->GetTimeSlicedGeometry()->GetTimeSteps() ) )
    update = true;
  if ( update )
  {
    mitk::ThinPlateSplineCurvedGeometry::Pointer curvedGeometry = mitk::ThinPlateSplineCurvedGeometry::New();
    output->SetGeometry(curvedGeometry);
    
    /*
    mitk::TimeSlicedGeometry::Pointer timeGeometry = mitk::TimeSlicedGeometry::New();
    mitk::ThinPlateSplineCurvedGeometry::Pointer curvedGeometry = mitk::ThinPlateSplineCurvedGeometry::New();

    timeGeometry->InitializeEvenlyTimed ( curvedGeometry, input->GetPointSetSeriesSize() );

    for ( unsigned int t = 1; t < input->GetPointSetSeriesSize(); ++t )
    {
      mitk::ThinPlateSplineCurvedGeometry::Pointer tmpCurvedGeometry = mitk::ThinPlateSplineCurvedGeometry::New();
      timeGeometry->SetGeometry3D ( tmpCurvedGeometry.GetPointer(), t );
    }
    output->SetGeometry ( timeGeometry );
    output->SetGeometry2D ( curvedGeometry ); // @FIXME SetGeometry2D of mitk::Geometry2DData reinitializes the TimeSlicedGeometry to 1 time step
    */
  }
}

void mitk::PointSetToCurvedGeometryFilter::GenerateData()
{
  mitk::PointSet::ConstPointer input  = this->GetInput();
  mitk::GeometryData::Pointer output  = this->GetOutput();

  //
  // check preconditions
  //
  if ( input.IsNull() )
    itkGenericExceptionMacro ( "Input point set is NULL!" );
  if ( output.IsNull() )
    itkGenericExceptionMacro ( "output geometry data is NULL!" );
  if ( output->GetTimeSlicedGeometry() == NULL )
    itkGenericExceptionMacro ( "Output time sliced geometry is NULL!" );
  if ( output->GetTimeSlicedGeometry()->GetGeometry3D ( 0 ) == NULL )
    itkGenericExceptionMacro ( "Output geometry3d is NULL!" );
  mitk::ThinPlateSplineCurvedGeometry::Pointer curvedGeometry = dynamic_cast<mitk::ThinPlateSplineCurvedGeometry*> ( output->GetTimeSlicedGeometry()->GetGeometry3D ( 0 ) );
  if ( curvedGeometry.IsNull() )
    itkGenericExceptionMacro ( "Output geometry3d is not an instance of mitk::ThinPlateSPlineCurvedGeometry!" );
  if ( m_ImageToBeMapped.IsNull() )
    itkGenericExceptionMacro ( "Image to be mapped is NULL!" );

  //
  // initialize members if needed
  //
  if ( m_XYPlane.IsNull()  ||  m_XZPlane.IsNull() || m_YZPlane.IsNull() )
  {
    m_ImageToBeMapped->UpdateOutputInformation();
    const mitk::Geometry3D* imageGeometry = m_ImageToBeMapped->GetUpdatedGeometry();
    imageGeometry = m_ImageToBeMapped->GetUpdatedGeometry();
    m_XYPlane = mitk::PlaneGeometry::New();
    m_XZPlane = mitk::PlaneGeometry::New();
    m_YZPlane = mitk::PlaneGeometry::New();
    m_XYPlane->InitializeStandardPlane ( imageGeometry, mitk::PlaneGeometry::Transversal );
    m_YZPlane->InitializeStandardPlane ( imageGeometry, mitk::PlaneGeometry::Sagittal );
    m_XZPlane->InitializeStandardPlane ( imageGeometry, mitk::PlaneGeometry::Frontal );
  }
  if ( m_PlaneLandmarkProjector.IsNull() )
  {
    m_PlaneLandmarkProjector = mitk::PlaneLandmarkProjector::New();
    m_SphereLandmarkProjector = mitk::SphereLandmarkProjector::New();
  }

  //
  // set up geometry according to the current settings
  //
  if ( m_ProjectionMode == Sphere )
  {
    curvedGeometry->SetLandmarkProjector ( m_SphereLandmarkProjector );
  }
  else
  {
    if ( m_ProjectionMode == XYPlane )
      m_PlaneLandmarkProjector->SetProjectionPlane ( m_XYPlane );
    else if ( m_ProjectionMode == XZPlane )
      m_PlaneLandmarkProjector->SetProjectionPlane ( m_XZPlane );
    else if ( m_ProjectionMode == YZPlane )
      m_PlaneLandmarkProjector->SetProjectionPlane ( m_YZPlane );
    else if ( m_ProjectionMode == PCAPlane )
    {
      itkExceptionMacro ( "PCAPlane not yet implemented!" );
      m_PCAPlaneCalculator->SetInput ( input );
      m_PCAPlaneCalculator->Update();
      m_PlaneLandmarkProjector->SetProjectionPlane ( dynamic_cast<mitk::PlaneGeometry*> ( m_PCAPlaneCalculator->GetOutput() ) );
    }
    else
      itkExceptionMacro ( "Unknown projection mode" );

    curvedGeometry->SetLandmarkProjector ( m_PlaneLandmarkProjector );
  }
  //curvedGeometry->SetReferenceGeometry( m_ImageToBeMapped->GetGeometry() );
  curvedGeometry->SetTargetLandmarks ( input->GetPointSet ( 0 )->GetPoints() );
  curvedGeometry->SetSigma ( m_Sigma );
  curvedGeometry->ComputeGeometry();
  curvedGeometry->SetOversampling ( 1.0 );

}

mitk::GeometryDataSource::DataObjectPointer mitk::PointSetToCurvedGeometryFilter::MakeOutput ( unsigned int )
{
  return static_cast<itk::DataObject*> ( mitk::Geometry2DData::New().GetPointer() );
}


void mitk::PointSetToCurvedGeometryFilter::SetDefaultCurvedGeometryProperties ( mitk::DataTreeNode* node )
{
  if ( node == NULL )
  {
    itkGenericOutputMacro ( "Warning: node is NULL!" );
    return;
  }
  node->SetIntProperty ( "xresolution", 50 );
  node->SetIntProperty ( "yresolution", 50 );
  node->SetProperty ( "name", new mitk::StringProperty ( "Curved Plane" ) );
  // exclude extent of this plane when calculating DataTree bounding box
  node->SetProperty ( "includeInBoundingBox", new mitk::BoolProperty ( false ) );
}
