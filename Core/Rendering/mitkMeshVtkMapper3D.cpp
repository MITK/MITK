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
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"
#include "mitkMeshUtil.h"


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
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Mesh * > ( GetData() );
}

vtkProp* mitk::MeshVtkMapper3D::GetProp()
{
  return m_PropAssemply;
}

void mitk::MeshVtkMapper3D::GenerateData()
{
}

void mitk::MeshVtkMapper3D::GenerateOutputInformation()
{
}

mitk::MeshVtkMapper3D::MeshVtkMapper3D() : m_PropAssemply(NULL)
{
	m_Spheres = vtkAppendPolyData::New();
	m_vtkTextList = vtkAppendPolyData::New();
	m_Contour = vtkPolyData::New();
	m_tubefilter = vtkTubeFilter::New();

  m_SpheresActor = vtkActor::New();
  m_SpheresMapper = vtkPolyDataMapper::New();
  m_SpheresActor->SetMapper(m_SpheresMapper);

  m_ContourActor = vtkActor::New();
  m_ContourMapper = vtkPolyDataMapper::New();
  m_ContourActor->SetMapper(m_ContourMapper);

  m_PropAssemply = vtkPropAssembly::New();
  m_PropAssemply->AddPart(m_ContourActor);
  m_PropAssemply->AddPart(m_SpheresActor);

  m_Prop3D = m_ContourActor;
}

mitk::MeshVtkMapper3D::~MeshVtkMapper3D()
{
  m_ContourActor->Delete();
  m_SpheresActor->Delete();
  m_ContourMapper->Delete();
  m_SpheresMapper->Delete();
  m_PropAssemply->Delete();
  m_Spheres->Delete();
}

void mitk::MeshVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
  {
    m_PropAssemply->VisibilityOff();
    return;
  }
  m_PropAssemply->VisibilityOn();

	m_Spheres->Delete();
	m_vtkTextList->Delete();
	m_Contour->Delete();
	m_tubefilter->Delete();

	m_Spheres = vtkAppendPolyData::New();
	m_vtkTextList = vtkAppendPolyData::New();
	m_tubefilter = vtkTubeFilter::New();

  mitk::Mesh::Pointer input  = const_cast<mitk::Mesh*>(this->GetInput());
  mitk::Mesh::DataType::Pointer mesh;

	mesh = input->GetMesh();

  mitk::Mesh::PointsContainer::Iterator i;

	int j;

  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  GetColor(rgba, renderer);

  for (j=0, i=mesh->GetPoints()->Begin(); i!=mesh->GetPoints()->End() ; i++,j++)
	{
	  vtkSphereSource *sphere = vtkSphereSource::New();

		sphere->SetRadius(2);
    sphere->SetCenter(i.Value()[0],i.Value()[1],i.Value()[2]);

		m_Spheres->AddInput(sphere->GetOutput());
	}

 // build contour vtkPolyData
  m_Contour = MeshUtil<mitk::Mesh::MeshType>::meshToPolyData(mesh.GetPointer());
  m_ContourMapper->SetInput(m_Contour);

  bool wireframe=true;
  GetDataTreeNode()->GetVisibility(wireframe, renderer, "wireframe");
  if(wireframe)
    m_ContourActor->GetProperty()->SetRepresentationToWireframe();
  else
    m_ContourActor->GetProperty()->SetRepresentationToSurface();

  m_ContourActor->GetProperty()->SetColor(rgba);
  m_SpheresActor->GetProperty()->SetColor(rgba);

  m_SpheresMapper->SetInput(m_Spheres->GetOutput());

  StandardUpdate();
}


