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

#include "mitkPeakImageMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include <vtkActor.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLHelper.h>
#include <vtkShaderProgram.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkCamera.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkTubeFilter.h>
#include <mitkPlaneGeometry.h>
#include <mitkSliceNavigationController.h>
#include <mitkCoreServices.h>

class vtkPeakShaderCallback : public vtkCommand
{
public:
  static vtkPeakShaderCallback *New()
  {
    return new vtkPeakShaderCallback;
  }
  mitk::BaseRenderer *renderer;
  mitk::DataNode *node;

  void Execute(vtkObject *, unsigned long, void*cbo) override
  {
    vtkOpenGLHelper *cellBO = reinterpret_cast<vtkOpenGLHelper*>(cbo);


    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();

    float minSpacing = 1;
    if(spacing[0]<spacing[1] && spacing[0]<spacing[2])
      minSpacing = spacing[0];
    else if (spacing[1] < spacing[2])
      minSpacing = spacing[1];
    else
      minSpacing = spacing[2];

    float peakOpacity;
    node->GetOpacity(peakOpacity, nullptr);

    cellBO->Program->SetUniformf("peakOpacity", peakOpacity);
    cellBO->Program->SetUniformf("clippingPlaneThickness", minSpacing/2);

    if (this->renderer)
    {
      //get information about current position of views
      mitk::SliceNavigationController::Pointer sliceContr = renderer->GetSliceNavigationController();
      mitk::PlaneGeometry::ConstPointer planeGeo = sliceContr->GetCurrentPlaneGeometry();

      //generate according cutting planes based on the view position
      float planeNormal[3];
      planeNormal[0] = planeGeo->GetNormal()[0];
      planeNormal[1] = planeGeo->GetNormal()[1];
      planeNormal[2] = planeGeo->GetNormal()[2];

      float tmp1 = planeGeo->GetOrigin()[0] * planeNormal[0];
      float tmp2 = planeGeo->GetOrigin()[1] * planeNormal[1];
      float tmp3 = planeGeo->GetOrigin()[2] * planeNormal[2];
      float thickness = tmp1 + tmp2 + tmp3; //attention, correct normalvector

      float* a = new float[4];
      for (int i = 0; i < 3; ++i)
        a[i] = planeNormal[i];

      a[3] = thickness;
      cellBO->Program->SetUniform4f("slicingPlane", a);
    }
  }

  vtkPeakShaderCallback() { this->renderer = 0; }
};



mitk::PeakImageMapper2D::PeakImageMapper2D()
{
  m_lut = vtkLookupTable::New();
  m_lut->Build();
}

mitk::PeakImageMapper2D::~PeakImageMapper2D()
{
}


mitk::PeakImage* mitk::PeakImageMapper2D::GetInput()
{
  return dynamic_cast< mitk::PeakImage * > ( GetDataNode()->GetData() );
}

void mitk::PeakImageMapper2D::UpdateVtkTransform(mitk::BaseRenderer *)
{
  // don't apply transform since the peak polydata is already in world coordinates.
  return;
}

void mitk::PeakImageMapper2D::Update(mitk::BaseRenderer * renderer)
{
  mitk::DataNode* node = this->GetDataNode();
  if (node == nullptr)
    return;

  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");
  if ( !visible )
    return;

  this->GenerateDataForRenderer( renderer );
}

void mitk::PeakImageMapper2D::UpdateShaderParameter(mitk::BaseRenderer *)
{
  // see new vtkPeakShaderCallback
}

// vtkActors and Mappers are feeded here
void mitk::PeakImageMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  mitk::PeakImage* peakImage = this->GetInput();

  //the handler of local storage gets feeded in this method with requested data for related renderwindow
  FBXLocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  vtkSmartPointer<vtkPolyData> polyData = peakImage->GetPolyData();
  if (polyData == nullptr)
    return;

  localStorage->m_Mapper->ScalarVisibilityOn();
  localStorage->m_Mapper->SetScalarModeToUsePointFieldData();
  localStorage->m_Mapper->SetLookupTable(m_lut);  //apply the properties after the slice was set
//  localStorage->m_PointActor->GetProperty()->SetOpacity(0.999);
  localStorage->m_Mapper->SelectColorArray("FIBER_COLORS");

  localStorage->m_Mapper->SetInputData(polyData);
  localStorage->m_Mapper->SetVertexShaderCode(
        "//VTK::System::Dec\n"
        "attribute vec4 vertexMC;\n"

        "//VTK::Normal::Dec\n"
        "uniform mat4 MCDCMatrix;\n"

        "//VTK::Color::Dec\n"

        "varying vec4 positionWorld;\n"
        "varying vec4 colorVertex;\n"

        "void main(void)\n"
        "{\n"
        "  colorVertex = scalarColor;\n"
        "  positionWorld = vertexMC;\n"
        "  gl_Position = MCDCMatrix * vertexMC;\n"
        "}\n"
        );
  localStorage->m_Mapper->SetFragmentShaderCode(
        "//VTK::System::Dec\n"  // always start with this line
        "//VTK::Output::Dec\n"  // always have this line in your FS
        "uniform vec4 slicingPlane;\n"
        "uniform float clippingPlaneThickness;\n"
        "uniform float peakOpacity;\n"
        "varying vec4 positionWorld;\n"
        "varying vec4 colorVertex;\n"
        "out vec4 out_Color;\n"

        "void main(void)\n"
        "{\n"
        "  float r1 = dot(positionWorld.xyz, slicingPlane.xyz) - slicingPlane.w;\n"

        "  if (abs(r1) >= clippingPlaneThickness)\n"
        "    discard;\n"
        "  out_Color = vec4(colorVertex.xyz,peakOpacity);\n"
        "}\n"
        );

  vtkSmartPointer<vtkPeakShaderCallback> myCallback = vtkSmartPointer<vtkPeakShaderCallback>::New();
  myCallback->renderer = renderer;
  myCallback->node = this->GetDataNode();
  localStorage->m_Mapper->AddObserver(vtkCommand::UpdateShaderEvent,myCallback);

  localStorage->m_PointActor->SetMapper(localStorage->m_Mapper);
  localStorage->m_PointActor->GetProperty()->ShadingOn();

  float linewidth = 1.0;
  this->GetDataNode()->GetFloatProperty("shape.linewidth",linewidth);
  localStorage->m_PointActor->GetProperty()->SetLineWidth(linewidth);

  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();
}


vtkProp* mitk::PeakImageMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  this->Update(renderer);
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_PointActor;
}


void mitk::PeakImageMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  //add other parameters to propertylist
  node->AddProperty( "color", mitk::ColorProperty::New(1.0,1.0,1.0), renderer, overwrite);
  node->AddProperty( "shape.linewidth", mitk::FloatProperty::New(1.0), renderer, overwrite);
}


mitk::PeakImageMapper2D::FBXLocalStorage::FBXLocalStorage()
{
  m_PointActor = vtkSmartPointer<vtkActor>::New();
  m_Mapper = vtkSmartPointer<MITKPeakImageMapper2D_POLYDATAMAPPER>::New();
}
