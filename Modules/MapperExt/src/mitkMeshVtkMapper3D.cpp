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

#include "mitkMeshVtkMapper3D.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkVtkPropRenderer.h"

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
  return static_cast<const mitk::Mesh * > ( GetDataNode()->GetData() );
}

vtkProp* mitk::MeshVtkMapper3D::GetVtkProp(mitk::BaseRenderer * /*renderer*/)
{
  return m_PropAssembly;
}

void mitk::MeshVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer * /*renderer*/)
{
  vtkLinearTransform * vtktransform =
      this->GetDataNode()->GetVtkTransform(this->GetTimestep());

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


void mitk::MeshVtkMapper3D::GenerateDataForRenderer( mitk::BaseRenderer* renderer )
{

  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool needGenerateData = ls->IsGenerateDataRequired( renderer, this, GetDataNode() );

  if(needGenerateData)
  {
    ls->UpdateGenerateDataTime();

    m_PropAssembly->VisibilityOn();

    if(m_PropAssembly->GetParts()->IsItemPresent(m_SpheresActor))
      m_PropAssembly->RemovePart(m_SpheresActor);
    if(m_PropAssembly->GetParts()->IsItemPresent(m_ContourActor))
      m_PropAssembly->RemovePart(m_ContourActor);

    m_Spheres->RemoveAllInputs();
    m_Contour->Initialize();

    mitk::Mesh::Pointer input  = const_cast<mitk::Mesh*>(this->GetInput());
    input->Update();

    mitk::Mesh::DataType::Pointer itkMesh = input->GetMesh( this->GetTimestep() );

    if ( itkMesh.GetPointer() == NULL)
    {
      m_PropAssembly->VisibilityOff();
      return;
    }


    mitk::Mesh::PointsContainer::Iterator i;

    int j;

    float floatRgba[4] = {1.0f,1.0f,1.0f,1.0f};
    double doubleRgba[4]={1.0f,1.0f,1.0f,1.0f};
    mitk::Color tmpColor;

    // check for color prop and use it for rendering if it exists
    m_DataNode->GetColor(floatRgba, NULL);

    if (dynamic_cast<mitk::ColorProperty*>(this->GetDataNode()->GetProperty("unselectedcolor")) != NULL)
    {
      tmpColor = dynamic_cast<mitk::ColorProperty *>(this->GetDataNode()->GetProperty("unselectedcolor"))->GetValue();
      floatRgba[0] = tmpColor[0];
      floatRgba[1] = tmpColor[1];
      floatRgba[2] = tmpColor[2];
      floatRgba[3] = 1.0f; //!!define a new ColorProp to be able to pass alpha value
      doubleRgba[0] = floatRgba[0];
      doubleRgba[1] = floatRgba[1];
      doubleRgba[2] = floatRgba[2];
      doubleRgba[3] = floatRgba[3];
    }

    if(itkMesh->GetNumberOfPoints()>0)
    {
      // build m_Spheres->GetOutput() vtkPolyData
      float pointSize = 2.0;
      mitk::FloatProperty::Pointer pointSizeProp = dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("pointsize"));
      if (pointSizeProp.IsNotNull())
        pointSize = pointSizeProp->GetValue();

      for (j=0, i=itkMesh->GetPoints()->Begin(); i!=itkMesh->GetPoints()->End() ; i++,j++)
      {
        vtkSphereSource *sphere = vtkSphereSource::New();

        sphere->SetRadius(pointSize);
        sphere->SetCenter(i.Value()[0],i.Value()[1],i.Value()[2]);

        m_Spheres->AddInputConnection(sphere->GetOutputPort());
        sphere->Delete();
      }

      // setup mapper, actor and add to assembly
      m_SpheresMapper->SetInputConnection(m_Spheres->GetOutputPort());
      m_SpheresActor->GetProperty()->SetColor(doubleRgba);
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
        m_ContourMapper->SetInputData(m_Contour);
        bool wireframe=true;
        GetDataNode()->GetVisibility(wireframe, NULL, "wireframe");
        if(wireframe)
          m_ContourActor->GetProperty()->SetRepresentationToWireframe();
        else
          m_ContourActor->GetProperty()->SetRepresentationToSurface();
        m_ContourActor->GetProperty()->SetColor(doubleRgba);
        m_PropAssembly->AddPart(m_ContourActor);
      }
    }

  }

  SetVtkMapperImmediateModeRendering(m_ContourMapper);
  SetVtkMapperImmediateModeRendering(m_SpheresMapper);

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if(!visible)
  {
    m_SpheresActor->VisibilityOff();
    m_ContourActor->VisibilityOff();
    return;
  }

  bool makeContour = false;
  this->GetDataNode()->GetBoolProperty("show contour", makeContour);

  if (makeContour)
  {
    m_ContourActor->VisibilityOn();
  }
  else
  {
    m_ContourActor->VisibilityOff();
  }

  bool showPoints = true;
  this->GetDataNode()->GetBoolProperty("show points", showPoints);
  if(showPoints)
  {
    m_SpheresActor->VisibilityOn();
  }
  else
  {
    m_SpheresActor->VisibilityOff();
  }
}

void mitk::MeshVtkMapper3D::ResetMapper( BaseRenderer* /*renderer*/ )
{
  m_PropAssembly->VisibilityOff();
}
