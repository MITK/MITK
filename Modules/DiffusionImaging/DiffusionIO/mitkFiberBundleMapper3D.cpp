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

#include "mitkFiberBundleMapper3D.h"
#include <mitkProperties.h>

#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkDepthSortPolyData.h>
#include <vtkCamera.h>
#include <vtkTubeFilter.h>
#include <vtkRibbonFilter.h>
#include <vtkLookupTable.h>
#include <vtkOpenGLHelper.h>
#include <vtkShaderProgram.h>
#include <mitkVectorProperty.h>

class vtkShaderCallback3D : public vtkCommand
{
public:
  static vtkShaderCallback3D *New()
  {
    return new vtkShaderCallback3D;
  }
  mitk::BaseRenderer *renderer;
  mitk::DataNode *node;

  virtual void Execute(vtkObject *, unsigned long, void*cbo)
  {
    vtkOpenGLHelper *cellBO = reinterpret_cast<vtkOpenGLHelper*>(cbo);

    float fiberOpacity;
    node->GetOpacity(fiberOpacity, nullptr);
    cellBO->Program->SetUniformf("fiberOpacity", fiberOpacity);

    if (this->renderer)
    {
      mitk::Vector3D plane_vec;
      node->GetPropertyValue("Fiber3DClippingPlane",plane_vec);
      float distance = plane_vec.GetNorm();
      plane_vec.Normalize();

      bool flip;
      node->GetBoolProperty("Fiber3DClippingPlaneFlip",flip);

      if (flip)
      {
        plane_vec *= -1;
        distance *= -1;
      }

      node->GetBoolProperty("Fiber3DClippingPlaneSecondFlip",flip);
      if (flip)
      {
        plane_vec *= -1;
        distance *= -1;
      }

      float* a = new float[4];
      for (int i = 0; i < 3; ++i)
        a[i] = plane_vec[i];

      a[3] = distance;
      cellBO->Program->SetUniform4f("slicingPlane", a);

      float v = 1;
      node->GetFloatProperty("light.ambient", v);
      cellBO->Program->SetUniformf("ambient", v);

      node->GetFloatProperty("light.diffuse", v);
      cellBO->Program->SetUniformf("diffuse", v);

      node->GetFloatProperty("light.specular", v);
      cellBO->Program->SetUniformf("intensity", v);

      node->GetFloatProperty("light.intensity", v);
      cellBO->Program->SetUniformf("intensity", v);

      bool enable_light = false;
      node->GetBoolProperty("light.enable_light", enable_light);
      cellBO->Program->SetUniformi("enable_light", enable_light);
    }
  }

  vtkShaderCallback3D() { this->renderer = 0; }
};

mitk::FiberBundleMapper3D::FiberBundleMapper3D()
  : m_TubeRadius(0.0)
  , m_TubeSides(15)
  , m_LineWidth(1)
{
  m_lut = vtkLookupTable::New();
  m_lut->Build();
}


mitk::FiberBundleMapper3D::~FiberBundleMapper3D()
{

}


const mitk::FiberBundle* mitk::FiberBundleMapper3D::GetInput()
{
  return static_cast<const mitk::FiberBundle * > ( GetDataNode()->GetData() );
}


/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::FiberBundleMapper3D::InternalGenerateData(mitk::BaseRenderer *renderer)
{
  m_FiberPolyData->GetPointData()->AddArray(m_FiberBundle->GetFiberColors());
  float tmpopa;
  this->GetDataNode()->GetOpacity(tmpopa, nullptr);
  FBXLocalStorage3D *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  if (m_TubeRadius>0.0)
  {
    vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
    tubeFilter->SetInputData(m_FiberPolyData);
    tubeFilter->SetNumberOfSides(m_TubeSides);
    tubeFilter->SetRadius(m_TubeRadius);
    tubeFilter->Update();
    m_FiberPolyData = tubeFilter->GetOutput();
  }
  else if (m_RibbonWidth>0.0)
  {
    vtkSmartPointer<vtkRibbonFilter> tubeFilter = vtkSmartPointer<vtkRibbonFilter>::New();
    tubeFilter->SetInputData(m_FiberPolyData);
    tubeFilter->SetWidth(m_RibbonWidth);
    tubeFilter->Update();
    m_FiberPolyData = tubeFilter->GetOutput();
  }

  if (tmpopa<1)
  {
    vtkSmartPointer<vtkDepthSortPolyData> depthSort = vtkSmartPointer<vtkDepthSortPolyData>::New();
    depthSort->SetInputData( m_FiberPolyData );
    depthSort->SetCamera( renderer->GetVtkRenderer()->GetActiveCamera() );
    depthSort->SetDirectionToFrontToBack();
    depthSort->Update();
    localStorage->m_FiberMapper->SetInputConnection(depthSort->GetOutputPort());
  }
  else
  {
    localStorage->m_FiberMapper->SetInputData(m_FiberPolyData);
  }

  if (m_Lighting)
  {
    float floatProp = 1.0;
    GetDataNode()->GetFloatProperty("light.ambient", floatProp);
    localStorage->m_FiberActor->GetProperty()->SetAmbient(floatProp);
    GetDataNode()->GetFloatProperty("light.diffuse", floatProp);
    localStorage->m_FiberActor->GetProperty()->SetDiffuse(floatProp);
    GetDataNode()->GetFloatProperty("light.specular", floatProp);
    localStorage->m_FiberActor->GetProperty()->SetSpecular(floatProp);
    GetDataNode()->GetFloatProperty("light.specularpower", floatProp);
    localStorage->m_FiberActor->GetProperty()->SetSpecularPower( floatProp );

    mitk::ColorProperty* ambientC = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("light.ambientcolor"));
    mitk::ColorProperty* diffuseC = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("light.diffusecolor"));
    mitk::ColorProperty* specularC = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("light.specularcolor"));

    localStorage->m_FiberActor->GetProperty()->SetAmbientColor( ambientC->GetColor()[0], ambientC->GetColor()[1], ambientC->GetColor()[2] );
    localStorage->m_FiberActor->GetProperty()->SetDiffuseColor( diffuseC->GetColor()[0], diffuseC->GetColor()[1], diffuseC->GetColor()[2] );
    localStorage->m_FiberActor->GetProperty()->SetSpecularColor( specularC->GetColor()[0], specularC->GetColor()[1], specularC->GetColor()[2] );
    localStorage->m_FiberActor->GetProperty()->SetLighting(true);
  }
  else
  {
    localStorage->m_FiberActor->GetProperty()->SetLighting(false);
  }

  localStorage->m_FiberMapper->SelectColorArray("FIBER_COLORS");
  localStorage->m_FiberMapper->ScalarVisibilityOn();
  localStorage->m_FiberMapper->SetScalarModeToUsePointFieldData();
  localStorage->m_FiberActor->SetMapper(localStorage->m_FiberMapper);
  localStorage->m_FiberMapper->SetLookupTable(m_lut);

  // set Opacity
  localStorage->m_FiberActor->GetProperty()->SetOpacity((double) tmpopa);
  localStorage->m_FiberActor->GetProperty()->SetLineWidth(m_LineWidth);

  localStorage->m_FiberAssembly->AddPart(localStorage->m_FiberActor);


  localStorage->m_FiberMapper->SetVertexShaderCode(
        "//VTK::System::Dec\n"
        "attribute vec4 vertexMC;\n"

        "//VTK::Normal::Dec\n"
        "uniform mat4 MCDCMatrix;\n"
        "uniform mat4 MCVCMatrix;\n"

        "//VTK::Color::Dec\n"
        "attribute vec3 a_vNormal;\n"

        "varying vec4 positionWorld;\n"
        "varying vec4 colorVertex;\n"
        "varying vec4 N;\n"
        "varying vec4 v;\n"

        "void main(void)\n"
        "{\n"
        "  colorVertex = scalarColor;\n"
        "  positionWorld = vertexMC;\n"
        "  v = MCVCMatrix * vertexMC;\n"
        "  mat4 glNormalMatrix = transpose(inverse(MCVCMatrix));\n"
        "  N = normalize(glNormalMatrix * vec4(a_vNormal,0));\n"
        "  gl_Position = MCDCMatrix * vertexMC;\n"
        "}\n"
        );
  localStorage->m_FiberMapper->SetFragmentShaderCode(
        "//VTK::System::Dec\n"  // always start with this line
        "//VTK::Output::Dec\n"  // always have this line in your FS

        "uniform vec4 slicingPlane;\n"
        "uniform float fiberOpacity;\n"
        "uniform float ambient;\n"
        "uniform float diffuse;\n"
        "uniform float specular;\n"
        "uniform float intensity;\n"
        "uniform int enable_light;\n"
        "varying vec4 positionWorld;\n"
        "varying vec4 colorVertex;\n"

        "varying vec4 N;\n"
        "varying vec4 v;\n"

        "void main(void)\n"
        "{\n"
        "   float r1 = dot(positionWorld.xyz, slicingPlane.xyz) - slicingPlane.w;\n"
        "   if ( r1 > 0 )\n"
        "     discard;\n"

        "   if (enable_light!=0)\n"
        "   {\n"
        "     vec3 L = normalize(-v.xyz);\n"
//        "normalize(gl_LightSource[0].position.xyz - v.xyz);\n"
        "     vec3 E = normalize(-v.xyz); // we are in Eye Coordinates, so EyePos is (0,0,0)\n"
        "     vec3 R = normalize(-reflect(L,N.xyz));\n"

        //calculate Diffuse Term:
        "     float Idiff = diffuse * max(dot(N.xyz,L), 0.0);\n"
        "     Idiff = clamp(Idiff, 0.0, 1.0);\n"

        // calculate Specular Term:
        "     float Ispec = specular * pow(max(dot(R,E),0.0),0.3);\n"
        "     Ispec = clamp(Ispec, 0.0, 1.0);\n"

        "     gl_FragColor = vec4(colorVertex.xyz, fiberOpacity)*(1-intensity) + vec4(colorVertex.xyz * (ambient + Idiff + Ispec) * intensity, fiberOpacity);\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "     gl_FragColor = vec4(colorVertex.xyz, fiberOpacity);\n"
        "   }\n"

        "}\n"
        );


  vtkSmartPointer<vtkShaderCallback3D> myCallback = vtkSmartPointer<vtkShaderCallback3D>::New();
  myCallback->renderer = renderer;
  myCallback->node = this->GetDataNode();
  localStorage->m_FiberMapper->AddObserver(vtkCommand::UpdateShaderEvent,myCallback);


  localStorage->m_LastUpdateTime.Modified();
}



void mitk::FiberBundleMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if ( !visible )
    return;

  const DataNode* node = this->GetDataNode();
  FBXLocalStorage3D* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  m_FiberBundle = dynamic_cast<mitk::FiberBundle*>(node->GetData());
  m_FiberPolyData = m_FiberBundle->GetFiberPolyData();

  //  this->ApplyShaderProperties(renderer, "shader_3d");

  // did any rendering properties change?
  float tubeRadius = 0;
  node->GetFloatProperty("shape.tuberadius", tubeRadius);
  if (m_TubeRadius!=tubeRadius)
  {
    m_TubeRadius = tubeRadius;
    m_FiberBundle->RequestUpdate3D();
  }

  int tubeSides = 0;
  node->GetIntProperty("shape.tubesides", tubeSides);
  if (m_TubeSides!=tubeSides)
  {
    m_TubeSides = tubeSides;
    m_FiberBundle->RequestUpdate3D();
  }

  int lineWidth = 0;
  node->GetIntProperty("shape.linewidth", lineWidth);
  if (m_LineWidth!=lineWidth)
  {
    m_LineWidth = lineWidth;
    m_FiberBundle->RequestUpdate3D();
  }

  float ribbonWidth = 0;
  node->GetFloatProperty("shape.ribbonwidth", ribbonWidth);
  if (m_RibbonWidth!=ribbonWidth)
  {
    m_RibbonWidth = ribbonWidth;
    m_FiberBundle->RequestUpdate3D();
  }

  bool lighting = false;
  node->GetBoolProperty("light.enable", lighting);
  if (m_Lighting!=lighting)
  {
    m_Lighting = lighting;
    m_FiberBundle->RequestUpdate3D();
  }

  if (localStorage->m_LastUpdateTime>=m_FiberBundle->GetUpdateTime3D())
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
  // this method is implemented in mitkMapper
  this->CalculateTimeStep( renderer );
  this->InternalGenerateData(renderer);
}

void mitk::FiberBundleMapper3D::UpdateShaderParameter(mitk::BaseRenderer * )
{
  // see new vtkShaderCallback3D
}

void mitk::FiberBundleMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  mitk::Vector3D plane_vec; plane_vec.Fill(0.0);
  node->AddProperty( "Fiber3DClippingPlane", mitk::Vector3DProperty::New( plane_vec ), renderer, overwrite );
  node->AddProperty( "Fiber3DClippingPlaneId", mitk::IntProperty::New( 0 ), renderer, overwrite );
  node->AddProperty( "Fiber3DClippingPlaneFlip", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "Fiber3DClippingPlaneSecondFlip", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "opacity", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
  node->AddProperty( "color", mitk::ColorProperty::New(1.0,1.0,1.0), renderer, overwrite);
  node->AddProperty( "pickable", mitk::BoolProperty::New( true ), renderer, overwrite);

  node->AddProperty( "shape.linewidth", mitk::IntProperty::New( true ), renderer, overwrite );
  node->AddProperty( "shape.tuberadius",mitk::FloatProperty::New( 0.0 ), renderer, overwrite);
  node->AddProperty( "shape.tubesides",mitk::IntProperty::New( 15 ), renderer, overwrite);
  node->AddProperty( "shape.ribbonwidth", mitk::FloatProperty::New( 0.0 ), renderer, overwrite);

  node->AddProperty( "light.intensity", mitk::FloatProperty::New( 0.6 ), renderer, overwrite);
  node->AddProperty( "light.enable_light", mitk::BoolProperty::New( false ), renderer, overwrite);
  node->AddProperty( "light.ambient", mitk::FloatProperty::New( 0.05 ), renderer, overwrite);
  node->AddProperty( "light.diffuse", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
  node->AddProperty( "light.specular", mitk::FloatProperty::New( 0.0 ), renderer, overwrite);
  node->AddProperty( "light.specularpower", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
  node->AddProperty( "light.ambientcolor", mitk::ColorProperty::New(1,1,1), renderer, overwrite);
  node->AddProperty( "light.diffusecolor", mitk::ColorProperty::New(1,1,1), renderer, overwrite);
  node->AddProperty( "light.specularcolor", mitk::ColorProperty::New(1,1,1), renderer, overwrite);
}

vtkProp* mitk::FiberBundleMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_FiberAssembly;
}

void mitk::FiberBundleMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{

}

mitk::FiberBundleMapper3D::FBXLocalStorage3D::FBXLocalStorage3D()
{
  m_FiberActor = vtkSmartPointer<vtkActor>::New();
  m_FiberMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
  m_FiberAssembly = vtkSmartPointer<vtkPropAssembly>::New();
}

