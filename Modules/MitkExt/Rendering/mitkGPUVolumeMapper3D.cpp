/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 15:59:04 +0200 (Thu, 18 Jun 2009) $
Version:   $Revision: 17786 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkGPUVolumeMapper3D.h"

#include "mitkDataTreeNode.h"

#include "mitkProperties.h"
#include "mitkLevelWindow.h"
#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkColorProperty.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkVolumeRayCastMapper.h>

#include <vtkVolumeTextureMapper2D.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>
#include <vtkFiniteDifferenceGradientEstimator.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageShiftScale.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageWriter.h>
#include <vtkImageData.h>
#include <vtkLODProp3D.h>
#include <vtkImageResample.h>
#include <vtkPlane.h>
#include <vtkImplicitPlaneWidget.h>
#include <vtkAssembly.h>
#include <vtkFixedPointVolumeRayCastMapper.h>

#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include "mitkVtkVolumeRenderingProperty.h"


#include <itkMultiThreader.h>

#include "vtkMitkOpenGLVolumeTextureMapper3D.h"


#define GPU_LOG LOG_INFO(false)("VR")

const mitk::Image* mitk::GPUVolumeMapper3D::GetInput()
{
  return static_cast<const mitk::Image*> ( GetData() );
}

mitk::GPUVolumeMapper3D::GPUVolumeMapper3D()
{
  GPU_LOG << "Instantiating GPUVolumeMapper3D";
  
  m_T2DMapper =  vtkMitkOpenGLVolumeTextureMapper3D::New();
  m_T2DMapper->SetUseCompressedTexture(true);
  m_T2DMapper->SetPreferredMethodToFragmentProgram();
  m_T2DMapper->SetSampleDistance(1.0); 
 
  m_VolumePropertyLow = vtkVolumeProperty::New();

  m_VolumePropertyLow->ShadeOn();
  m_VolumePropertyLow->SetAmbient(0.2f);
  m_VolumePropertyLow->SetDiffuse(0.5f);
  m_VolumePropertyLow->SetSpecular(0.3f);
  m_VolumePropertyLow->SetSpecularPower(10.0f);
  m_VolumePropertyLow->SetInterpolationTypeToLinear();
    
  m_VolumeLOD = vtkVolume::New();
  m_VolumeLOD->SetMapper(   m_T2DMapper );
  m_VolumeLOD->SetProperty(m_VolumePropertyLow );
                    
  m_UnitSpacingImageFilter = vtkImageChangeInformation::New();
  m_UnitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );

  m_T2DMapper->SetInput( this->m_UnitSpacingImageFilter->GetOutput() );//m_Resampler->GetOutput());

  CreateDefaultTransferFunctions();
}


mitk::GPUVolumeMapper3D::~GPUVolumeMapper3D()
{
  GPU_LOG << "Destroying GPUVolumeMapper3D";

  m_UnitSpacingImageFilter->Delete();
  m_T2DMapper->Delete();
  m_VolumePropertyLow->Delete();
  m_VolumeLOD->Delete();
  m_DefaultColorTransferFunction->Delete();
  m_DefaultOpacityTransferFunction->Delete();
  m_DefaultGradientTransferFunction->Delete();
}

vtkProp *mitk::GPUVolumeMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return m_VolumeLOD;
}



void mitk::GPUVolumeMapper3D::GenerateData( mitk::BaseRenderer *renderer )
{
  GPU_LOG << "GenerateData";
  
  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );
  
  GPU_LOG << "mitk image mtime: " << input->GetMTime();
  
  if ( !input || !input->IsInitialized() )
    return;

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();

  if (this->IsVisible(renderer)==false ||
      this->GetDataTreeNode() == NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))==NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))->GetValue() == false
    )
  {
    m_VolumeLOD->VisibilityOff();
    return;
  }

  m_VolumeLOD->VisibilityOn();
  
  const TimeSlicedGeometry* inputtimegeometry = input->GetTimeSlicedGeometry();
  assert(inputtimegeometry!=NULL);

  const Geometry3D* worldgeometry = renderer->GetCurrentWorldGeometry();
  if(worldgeometry==NULL)
  {
    LOG_WARN << "no world geometry found, turning off volumerendering for data node " << GetDataTreeNode()->GetName();
    GetDataTreeNode()->SetProperty("volumerendering",mitk::BoolProperty::New(false));
    m_VolumeLOD->VisibilityOff();
    return;
  }

  int timestep=0;
  ScalarType time = worldgeometry->GetTimeBounds()[0];
  if (time> ScalarTypeNumericTraits::NonpositiveMin())
    timestep = inputtimegeometry->MSToTimeStep(time);

  if (inputtimegeometry->IsValidTime(timestep)==false)
    return;

  vtkImageData *inputData = input->GetVtkImageData(timestep);
  if(inputData==NULL)
    return;

   GPU_LOG << "input data mtime1: " << inputData->GetMTime();

   m_UnitSpacingImageFilter->SetInput( inputData );
              
   UpdateTransferFunctions( renderer );
}


void mitk::GPUVolumeMapper3D::CreateDefaultTransferFunctions()
{
  GPU_LOG << "CreateDefaultTransferFunctions";

  m_DefaultOpacityTransferFunction = vtkPiecewiseFunction::New();
  m_DefaultOpacityTransferFunction->AddPoint( 0.0, 0.0 );
  m_DefaultOpacityTransferFunction->AddPoint( 255.0, 0.8 );
  m_DefaultOpacityTransferFunction->ClampingOn();

  m_DefaultGradientTransferFunction = vtkPiecewiseFunction::New();
  m_DefaultGradientTransferFunction->AddPoint( 0.0, 0.0 );
  m_DefaultGradientTransferFunction->AddPoint( 255.0, 0.8 );
  m_DefaultGradientTransferFunction->ClampingOn();

  m_DefaultColorTransferFunction = vtkColorTransferFunction::New();
  m_DefaultColorTransferFunction->AddRGBPoint( 0.0, 0.0, 0.0, 0.0 );
  m_DefaultColorTransferFunction->AddRGBPoint( 127.5, 1, 1, 0.0 );
  m_DefaultColorTransferFunction->AddRGBPoint( 255.0, 0.8, 0.2, 0 );
  m_DefaultColorTransferFunction->ClampingOn();
}

void mitk::GPUVolumeMapper3D::UpdateTransferFunctions( mitk::BaseRenderer *renderer )
{
  GPU_LOG << "UpdateTransferFunctions";

  vtkPiecewiseFunction *opacityTransferFunction = m_DefaultOpacityTransferFunction;
  vtkPiecewiseFunction *gradientTransferFunction = m_DefaultGradientTransferFunction;
  vtkColorTransferFunction *colorTransferFunction = m_DefaultColorTransferFunction;

  mitk::TransferFunctionProperty::Pointer transferFunctionProp = 
    dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction"));

  if( transferFunctionProp.IsNotNull() )   
  {

    opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
    gradientTransferFunction = transferFunctionProp->GetValue()->GetGradientOpacityFunction();
    colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction();
  }


  m_VolumePropertyLow->SetColor( colorTransferFunction );
  m_VolumePropertyLow->SetScalarOpacity( opacityTransferFunction );
  m_VolumePropertyLow->SetGradientOpacity( gradientTransferFunction );
}

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>



void mitk::GPUVolumeMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* /*renderer*/)
{
  GPU_LOG << "ApplyProperties";

}

void mitk::GPUVolumeMapper3D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  GPU_LOG << "SetDefaultProperties";

  node->AddProperty( "volumerendering", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "volumerendering configuration", mitk::VtkVolumeRenderingProperty::New( 1 ), renderer, overwrite );
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

      tf->GetColorTransferFunction()->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0 );
      tf->GetColorTransferFunction()->AddRGBPoint( 143.556, 0.615686, 0.356863, 0.184314, 0.5, 0 );
      tf->GetColorTransferFunction()->AddRGBPoint( 166.222, 0.882353, 0.603922, 0.290196, 0.5, 0 );
      tf->GetColorTransferFunction()->AddRGBPoint( 214.389, 1, 1, 1, 0.5, 0 );
      tf->GetColorTransferFunction()->AddRGBPoint( 419.736, 1, 0.937033, 0.954531, 0.5, 0 );
      tf->GetColorTransferFunction()->AddRGBPoint( 3071, 0.827451, 0.658824, 1, 0.5, 0 );

      tf->GetScalarOpacityFunction()->Initialize();
      tf->GetScalarOpacityFunction()->AddPoint( -3024, 0, 0.5, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 143.556, 0, 0.5, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 166.222, 0.686275, 0.5, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 214.389, 0.696078, 0.5, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 419.736, 0.833333, 0.5, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 3071, 0.803922, 0.5, 0 );

      tf->GetGradientOpacityFunction()->Initialize();
      tf->GetGradientOpacityFunction()->AddPoint( 0, 1, 0.5, 0 );
      tf->GetGradientOpacityFunction()->AddPoint( 255, 1, 0.5, 0);

      node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );
    }
  }

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}


bool mitk::GPUVolumeMapper3D::IsLODEnabled( mitk::BaseRenderer *renderer ) const
{
 GPU_LOG << "IsLODEnabled";
 return false;
}


