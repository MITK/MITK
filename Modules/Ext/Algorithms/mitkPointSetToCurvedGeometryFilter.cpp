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

#include "mitkPointSetToCurvedGeometryFilter.h"
#include "mitkThinPlateSplineCurvedGeometry.h"
#include "mitkPlaneGeometry.h"
#include "mitkImage.h"
#include "mitkDataNode.h"
#include "mitkGeometryData.h"
#include "mitkPlaneGeometryData.h"
#include "mitkProperties.h"
#include "itkMesh.h"
#include "itkPointSet.h"

mitk::PointSetToCurvedGeometryFilter::PointSetToCurvedGeometryFilter()
{
  m_ProjectionMode = YZPlane;
  m_PCAPlaneCalculator = mitk::PlaneFit::New();
  m_ImageToBeMapped = NULL;
  m_Sigma = 1000;
  mitk::PlaneGeometryData::Pointer output = static_cast<mitk::PlaneGeometryData*> ( this->MakeOutput ( 0 ).GetPointer() );
  output->Initialize();
  Superclass::SetNumberOfRequiredOutputs ( 1 );
  Superclass::SetNthOutput ( 0, output.GetPointer() );
}



mitk::PointSetToCurvedGeometryFilter::~PointSetToCurvedGeometryFilter()
{}

void mitk::PointSetToCurvedGeometryFilter::GenerateOutputInformation()
{
  mitk::PointSet::ConstPointer input  = this->GetInput();
  mitk::PlaneGeometryData::Pointer output  = dynamic_cast<mitk::PlaneGeometryData*> ( this->GetOutput() );

  if ( input.IsNull() )
    itkGenericExceptionMacro ( "Input point set is NULL!" );

  if ( input->GetTimeGeometry()->CountTimeSteps() != 1 )
    itkWarningMacro ( "More than one time step is not yet supported!" );

  if ( output.IsNull() )
    itkGenericExceptionMacro ( "Output is NULL!" );

  if ( m_ImageToBeMapped.IsNull() )
    itkGenericExceptionMacro ( "Image to be mapped is NULL!" );

  bool update = false;
  if ( output->GetGeometry() == NULL || output->GetPlaneGeometry() == NULL || output->GetTimeGeometry() == NULL )
    update = true;
  if ( ( ! update ) && ( output->GetTimeGeometry()->CountTimeSteps() != input->GetTimeGeometry()->CountTimeSteps() ) )
    update = true;
  if ( update )
  {
    mitk::ThinPlateSplineCurvedGeometry::Pointer curvedGeometry = mitk::ThinPlateSplineCurvedGeometry::New();
    output->SetGeometry(curvedGeometry);
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
  if ( output->GetTimeGeometry() == NULL )
    itkGenericExceptionMacro ( "Output time sliced geometry is NULL!" );
  if ( output->GetTimeGeometry()->GetGeometryForTimeStep ( 0 ).IsNull() )
    itkGenericExceptionMacro ( "Output geometry3d is NULL!" );
  mitk::ThinPlateSplineCurvedGeometry::Pointer curvedGeometry = dynamic_cast<mitk::ThinPlateSplineCurvedGeometry*> ( output->GetTimeGeometry()->GetGeometryForTimeStep( 0 ).GetPointer() );
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
    const mitk::BaseGeometry* imageGeometry = m_ImageToBeMapped->GetUpdatedGeometry();
    imageGeometry = m_ImageToBeMapped->GetUpdatedGeometry();
    m_XYPlane = mitk::PlaneGeometry::New();
    m_XZPlane = mitk::PlaneGeometry::New();
    m_YZPlane = mitk::PlaneGeometry::New();
    m_XYPlane->InitializeStandardPlane ( imageGeometry, mitk::PlaneGeometry::Axial );
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


void mitk::PointSetToCurvedGeometryFilter::SetDefaultCurvedGeometryProperties ( mitk::DataNode* node )
{
  if ( node == NULL )
  {
    itkGenericOutputMacro ( "Warning: node is NULL!" );
    return;
  }
  node->SetIntProperty ( "xresolution", 50 );
  node->SetIntProperty ( "yresolution", 50 );
  node->SetProperty ( "name", mitk::StringProperty::New ( "Curved Plane" ) );
  // exclude extent of this plane when calculating DataStorage bounding box
  node->SetProperty ( "includeInBoundingBox", mitk::BoolProperty::New ( false ) );
}
