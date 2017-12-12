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

#include "mitkFiberBundleMapper2D.h"
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

class vtkShaderCallback : public vtkCommand
{
public:
  static vtkShaderCallback *New()
  {
    return new vtkShaderCallback;
  }
  mitk::BaseRenderer *renderer;
  mitk::DataNode *node;

  void Execute(vtkObject *, unsigned long, void*cbo) override
  {
    vtkOpenGLHelper *cellBO = reinterpret_cast<vtkOpenGLHelper*>(cbo);

    float fiberOpacity;
    bool fiberFading = false;
    float fiberThickness = 0.0;

    node->GetOpacity(fiberOpacity, nullptr);
    node->GetFloatProperty("Fiber2DSliceThickness", fiberThickness);
    node->GetBoolProperty("Fiber2DfadeEFX", fiberFading);

    cellBO->Program->SetUniformf("fiberOpacity", fiberOpacity);
    cellBO->Program->SetUniformi("fiberFadingON", fiberFading);
    cellBO->Program->SetUniformf("fiberThickness", fiberThickness);

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

  vtkShaderCallback() { this->renderer = 0; }
};



mitk::FiberBundleMapper2D::FiberBundleMapper2D()
  : m_LineWidth(1)
{
  m_lut = vtkLookupTable::New();
  m_lut->Build();

}

mitk::FiberBundleMapper2D::~FiberBundleMapper2D()
{
}


mitk::FiberBundle* mitk::FiberBundleMapper2D::GetInput()
{
  return dynamic_cast< mitk::FiberBundle * > ( GetDataNode()->GetData() );
}



void mitk::FiberBundleMapper2D::Update(mitk::BaseRenderer * renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if ( !visible )
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
  // this method is implemented in mitkMapper
  this->CalculateTimeStep( renderer );

  //check if updates occured in the node or on the display
  FBXLocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  //set renderer independent shader properties
  const DataNode::Pointer node = this->GetDataNode();
  float thickness = 2.0;
  if(!this->GetDataNode()->GetPropertyValue("Fiber2DSliceThickness",thickness))
    MITK_INFO << "FIBER2D SLICE THICKNESS PROPERTY ERROR";

  bool fiberfading = false;
  if(!this->GetDataNode()->GetPropertyValue("Fiber2DfadeEFX",fiberfading))
    MITK_INFO << "FIBER2D SLICE FADE EFX PROPERTY ERROR";

  mitk::FiberBundle* fiberBundle = this->GetInput();
  if (fiberBundle==nullptr)
    return;

  int lineWidth = 0;
  node->GetIntProperty("LineWidth", lineWidth);
  if (m_LineWidth!=lineWidth)
  {
    m_LineWidth = lineWidth;
    fiberBundle->RequestUpdate2D();
  }

  if ( localStorage->m_LastUpdateTime<renderer->GetCurrentWorldPlaneGeometryUpdateTime() || localStorage->m_LastUpdateTime<fiberBundle->GetUpdateTime2D() )
  {
    this->UpdateShaderParameter(renderer);
    this->GenerateDataForRenderer( renderer );
  }
}

void mitk::FiberBundleMapper2D::UpdateShaderParameter(mitk::BaseRenderer *)
{
  // see new vtkShaderCallback
}

// vtkActors and Mappers are feeded here
void mitk::FiberBundleMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  mitk::FiberBundle* fiberBundle = this->GetInput();

  //the handler of local storage gets feeded in this method with requested data for related renderwindow
  FBXLocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  mitk::DataNode* node = this->GetDataNode();
  if (node == nullptr)
    return;

  vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
  if (fiberPolyData == nullptr)
    return;

  fiberPolyData->GetPointData()->AddArray(fiberBundle->GetFiberColors());
  localStorage->m_FiberMapper->ScalarVisibilityOn();
  localStorage->m_FiberMapper->SetScalarModeToUsePointFieldData();
  localStorage->m_FiberMapper->SetLookupTable(m_lut);  //apply the properties after the slice was set
  localStorage->m_PointActor->GetProperty()->SetOpacity(0.999);
  localStorage->m_FiberMapper->SelectColorArray("FIBER_COLORS");

  localStorage->m_FiberMapper->SetInputData(fiberPolyData);
  localStorage->m_FiberMapper->SetVertexShaderCode(
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
  localStorage->m_FiberMapper->SetFragmentShaderCode(
        "//VTK::System::Dec\n"  // always start with this line
        "//VTK::Output::Dec\n"  // always have this line in your FS
        "uniform vec4 slicingPlane;\n"
        "uniform float fiberThickness;\n"
        "uniform int fiberFadingON;\n"
        "uniform float fiberOpacity;\n"

        "varying vec4 positionWorld;\n"
        "varying vec4 colorVertex;\n"

        "void main(void)\n"
        "{\n"
        "  float r1 = dot(positionWorld.xyz, slicingPlane.xyz) - slicingPlane.w;\n"

        "  if (abs(r1) >= fiberThickness)\n"
        "    discard;\n"

        "  if (fiberFadingON != 0)\n"
        "  {\n"
        "    float x = (r1 + fiberThickness) / (fiberThickness*2.0);\n"
        "    x = 1.0 - x;\n"
        "    gl_FragColor = vec4(colorVertex.xyz*x, fiberOpacity);\n"
        "  }\n"
        "  else{\n"
        "    gl_FragColor = vec4(colorVertex.xyz,fiberOpacity);\n"
        "  }\n"
        "}\n"
        );

  vtkSmartPointer<vtkShaderCallback> myCallback = vtkSmartPointer<vtkShaderCallback>::New();
  myCallback->renderer = renderer;
  myCallback->node = this->GetDataNode();
  localStorage->m_FiberMapper->AddObserver(vtkCommand::UpdateShaderEvent,myCallback);

  localStorage->m_PointActor->SetMapper(localStorage->m_FiberMapper);
  localStorage->m_PointActor->GetProperty()->ShadingOn();
  localStorage->m_PointActor->GetProperty()->SetLineWidth(m_LineWidth);

  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();
}


vtkProp* mitk::FiberBundleMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  this->Update(renderer);
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_PointActor;
}


void mitk::FiberBundleMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);
  //    node->SetProperty("shader",mitk::ShaderProperty::New("mitkShaderFiberClipping"));

  //add other parameters to propertylist
  node->AddProperty( "Fiber2DSliceThickness", mitk::FloatProperty::New(1.0f), renderer, overwrite );
  node->AddProperty( "Fiber2DfadeEFX", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "color", mitk::ColorProperty::New(1.0,1.0,1.0), renderer, overwrite);
}


mitk::FiberBundleMapper2D::FBXLocalStorage::FBXLocalStorage()
{
  m_PointActor = vtkSmartPointer<vtkActor>::New();
  m_FiberMapper = vtkSmartPointer<MITKFIBERBUNDLEMAPPER2D_POLYDATAMAPPER>::New();
}
