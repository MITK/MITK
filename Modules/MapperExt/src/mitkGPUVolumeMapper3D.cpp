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

#define GPU_INFO MITK_INFO("mapper.vr")
#define GPU_WARN MITK_WARN("mapper.vr")
#define GPU_ERROR MITK_ERROR("mapper.vr")

#include "mitkGPUVolumeMapper3D.h"

#include "mitkDataNode.h"

#include "mitkProperties.h"
#include "mitkLevelWindow.h"
#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkTransferFunctionInitializer.h"
#include "mitkColorProperty.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"
#include <mitkImageVtkAccessor.h>

#include <vtkActor.h>
#include <vtkProperty.h>

#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkFiniteDifferenceGradientEstimator.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageWriter.h>
#include <vtkImageData.h>
#include <vtkLODProp3D.h>
#include <vtkImageResample.h>
#include <vtkImplicitPlaneWidget.h>
#include <vtkAssembly.h>
#include <vtkPlaneCollection.h>
#include <vtkCubeSource.h>
#include <vtkOpenGLPolyDataMapper.h>

#include "vtkMitkGPUVolumeRayCastMapper.h"

#include "vtkOpenGLGPUVolumeRayCastMapper.h"


const mitk::Image* mitk::GPUVolumeMapper3D::GetInput()
{
  return static_cast<const mitk::Image*> ( GetDataNode()->GetData() );
}

void mitk::GPUVolumeMapper3D::MitkRenderVolumetricGeometry(mitk::BaseRenderer* renderer)
{
  VtkMapper::MitkRenderVolumetricGeometry(renderer);

//  if (ls->m_gpuInitialized)
//    ls->m_MapperGPU->UpdateMTime();
}

bool mitk::GPUVolumeMapper3D::InitGPU(mitk::BaseRenderer* /*renderer*/)
{
  return false;
}

void mitk::GPUVolumeMapper3D::InitCPU(mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(ls->m_cpuInitialized)
    return;

  ls->m_VtkRenderWindow = renderer->GetVtkRenderer()->GetRenderWindow();

  ls->m_MapperCPU = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
  int numThreads = ls->m_MapperCPU->GetNumberOfThreads( );

  GPU_INFO << "initializing cpu-raycast-vr (vtkFixedPointVolumeRayCastMapper) (" << numThreads << " threads)";

  ls->m_MapperCPU->SetSampleDistance(1.0);
//  ls->m_MapperCPU->LockSampleDistanceToInputSpacingOn();
  ls->m_MapperCPU->SetImageSampleDistance(1.0);
  ls->m_MapperCPU->IntermixIntersectingGeometryOn();
  ls->m_MapperCPU->SetAutoAdjustSampleDistances(0);

  ls->m_VolumePropertyCPU = vtkSmartPointer<vtkVolumeProperty>::New();
  ls->m_VolumePropertyCPU->ShadeOn();
  ls->m_VolumePropertyCPU->SetAmbient (0.10f); //0.05f
  ls->m_VolumePropertyCPU->SetDiffuse (0.50f); //0.45f
  ls->m_VolumePropertyCPU->SetSpecular(0.40f); //0.50f
  ls->m_VolumePropertyCPU->SetSpecularPower(16.0f);
  ls->m_VolumePropertyCPU->SetInterpolationTypeToLinear();

  ls->m_VolumeCPU = vtkSmartPointer<vtkVolume>::New();
  ls->m_VolumeCPU->SetMapper( ls->m_MapperCPU );
  ls->m_VolumeCPU->SetProperty( ls->m_VolumePropertyCPU );
  ls->m_VolumeCPU->VisibilityOn();

  ls->m_MapperCPU->SetInputConnection( m_UnitSpacingImageFilter->GetOutputPort() );//m_Resampler->GetOutput());

  ls->m_cpuInitialized=true;
}

void mitk::GPUVolumeMapper3D::DeinitCPU(mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(!ls->m_cpuInitialized)
    return;

  GPU_INFO << "deinitializing cpu-raycast-vr";

  ls->m_VolumePropertyCPU = NULL;
  ls->m_MapperCPU = NULL;
  ls->m_VolumeCPU = NULL;
  ls->m_cpuInitialized=false;
}

mitk::GPUVolumeMapper3D::GPUVolumeMapper3D()
{
  m_VolumeNULL=0;
  m_commonInitialized=false;
  m_ClippingPlanes = nullptr;
  m_Clipping = false;
}

mitk::GPUVolumeMapper3D::~GPUVolumeMapper3D()
{
  if (m_ClippingPlanes) {
    m_ClippingPlanes->Delete();
    m_ClippingPlanes = nullptr;
  }
  DeinitCommon();
}

void mitk::GPUVolumeMapper3D::setClippingPlanes(vtkPlanes* planes)
{
  if (planes) {
    m_Clipping = true;
    if (m_ClippingPlanes == nullptr) {
      m_ClippingPlanes = vtkPlanes::New();
    }
    m_ClippingPlanes->SetPoints(planes->GetPoints());
    m_ClippingPlanes->SetNormals(planes->GetNormals());
  } else {
    m_Clipping = false;
  }
}

void mitk::GPUVolumeMapper3D::InitCommon()
{
  if(m_commonInitialized)
    return;

  m_UnitSpacingImageFilter = vtkSmartPointer<vtkImageChangeInformation>::New();

  m_UnitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );

  CreateDefaultTransferFunctions();

  m_commonInitialized=true;
}

void mitk::GPUVolumeMapper3D::DeinitCommon()
{
  if(!m_commonInitialized)
    return;

  m_commonInitialized=false;
}

bool mitk::GPUVolumeMapper3D::IsRenderable(mitk::BaseRenderer* renderer)
{

  if(!GetDataNode())
    return false;

  DataNode* node = GetDataNode();

  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");

  if(!visible) return false;

  bool value = false;
  if(!node->GetBoolProperty("volumerendering",value,renderer))
    return false;

  if(!value)
    return false;

  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );

  if ( !input || !input->IsInitialized() )
    return false;

  mitk::ImageVtkAccessor accessor(input);
  vtkImageData *inputData = accessor.getVtkImageData(this->GetTimestep());

  if(inputData==NULL)
    return false;

  return true;
}

void mitk::GPUVolumeMapper3D::InitVtkMapper(mitk::BaseRenderer* renderer)
{
  if(IsRAYEnabled(renderer))
  {
    DeinitCPU(renderer);
    if(!InitRAY(renderer))
    {
      GPU_WARN << "hardware renderer can't initialize ... falling back to software renderer";
      goto fallback;
    }
  }
  else if(IsGPUEnabled(renderer))
  {
    DeinitCPU(renderer);
    DeinitRAY(renderer);
    if(!InitGPU(renderer))
    {
      GPU_WARN << "hardware renderer can't initialize ... falling back to software renderer";
      goto fallback;
    }
  }
  else
  {
    fallback:
    DeinitRAY(renderer);
    InitCPU(renderer);
  }
}

vtkProp *mitk::GPUVolumeMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  if(!IsRenderable(renderer))
  {
    if(!m_VolumeNULL)
    {
      m_VolumeNULL = vtkSmartPointer<vtkVolume>::New();
      m_VolumeNULL->VisibilityOff();
    }
    return m_VolumeNULL;
  }

  InitCommon();
  InitVtkMapper( renderer );

  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(ls->m_rayInitialized)
    return ls->m_VolumeRAY;

  if(ls->m_gpuInitialized)
    return ls->m_VolumeGPU;

  return ls->m_VolumeCPU;
}


void mitk::GPUVolumeMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  if(!IsRenderable(renderer))
    return;

  InitCommon();
  InitVtkMapper( renderer );

  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );
  mitk::ImageVtkAccessor accessor(input);
  vtkImageData *inputData = accessor.getVtkImageData(this->GetTimestep());
  m_UnitSpacingImageFilter->SetInputData( inputData );

  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(ls->m_rayInitialized)
  {
    GenerateDataRAY(renderer);
  }
  else

  if(ls->m_gpuInitialized)
  {
    GenerateDataGPU(renderer);
  }
  else
  {
    GenerateDataCPU(renderer);
  }

  // UpdateTransferFunctions
  UpdateTransferFunctions( renderer );
}

void mitk::GPUVolumeMapper3D::GenerateDataGPU( mitk::BaseRenderer * /*renderer*/ )
{
}

void mitk::GPUVolumeMapper3D::GenerateDataCPU( mitk::BaseRenderer *renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  mitk::RenderingManager* rm = renderer->GetRenderingManager();
  int nextLod = rm != nullptr ? rm->GetNextLOD( renderer ) : 0;

  if( IsLODEnabled(renderer) && nextLod == 0 )
  {
    ls->m_MapperCPU->SetImageSampleDistance(3.5);
    ls->m_MapperCPU->SetSampleDistance(1.25);
    ls->m_VolumePropertyCPU->SetInterpolationTypeToNearest();
  }
  else
  {
    ls->m_MapperCPU->SetImageSampleDistance(1.0);
    ls->m_MapperCPU->SetSampleDistance(1.0);
    ls->m_VolumePropertyCPU->SetInterpolationTypeToLinear();
  }

  // Check raycasting mode
  if(IsMIPEnabled(renderer))
    ls->m_MapperCPU->SetBlendModeToMaximumIntensity();
  else
    ls->m_MapperCPU->SetBlendModeToComposite();

  // Updating shadings
  {
    float value=0;
    if(GetDataNode()->GetFloatProperty("volumerendering.cpu.ambient",value,renderer))
      ls->m_VolumePropertyCPU->SetAmbient(value);
    if(GetDataNode()->GetFloatProperty("volumerendering.cpu.diffuse",value,renderer))
      ls->m_VolumePropertyCPU->SetDiffuse(value);
    if(GetDataNode()->GetFloatProperty("volumerendering.cpu.specular",value,renderer))
      ls->m_VolumePropertyCPU->SetSpecular(value);
    if(GetDataNode()->GetFloatProperty("volumerendering.cpu.specular.power",value,renderer))
      ls->m_VolumePropertyCPU->SetSpecularPower(value);
  }
}

void mitk::GPUVolumeMapper3D::CreateDefaultTransferFunctions()
{
  m_DefaultOpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_DefaultOpacityTransferFunction->AddPoint( 0.0, 0.0 );
  m_DefaultOpacityTransferFunction->AddPoint( 255.0, 0.8 );
  m_DefaultOpacityTransferFunction->ClampingOn();

  m_DefaultGradientTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_DefaultGradientTransferFunction->AddPoint( 0.0, 0.0 );
  m_DefaultGradientTransferFunction->AddPoint( 255.0, 0.8 );
  m_DefaultGradientTransferFunction->ClampingOn();

  m_DefaultColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_DefaultColorTransferFunction->AddRGBPoint( 0.0, 0.0, 0.0, 0.0 );
  m_DefaultColorTransferFunction->AddRGBPoint( 127.5, 1, 1, 0.0 );
  m_DefaultColorTransferFunction->AddRGBPoint( 255.0, 0.8, 0.2, 0 );
  m_DefaultColorTransferFunction->ClampingOn();

  m_BinaryOpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_BinaryOpacityTransferFunction->AddPoint( 0, 0.0 );
  m_BinaryOpacityTransferFunction->AddPoint( 1, 1.0 );

  m_BinaryGradientTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_BinaryGradientTransferFunction->AddPoint( 0.0, 1.0 );

  m_BinaryColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
}

void mitk::GPUVolumeMapper3D::UpdateTransferFunctions( mitk::BaseRenderer * renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  vtkPiecewiseFunction *opacityTransferFunction = m_DefaultOpacityTransferFunction;
  vtkPiecewiseFunction *gradientTransferFunction = m_DefaultGradientTransferFunction;
  vtkColorTransferFunction *colorTransferFunction = m_DefaultColorTransferFunction;

  bool isBinary = false;

  GetDataNode()->GetBoolProperty("binary", isBinary, renderer);

  if(isBinary)
  {
    opacityTransferFunction = m_BinaryOpacityTransferFunction;
    gradientTransferFunction = m_BinaryGradientTransferFunction;
    colorTransferFunction = m_BinaryColorTransferFunction;

    colorTransferFunction->RemoveAllPoints();
    float rgb[3];
    if( !GetDataNode()->GetColor( rgb,renderer ) )
       rgb[0]=rgb[1]=rgb[2]=1;
    colorTransferFunction->AddRGBPoint( 0,rgb[0],rgb[1],rgb[2] );
    colorTransferFunction->Modified();
  }
  else
  {
    mitk::TransferFunctionProperty *transferFunctionProp =
      dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataNode()->GetProperty("TransferFunction",renderer));

    if( transferFunctionProp )
    {
      opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
      gradientTransferFunction = transferFunctionProp->GetValue()->GetGradientOpacityFunction();
      colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction();
    }
  }

  if(ls->m_gpuInitialized)
  {
    ls->m_VolumePropertyGPU->SetColor( colorTransferFunction );
    ls->m_VolumePropertyGPU->SetScalarOpacity( opacityTransferFunction );
    ls->m_VolumePropertyGPU->SetGradientOpacity( gradientTransferFunction );
  }

  if(ls->m_rayInitialized)
  {
    ls->m_VolumePropertyRAY->SetColor( colorTransferFunction );
    ls->m_VolumePropertyRAY->SetScalarOpacity( opacityTransferFunction );
    ls->m_VolumePropertyRAY->SetGradientOpacity( gradientTransferFunction );
  }

  if(ls->m_cpuInitialized)
  {
    ls->m_VolumePropertyCPU->SetColor( colorTransferFunction );
    ls->m_VolumePropertyCPU->SetScalarOpacity( opacityTransferFunction );
    ls->m_VolumePropertyCPU->SetGradientOpacity( gradientTransferFunction );
  }
}


void mitk::GPUVolumeMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* /*renderer*/)
{
  //GPU_INFO << "ApplyProperties";
}

void mitk::GPUVolumeMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  //GPU_INFO << "SetDefaultProperties";

  node->AddProperty( "volumerendering", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "volumerendering.usemip", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "volumerendering.uselod", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "volumerendering.cpu.ambient",  mitk::FloatProperty::New( 0.10f ), renderer, overwrite );
  node->AddProperty( "volumerendering.cpu.diffuse",  mitk::FloatProperty::New( 0.50f ), renderer, overwrite );
  node->AddProperty( "volumerendering.cpu.specular", mitk::FloatProperty::New( 0.40f ), renderer, overwrite );
  node->AddProperty( "volumerendering.cpu.specular.power", mitk::FloatProperty::New( 16.0f ), renderer, overwrite );
  bool usegpu = true;
#ifdef __APPLE__
  usegpu = false;
  node->AddProperty( "volumerendering.uselod", mitk::BoolProperty::New( true ), renderer, overwrite );
#endif
  node->AddProperty( "volumerendering.usegpu", mitk::BoolProperty::New( usegpu ), renderer, overwrite );

  node->AddProperty( "volumerendering.useray", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "volumerendering.ray.ambient",               mitk::FloatProperty::New( 0.25f ), renderer, overwrite );
  node->AddProperty( "volumerendering.ray.diffuse",               mitk::FloatProperty::New( 0.50f ), renderer, overwrite );
  node->AddProperty( "volumerendering.ray.specular",              mitk::FloatProperty::New( 0.40f ), renderer, overwrite );
  node->AddProperty( "volumerendering.ray.specular.power",        mitk::FloatProperty::New( 16.0f ), renderer, overwrite );

  node->AddProperty( "volumerendering.gpu.ambient",               mitk::FloatProperty::New( 0.25f ), renderer, overwrite );
  node->AddProperty( "volumerendering.gpu.diffuse",               mitk::FloatProperty::New( 0.50f ), renderer, overwrite );
  node->AddProperty( "volumerendering.gpu.specular",              mitk::FloatProperty::New( 0.40f ), renderer, overwrite );
  node->AddProperty( "volumerendering.gpu.specular.power",        mitk::FloatProperty::New( 16.0f ), renderer, overwrite );
  node->AddProperty( "volumerendering.gpu.usetexturecompression", mitk::BoolProperty ::New( false ), renderer, overwrite );
  node->AddProperty( "volumerendering.gpu.reducesliceartifacts" , mitk::BoolProperty ::New( false ), renderer, overwrite );

  node->AddProperty( "binary", mitk::BoolProperty::New( false ), renderer, overwrite );

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull() && image->IsInitialized())
  {
    if((overwrite) || (node->GetProperty("levelwindow", renderer)==NULL))
    {
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelwindow;
      levelwindow.SetAuto( image );
      levWinProp->SetLevelWindow( levelwindow );
      node->SetProperty( "levelwindow", levWinProp, renderer );
    }

    if((overwrite) || (node->GetProperty("TransferFunction", renderer)==NULL))
    {
      // add a default transfer function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      mitk::TransferFunctionInitializer::Pointer tfInit = mitk::TransferFunctionInitializer::New(tf);
      tfInit->SetTransferFunctionMode(0);
      node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );
    }
  }

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

bool mitk::GPUVolumeMapper3D::IsLODEnabled( mitk::BaseRenderer * renderer ) const
{
  bool value = false;
  return GetDataNode()->GetBoolProperty("volumerendering.uselod",value,renderer) && value;
}

bool mitk::GPUVolumeMapper3D::IsMIPEnabled( mitk::BaseRenderer * renderer )
{
  bool value = false;
  return GetDataNode()->GetBoolProperty("volumerendering.usemip",value,renderer) && value;
}

bool mitk::GPUVolumeMapper3D::IsGPUEnabled( mitk::BaseRenderer * renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool value = false;
  return ls->m_gpuSupported && GetDataNode()->GetBoolProperty("volumerendering.usegpu",value,renderer) && value;
}

bool mitk::GPUVolumeMapper3D::InitRAY(mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(ls->m_rayInitialized)
    return ls->m_raySupported;

  ls->m_VtkRenderWindow = renderer->GetVtkRenderer()->GetRenderWindow();

  GPU_INFO << "initializing gpu-raycast-vr (vtkOpenGLGPUVolumeRayCastMapper)";

  ls->m_MapperRAY = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
  ls->m_MapperRAY->SetAutoAdjustSampleDistances(0);
  ls->m_MapperRAY->SetSampleDistance(1.0);

  ls->m_VolumePropertyRAY = vtkSmartPointer<vtkVolumeProperty>::New();
  ls->m_VolumePropertyRAY->ShadeOn();
  ls->m_VolumePropertyRAY->SetAmbient (0.25f); //0.05f
  ls->m_VolumePropertyRAY->SetDiffuse (0.50f); //0.45f
  ls->m_VolumePropertyRAY->SetSpecular(0.40f); //0.50f
  ls->m_VolumePropertyRAY->SetSpecularPower(16.0f);
  ls->m_VolumePropertyRAY->SetInterpolationTypeToLinear();

  ls->m_VolumeRAY = vtkSmartPointer<vtkVolume>::New();
  ls->m_VolumeRAY->SetMapper( ls->m_MapperRAY );
  ls->m_VolumeRAY->SetProperty( ls->m_VolumePropertyRAY );
  ls->m_VolumeRAY->VisibilityOn();

  ls->m_MapperRAY->SetInputConnection( this->m_UnitSpacingImageFilter->GetOutputPort() );

  ls->m_raySupported = ls->m_MapperRAY->IsRenderSupported(renderer->GetRenderWindow(),ls->m_VolumePropertyRAY);


  ls->m_rayInitialized = true;
  return ls->m_raySupported;
}

void mitk::GPUVolumeMapper3D::DeinitRAY(mitk::BaseRenderer* renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  if(ls->m_rayInitialized)
  {
    GPU_INFO << "deinitializing gpu-raycast-vr";

    ls->m_MapperRAY = NULL;
    ls->m_VolumePropertyRAY = NULL;
    //Here ReleaseGraphicsResources has to be called to avoid VTK error messages.
    //This seems like a VTK bug, because ReleaseGraphicsResources() is ment for internal use,
    //but you cannot just delete the object (last smartpointer reference) without getting the
    //VTK error.
    ls->m_VolumeRAY->ReleaseGraphicsResources(renderer->GetVtkRenderer()->GetRenderWindow());
    ls->m_VolumeRAY = NULL;
    ls->m_rayInitialized=false;
  }
}

void mitk::GPUVolumeMapper3D::GenerateDataRAY( mitk::BaseRenderer *renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  mitk::RenderingManager* rm = renderer->GetRenderingManager();
  if( IsLODEnabled(renderer) && rm != nullptr && rm->GetNextLOD( renderer ) == 0 )
    ls->m_MapperRAY->SetImageSampleDistance(4.0);
  else
    ls->m_MapperRAY->SetImageSampleDistance(1.0);

  // Check raycasting mode
  if(IsMIPEnabled(renderer))
    ls->m_MapperRAY->SetBlendModeToMaximumIntensity();
  else
    ls->m_MapperRAY->SetBlendModeToComposite();

  // Updating shadings
  {
    float value=0;
    if(GetDataNode()->GetFloatProperty("volumerendering.ray.ambient",value,renderer))
      ls->m_VolumePropertyRAY->SetAmbient(value);
    if(GetDataNode()->GetFloatProperty("volumerendering.ray.diffuse",value,renderer))
      ls->m_VolumePropertyRAY->SetDiffuse(value);
    if(GetDataNode()->GetFloatProperty("volumerendering.ray.specular",value,renderer))
      ls->m_VolumePropertyRAY->SetSpecular(value);
    if(GetDataNode()->GetFloatProperty("volumerendering.ray.specular.power",value,renderer))
      ls->m_VolumePropertyRAY->SetSpecularPower(value);
  }

}

bool mitk::GPUVolumeMapper3D::IsRAYEnabled( mitk::BaseRenderer * renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool value = false;
  return ls->m_raySupported && GetDataNode()->GetBoolProperty("volumerendering.useray",value,renderer) && value;
}

