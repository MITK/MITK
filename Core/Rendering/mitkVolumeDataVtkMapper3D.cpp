#include "mitkVolumeDataVtkMapper3D.h"
#include "DataTreeNode.h"
#include "mitkFloatProperty.h"
#include "mitkBoolProperty.h"
#include "mitkColorProperty.h"
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>



const mitk::VolumeData* mitk::VolumeDataVtkMapper3D::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::VolumeData * > ( GetData() );
}


void mitk::VolumeDataVtkMapper3D::GenerateData()
{
}


void mitk::VolumeDataVtkMapper3D::GenerateOutputInformation()
{
}


mitk::VolumeDataVtkMapper3D::VolumeDataVtkMapper3D()
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();

  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);

  m_Prop3D = m_Actor;
  m_Prop3D->Register(NULL);
}


mitk::VolumeDataVtkMapper3D::~VolumeDataVtkMapper3D()
{
  m_VtkPolyDataMapper->Delete();
  m_Actor->Delete();
}


void mitk::VolumeDataVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
  {
    m_Actor->VisibilityOff();
    return;
  }
  m_Actor->VisibilityOn();

  mitk::VolumeData::Pointer input  = const_cast<mitk::VolumeData*>(this->GetInput());

  m_VtkPolyDataMapper->SetInput(input->GetVtkPolyData());

  //apply properties read from the PropertyList
  ApplyProperties(m_Actor, renderer);

  StandardUpdate();
}


void mitk::VolumeDataVtkMapper3D::Update()
{
}

void mitk::VolumeDataVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{
  bool useCellData;
  if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("useCellDataForColouring").GetPointer()) == NULL)
    useCellData = false;
  else
    useCellData = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("useCellDataForColouring").GetPointer())->GetBool();

  if (useCellData)
  {

    m_VtkPolyDataMapper->SetColorModeToDefault();
    m_VtkPolyDataMapper->SetScalarRange(0,255);
    m_VtkPolyDataMapper->ScalarVisibilityOn();
    m_VtkPolyDataMapper->SetScalarModeToUseCellData();
    m_Actor->GetProperty()->SetSpecular (1);
    m_Actor->GetProperty()->SetSpecularPower (50);
    m_Actor->GetProperty()->SetInterpolationToPhong();

    /*
    m_VtkPolyDataMapper->SetColorModeToDefault();
    m_VtkPolyDataMapper->UseLookupTableScalarRangeOff();
    m_MyMeshMapper->SetColorModeToMapScalars();
    ////m_MyMesh->SetInput(delaunay->GetOutput());
    m_VtkPolyDataMapper->SetScalarRange(0,255);
    m_VtkPolyDataMapper->ScalarVisibilityOn();
    m_VtkPolyDataMapper->SetScalarModeToUseCellData();
    //float rgba[4]={1.0f,1.0f,1.0f,1.0f};
    //// check for color prop and use it for rendering if it exists
    //GetColor(rgba, renderer);
    //m_Actor->GetProperty()->SetColor(rgba);

    m_Actor->GetProperty()->SetAmbient (0.25);
    //m_Actor->GetProperty()->SetDiffuse (0.5);
    //m_Actor->GetProperty()->SetSpecular (1);
    m_Actor->GetProperty()->SetSpecularPower (5);
    m_Actor->GetProperty()->SetInterpolationToPhong();
    */
  }
  else
  {
    Superclass::ApplyProperties(m_Actor, renderer);
    m_VtkPolyDataMapper->ScalarVisibilityOff();
  }

  bool wireframe=false;
  GetDataTreeNode()->GetVisibility(wireframe, renderer, "wireframe");
  if(wireframe)
    m_Actor->GetProperty()->SetRepresentationToWireframe();
  else
    m_Actor->GetProperty()->SetRepresentationToVolume();
}
