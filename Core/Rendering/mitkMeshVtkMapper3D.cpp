#include "mitkMeshVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"


#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>


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

void mitk::MeshVtkMapper3D::GenerateData()
{
}

void mitk::MeshVtkMapper3D::GenerateOutputInformation()
{
}

mitk::MeshVtkMapper3D::MeshVtkMapper3D()
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);
  
	m_Prop3D = m_Actor;
	m_Prop3D->Register(NULL);

	m_vtkMesh = vtkAppendPolyData::New();
	m_vtkTextList = vtkAppendPolyData::New();
	m_contour = vtkPolyData::New();
	m_tubefilter = vtkTubeFilter::New();
}

mitk::MeshVtkMapper3D::~MeshVtkMapper3D()
{
  m_VtkPolyDataMapper->Delete();
  m_Actor->Delete();
}

void mitk::MeshVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
  {
    m_Actor->VisibilityOff();
    return;
  }
  m_Actor->VisibilityOn();

	m_vtkMesh->Delete();
	m_vtkTextList->Delete();
	m_contour->Delete();
	m_tubefilter->Delete();

	m_vtkMesh = vtkAppendPolyData::New();
	m_vtkTextList = vtkAppendPolyData::New();
	m_contour = vtkPolyData::New();
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

		m_vtkMesh->AddInput(sphere->GetOutput());
	}

  m_VtkPolyDataMapper->SetInput(m_vtkMesh->GetOutput());
  m_Actor->GetProperty()->SetColor(rgba);

  StandardUpdate();
}


