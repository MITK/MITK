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


#include "mitkPlaneGeometryDataToSurfaceFilter.h"
#include "mitkSurface.h"
#include "mitkGeometry3D.h"
#include "mitkPlaneGeometryData.h"
#include "mitkPlaneGeometry.h"
#include "mitkAbstractTransformGeometry.h"

#include <vtkPolyData.h>
#include <vtkPlaneSource.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtkCubeSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkPlane.h>
#include <vtkPPolyDataNormals.h>
#include <vtkCutter.h>
#include <vtkStripper.h>
#include <vtkTriangleFilter.h>
#include <vtkBox.h>
#include <vtkClipPolyData.h>
#include <vtkTextureMapToPlane.h>


mitk::PlaneGeometryDataToSurfaceFilter::PlaneGeometryDataToSurfaceFilter()
: m_UseGeometryParametricBounds( true ), m_XResolution( 10 ),
  m_YResolution( 10 ), m_PlaceByGeometry( false ), m_UseBoundingBox( false )
{
  m_PlaneSource = vtkPlaneSource::New();
  m_Transform = vtkTransform::New();

  m_CubeSource = vtkCubeSource::New();
  m_PolyDataTransformer = vtkTransformPolyDataFilter::New();

  m_Plane = vtkPlane::New();
  m_PlaneCutter = vtkCutter::New();
  m_PlaneStripper = vtkStripper::New();
  m_PlanePolyData = vtkPolyData::New();
  m_NormalsUpdater = vtkPPolyDataNormals::New();
  m_PlaneTriangler = vtkTriangleFilter::New();
  m_TextureMapToPlane = vtkTextureMapToPlane::New();

  m_Box = vtkBox::New();
  m_PlaneClipper = vtkClipPolyData::New();

  m_VtkTransformPlaneFilter = vtkTransformPolyDataFilter::New();
  m_VtkTransformPlaneFilter->SetInputConnection(m_PlaneSource->GetOutputPort() );
}


mitk::PlaneGeometryDataToSurfaceFilter::~PlaneGeometryDataToSurfaceFilter()
{
  m_PlaneSource->Delete();
  m_Transform->Delete();

  m_CubeSource->Delete();
  m_PolyDataTransformer->Delete();

  m_Plane->Delete();
  m_PlaneCutter->Delete();
  m_PlaneStripper->Delete();
  m_PlanePolyData->Delete();
  m_NormalsUpdater->Delete();
  m_PlaneTriangler->Delete();
  m_TextureMapToPlane->Delete();

  m_Box->Delete();
  m_PlaneClipper->Delete();

  m_VtkTransformPlaneFilter->Delete();
}


void mitk::PlaneGeometryDataToSurfaceFilter::GenerateOutputInformation()
{
  mitk::PlaneGeometryData::ConstPointer input = this->GetInput();
  mitk::Surface::Pointer output = this->GetOutput();

  if ( input.IsNull() || (input->GetPlaneGeometry() == nullptr)
    || (input->GetPlaneGeometry()->IsValid() == false)
    || (m_UseBoundingBox && (m_BoundingBox.IsNull() || (m_BoundingBox->GetDiagonalLength2() < mitk::eps))) )
  {
    return;
  }

  Point3D origin;
  Point3D right, bottom;

  vtkPolyData *planeSurface = nullptr;




  // Does the PlaneGeometryData contain an AbstractTransformGeometry?
  if ( mitk::AbstractTransformGeometry *abstractGeometry =
    dynamic_cast< AbstractTransformGeometry * >( input->GetPlaneGeometry() ) )
  {
    // In the case of an AbstractTransformGeometry (which holds a possibly
    // non-rigid transform), we proceed slightly differently: since the
    // plane can be arbitrarily deformed, we need to transform it by the
    // abstract transform before clipping it. The setup for this is partially
    // done in the constructor.
    origin = abstractGeometry->GetPlane()->GetOrigin();
    right = origin + abstractGeometry->GetPlane()->GetAxisVector( 0 );
    bottom = origin + abstractGeometry->GetPlane()->GetAxisVector( 1 );

    // Define the plane
    m_PlaneSource->SetOrigin( origin[0], origin[1], origin[2] );
    m_PlaneSource->SetPoint1( right[0], right[1], right[2] );
    m_PlaneSource->SetPoint2( bottom[0], bottom[1], bottom[2] );

    // Set the plane's resolution (unlike for non-deformable planes, the plane
    // grid needs to have a certain resolution so that the deformation has the
    // desired effect).
    if ( m_UseGeometryParametricBounds )
    {
      m_PlaneSource->SetXResolution(
        (int)abstractGeometry->GetParametricExtent(0)
      );
      m_PlaneSource->SetYResolution(
        (int)abstractGeometry->GetParametricExtent(1)
      );
    }
    else
    {
      m_PlaneSource->SetXResolution( m_XResolution );
      m_PlaneSource->SetYResolution( m_YResolution );
    }
    if ( m_PlaceByGeometry )
    {
      // Let the output use the input geometry to appropriately transform the
      // coordinate system.
      mitk::Geometry3D::TransformType *affineTransform =
        abstractGeometry->GetIndexToWorldTransform();

      TimeGeometry *timeGeometry = output->GetTimeGeometry();
      BaseGeometry *g3d = timeGeometry->GetGeometryForTimeStep( 0 );
      g3d->SetIndexToWorldTransform( affineTransform );

      vtkGeneralTransform *composedResliceTransform = vtkGeneralTransform::New();
      composedResliceTransform->Identity();
      composedResliceTransform->Concatenate(
        abstractGeometry->GetVtkTransform()->GetLinearInverse() );
      composedResliceTransform->Concatenate(
        abstractGeometry->GetVtkAbstractTransform()
        );
      // Use the non-rigid transform for transforming the plane.
      m_VtkTransformPlaneFilter->SetTransform(
        composedResliceTransform
      );
    }
    else
    {
      // Use the non-rigid transform for transforming the plane.
      m_VtkTransformPlaneFilter->SetTransform(
        abstractGeometry->GetVtkAbstractTransform()
      );
    }

    if ( m_UseBoundingBox )
    {
      mitk::BoundingBox::PointType boundingBoxMin = m_BoundingBox->GetMinimum();
      mitk::BoundingBox::PointType boundingBoxMax = m_BoundingBox->GetMaximum();
      //mitk::BoundingBox::PointType boundingBoxCenter = m_BoundingBox->GetCenter();

      m_Box->SetXMin( boundingBoxMin[0], boundingBoxMin[1], boundingBoxMin[2] );
      m_Box->SetXMax( boundingBoxMax[0], boundingBoxMax[1], boundingBoxMax[2] );
    }
    else
    {
      // Plane will not be clipped
      m_Box->SetXMin( -10000.0, -10000.0, -10000.0 );
      m_Box->SetXMax( 10000.0, 10000.0, 10000.0 );
    }

    m_Transform->Identity();
    m_Transform->Concatenate( input->GetPlaneGeometry()->GetVtkTransform() );
    m_Transform->PreMultiply();

    m_Box->SetTransform( m_Transform );

    m_PlaneClipper->SetInputConnection(m_VtkTransformPlaneFilter->GetOutputPort() );
    m_PlaneClipper->SetClipFunction( m_Box );
    m_PlaneClipper->GenerateClippedOutputOff(); // important to NOT generate normals data for clipped part
    m_PlaneClipper->InsideOutOn();
    m_PlaneClipper->SetValue( 0.0 );
    m_PlaneClipper->Update();

    planeSurface = m_PlaneClipper->GetOutput();
  }
    // Does the PlaneGeometryData contain a PlaneGeometry?
  else if ( dynamic_cast< PlaneGeometry * >( input->GetPlaneGeometry() ) != nullptr )
  {
    mitk::PlaneGeometry *planeGeometry =
      dynamic_cast< PlaneGeometry * >( input->GetPlaneGeometry() );

    if ( m_PlaceByGeometry )
    {
      // Let the output use the input geometry to appropriately transform the
      // coordinate system.
      mitk::Geometry3D::TransformType *affineTransform =
        planeGeometry->GetIndexToWorldTransform();

      TimeGeometry *timeGeometry = output->GetTimeGeometry();
      BaseGeometry *geometrie3d = timeGeometry->GetGeometryForTimeStep( 0 );
      geometrie3d->SetIndexToWorldTransform( affineTransform );
    }

    if ( !m_UseBoundingBox)
    {
      // We do not have a bounding box, so no clipping is required.

      if ( m_PlaceByGeometry )
      {
        // Derive coordinate axes and origin from input geometry extent
        origin.Fill( 0.0 );
        FillVector3D( right,  planeGeometry->GetExtent(0), 0.0, 0.0 );
        FillVector3D( bottom, 0.0, planeGeometry->GetExtent(1), 0.0 );
      }
      else
      {
        // Take the coordinate axes and origin directly from the input geometry.
        origin = planeGeometry->GetOrigin();
        right = planeGeometry->GetCornerPoint( false, true );
        bottom = planeGeometry->GetCornerPoint( true, false );
      }

      // Since the plane is planar, there is no need to subdivide the grid
      // (cf. AbstractTransformGeometry case)
      m_PlaneSource->SetXResolution( 1 );
      m_PlaneSource->SetYResolution( 1 );

      m_PlaneSource->SetOrigin( origin[0], origin[1], origin[2] );
      m_PlaneSource->SetPoint1( right[0], right[1], right[2] );
      m_PlaneSource->SetPoint2( bottom[0], bottom[1], bottom[2] );

      m_PlaneSource->Update();
      planeSurface = m_PlaneSource->GetOutput();

    }
    else
    {
      // Set up a cube with the extent and origin of the bounding box. This
      // cube will be clipped by a plane later on. The intersection of the
      // cube and the plane will be the surface we are interested in. Note
      // that the bounding box needs to be explicitly specified by the user
      // of this class, since it is not necessarily clear from the data
      // available herein which bounding box to use. In most cases, this
      // would be the bounding box of the input geometry's reference
      // geometry, but this is not an inevitable requirement.
      mitk::BoundingBox::PointType boundingBoxMin = m_BoundingBox->GetMinimum();
      mitk::BoundingBox::PointType boundingBoxMax = m_BoundingBox->GetMaximum();
      mitk::BoundingBox::PointType boundingBoxCenter = m_BoundingBox->GetCenter();

      m_CubeSource->SetXLength( boundingBoxMax[0] - boundingBoxMin[0] );
      m_CubeSource->SetYLength( boundingBoxMax[1] - boundingBoxMin[1] );
      m_CubeSource->SetZLength( boundingBoxMax[2] - boundingBoxMin[2] );
      m_CubeSource->SetCenter(
        boundingBoxCenter[0],
        boundingBoxCenter[1],
        boundingBoxCenter[2] );


      // Now we have to transform the cube, so that it will cut our plane
      // appropriately. (As can be seen below, the plane corresponds to the
      // z-plane in the coordinate system and is *not* transformed.) Therefore,
      // we get the inverse of the plane geometry's transform and concatenate
      // it with the transform of the reference geometry, if available.
      m_Transform->Identity();
      m_Transform->Concatenate(
        planeGeometry->GetVtkTransform()->GetLinearInverse()
      );

      BaseGeometry *referenceGeometry = planeGeometry->GetReferenceGeometry();
      if ( referenceGeometry )
      {
        m_Transform->Concatenate(
          referenceGeometry->GetVtkTransform()
        );
      }

      // Transform the cube accordingly (s.a.)
      m_PolyDataTransformer->SetInputConnection( m_CubeSource->GetOutputPort() );
      m_PolyDataTransformer->SetTransform( m_Transform );

      // Initialize the plane to clip the cube with, as lying on the z-plane
      m_Plane->SetOrigin( 0.0, 0.0, 0.0 );
      m_Plane->SetNormal( 0.0, 0.0, 1.0 );

      // Cut the plane with the cube.
      m_PlaneCutter->SetInputConnection( m_PolyDataTransformer->GetOutputPort() );
      m_PlaneCutter->SetCutFunction( m_Plane );

      // The output of the cutter must be converted into appropriate poly data.
      m_PlaneStripper->SetInputConnection( m_PlaneCutter->GetOutputPort() );
      m_PlaneStripper->Update();

      if ( m_PlaneStripper->GetOutput()->GetNumberOfPoints() < 3 )
      {
        return;
      }

      m_PlanePolyData->SetPoints( m_PlaneStripper->GetOutput()->GetPoints() );
      m_PlanePolyData->SetPolys( m_PlaneStripper->GetOutput()->GetLines() );

      m_PlaneTriangler->SetInputData( m_PlanePolyData );


      // Get bounds of the resulting surface and use it to generate the texture
      // mapping information
      m_PlaneTriangler->Update();
      m_PlaneTriangler->GetOutput()->ComputeBounds();
      double *surfaceBounds =
        m_PlaneTriangler->GetOutput()->GetBounds();

      origin[0] = surfaceBounds[0];
      origin[1] = surfaceBounds[2];
      origin[2] = surfaceBounds[4];

      right[0] = surfaceBounds[1];
      right[1] = surfaceBounds[2];
      right[2] = surfaceBounds[4];

      bottom[0] = surfaceBounds[0];
      bottom[1] = surfaceBounds[3];
      bottom[2] = surfaceBounds[4];

      // Now we tell the data how it shall be textured afterwards;
      // description see above.
      m_TextureMapToPlane->SetInputConnection( m_PlaneTriangler->GetOutputPort() );
      m_TextureMapToPlane->AutomaticPlaneGenerationOn();
      m_TextureMapToPlane->SetOrigin( origin[0], origin[1], origin[2] );
      m_TextureMapToPlane->SetPoint1( right[0], right[1], right[2] );
      m_TextureMapToPlane->SetPoint2( bottom[0], bottom[1], bottom[2] );

      // Need to call update so that output data and bounds are immediately
      // available
      m_TextureMapToPlane->Update();


      // Return the output of this generation process
      planeSurface = dynamic_cast< vtkPolyData * >(
        m_TextureMapToPlane->GetOutput()
      );
    }
  }

  m_NormalsUpdater->SetInputData( planeSurface );
  m_NormalsUpdater->AutoOrientNormalsOn(); // that's the trick! Brings consistency between
                                          //  normals direction and front/back faces direction (see bug 1440)
  m_NormalsUpdater->ComputePointNormalsOn();
  m_NormalsUpdater->Update();

  output->SetVtkPolyData( m_NormalsUpdater->GetOutput() );
  output->CalculateBoundingBox();
}


void mitk::PlaneGeometryDataToSurfaceFilter::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();

  if (output.IsNull()) return;
  if (output->GetVtkPolyData()==nullptr) return;

//  output->GetVtkPolyData()->Update(); //VTK6_TODO vtk pipeline
}

const mitk::PlaneGeometryData *mitk::PlaneGeometryDataToSurfaceFilter::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
  {
    return nullptr;
  }

  return static_cast<const mitk::PlaneGeometryData * >
    ( this->ProcessObject::GetInput(0) );
}


const mitk::PlaneGeometryData *
mitk::PlaneGeometryDataToSurfaceFilter
::GetInput(unsigned int idx)
{
  return static_cast< const mitk::PlaneGeometryData * >
    ( this->ProcessObject::GetInput(idx) );
}


void
mitk::PlaneGeometryDataToSurfaceFilter
::SetInput(const mitk::PlaneGeometryData *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 0,
  const_cast< mitk::PlaneGeometryData * >( input )
  );
}


void
mitk::PlaneGeometryDataToSurfaceFilter
::SetInput(unsigned int index, const mitk::PlaneGeometryData *input)
{
  if( index+1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( index + 1 );
  }
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index,
    const_cast< mitk::PlaneGeometryData *>( input )
  );
}


void
mitk::PlaneGeometryDataToSurfaceFilter
::SetBoundingBox( const mitk::BoundingBox *boundingBox )
{
  m_BoundingBox = boundingBox;
  this->UseBoundingBoxOn();
}


const mitk::BoundingBox *
mitk::PlaneGeometryDataToSurfaceFilter
::GetBoundingBox() const
{
  return m_BoundingBox.GetPointer();
}
