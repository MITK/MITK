/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkMeshVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkOpenGLRenderer.h"

#ifndef VCL_VC60
#include "mitkMeshUtil.h"
#endif

#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>
#include <vtkPropAssembly.h>


#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <stdlib.h>

const mitk::Mesh* mitk::MeshVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Mesh * > ( GetData() );
}

vtkProp* mitk::MeshVtkMapper3D::GetProp()
{
  return m_PropAssembly;
}

void mitk::MeshVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer* renderer)
{
  vtkLinearTransform * vtktransform = 
    this->GetDataTreeNode()->GetVtkTransform(
      renderer->GetTimeStep(this->GetDataTreeNode()->GetData()));

  m_SpheresActor->SetUserTransform(vtktransform);
  m_ContourActor->SetUserTransform(vtktransform);
}

mitk::MeshVtkMapper3D::MeshVtkMapper3D()
: m_PropAssembly(NULL)
{
  m_Spheres = vtkAppendPolyData::New();
  m_Contour = vtkPolyData::New();

  m_SpheresActor = vtkActor::New();
  m_SpheresMapper = vtkPolyDataMapper::New();
  m_SpheresActor->SetMapper(m_SpheresMapper);

  m_ContourActor = vtkActor::New();
  m_ContourMapper = vtkPolyDataMapper::New();
  m_ContourActor->SetMapper(m_ContourMapper);
  m_ContourActor->GetProperty()->SetAmbient(1.0);

  m_PropAssembly = vtkPropAssembly::New();

  // a vtkPropAssembly is not a sub-class of vtkProp3D, so
  // we cannot use m_Prop3D.
  m_Prop3D = NULL;
}

mitk::MeshVtkMapper3D::~MeshVtkMapper3D()
{
  m_ContourActor->Delete();
  m_SpheresActor->Delete();
  m_ContourMapper->Delete();
  m_SpheresMapper->Delete();
  m_PropAssembly->Delete();
  m_Spheres->Delete();
  m_Contour->Delete();
}

void mitk::MeshVtkMapper3D::GenerateData()
{
  m_PropAssembly->VisibilityOn();

  if(m_PropAssembly->GetParts()->IsItemPresent(m_SpheresActor))
    m_PropAssembly->RemovePart(m_SpheresActor);
  if(m_PropAssembly->GetParts()->IsItemPresent(m_ContourActor))
    m_PropAssembly->RemovePart(m_ContourActor);

  m_Spheres->RemoveAllInputs();
  m_Contour->Initialize();

  mitk::Mesh::Pointer input  = const_cast<mitk::Mesh*>(this->GetInput());
  input->Update();

  mitk::Mesh::DataType::Pointer itkMesh = input->GetMesh( m_TimeStep );

  if ( itkMesh.GetPointer() == NULL) 
  {
    m_PropAssembly->VisibilityOff();
    return;
  }

  
  mitk::Mesh::PointsContainer::Iterator i;

  int j;

  vtkFloatingPointType rgba[4]={1.0f,1.0f,1.0f,1.0f};
  mitk::Color tmpColor;

  // check for color prop and use it for rendering if it exists
  m_DataTreeNode->GetColor((float*)rgba, NULL); 
 
  if (dynamic_cast<mitk::ColorProperty*>(this->GetDataTreeNode()->GetProperty("unselectedcolor").GetPointer()) != NULL)
  {
    tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataTreeNode()->GetProperty("unselectedcolor").GetPointer())->GetValue();
    rgba[0] = tmpColor[0];
    rgba[1] = tmpColor[1];
    rgba[2] = tmpColor[2];
    rgba[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
  }

  if(itkMesh->GetNumberOfPoints()>0)
  {
    // build m_Spheres->GetOutput() vtkPolyData
    float pointSize = 2.0;
    mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataTreeNode()->GetProperty("pointsize").GetPointer());
    if (pointSizeProp.IsNotNull())
      pointSize = pointSizeProp->GetValue();

    for (j=0, i=itkMesh->GetPoints()->Begin(); i!=itkMesh->GetPoints()->End() ; i++,j++)
    {
      vtkSphereSource *sphere = vtkSphereSource::New();

      sphere->SetRadius(pointSize);
      sphere->SetCenter(i.Value()[0],i.Value()[1],i.Value()[2]);

      m_Spheres->AddInput(sphere->GetOutput());
      sphere->Delete();
    }

    // setup mapper, actor and add to assembly
    m_SpheresMapper->SetInput(m_Spheres->GetOutput());
    m_SpheresActor->GetProperty()->SetColor(rgba);
    m_PropAssembly->AddPart(m_SpheresActor);
  }

  if(itkMesh->GetNumberOfCells()>0)
  {
    // build m_Contour vtkPolyData
#ifdef VCL_VC60
    itkExceptionMacro(<<"MeshVtkMapper3D currently not working for MS Visual C++ 6.0, because MeshUtils are currently not supported.");
#else
    m_Contour = MeshUtil<mitk::Mesh::MeshType>::MeshToPolyData(itkMesh.GetPointer(), false, false, 0, NULL, m_Contour);
#endif

    if(m_Contour->GetNumberOfCells()>0)
    {
      // setup mapper, actor and add to assembly
      m_ContourMapper->SetInput(m_Contour);
      bool wireframe=true;
      GetDataTreeNode()->GetVisibility(wireframe, NULL, "wireframe");
      if(wireframe)
        m_ContourActor->GetProperty()->SetRepresentationToWireframe();
      else
        m_ContourActor->GetProperty()->SetRepresentationToSurface();
      m_ContourActor->GetProperty()->SetColor(rgba);
      m_PropAssembly->AddPart(m_ContourActor);
    }
  }
}


void mitk::MeshVtkMapper3D::GenerateData( mitk::BaseRenderer *renderer )
{
  if(IsVisible(renderer)==false)
  {
    m_SpheresActor->VisibilityOff();
    m_ContourActor->VisibilityOff();
    return;
  }

  bool makeContour = false;
  this->GetDataTreeNode()->GetBoolProperty("contour", makeContour);

  if (makeContour)
  {
    m_ContourActor->VisibilityOn();
  }
  else
  {
    m_ContourActor->VisibilityOff();
  }

  bool showPoints = true;
  this->GetDataTreeNode()->GetBoolProperty("show points", showPoints);
  if(showPoints)
  {
    m_SpheresActor->VisibilityOn();
  }
  else
  {
    m_SpheresActor->VisibilityOff();
  }


  // Get the TimeSlicedGeometry of the input object
  mitk::Mesh::Pointer input  = const_cast<mitk::Mesh*>(this->GetInput());
  const TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  if (( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ))
  {
    m_PropAssembly->VisibilityOff();
    return;
  }

  if( inputTimeGeometry->IsValidTime( m_TimeStep ) == false )
  {
    m_PropAssembly->VisibilityOff();
    return;
  }
}
