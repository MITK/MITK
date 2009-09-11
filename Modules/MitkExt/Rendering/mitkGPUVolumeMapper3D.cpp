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

const mitk::Image* mitk::GPUVolumeMapper3D::GetInput()
{
  return static_cast<const mitk::Image*> ( GetData() );
}

mitk::GPUVolumeMapper3D::GPUVolumeMapper3D()
: m_Mask( NULL )
{
/*
  m_PlaneSet = false;

  m_ClippingPlane = vtkPlane::New();
  m_PlaneWidget = vtkImplicitPlaneWidget::New();
  */
  
  m_T2DMapper =  vtkMitkOpenGLVolumeTextureMapper3D::New();
 // m_T2DMapper->SetMaximumNumberOfPlanes( 256 );
 
 m_T2DMapper->SetUseCompressedTexture(true);
 
 m_T2DMapper->SetPreferredMethodToFragmentProgram();
  m_T2DMapper->SetSampleDistance(1.0); // 4 rays for every pixel
 
  m_HiResMapper = vtkFixedPointVolumeRayCastMapper::New();
  m_HiResMapper->SetSampleDistance(1); // 4 rays for every pixel
  m_HiResMapper->SetImageSampleDistance(4); // 4 rays for every pixel

  m_HiResMapper->IntermixIntersectingGeometryOn();
  m_HiResMapper->SetNumberOfThreads( itk::MultiThreader::GetGlobalDefaultNumberOfThreads() );
  /*
  vtkVolumeRayCastCompositeFunction* compositeFunction = vtkVolumeRayCastCompositeFunction::New();
  compositeFunction->SetCompositeMethodToClassifyFirst();
  m_HiResMapper->SetVolumeRayCastFunction(compositeFunction);
  compositeFunction->Delete();

  vtkVolumeRayCastMIPFunction* mipFunction = vtkVolumeRayCastMIPFunction::New();
  m_HiResMapper->SetVolumeRayCastFunction(mipFunction);
  mipFunction->Delete();
 */

  vtkFiniteDifferenceGradientEstimator* gradientEstimator =
  vtkFiniteDifferenceGradientEstimator::New();
//  m_HiResMapper->SetGradientEstimator(gradientEstimator);
  gradientEstimator->Delete();

  m_VolumePropertyLow = vtkVolumeProperty::New();
  m_VolumePropertyMed = vtkVolumeProperty::New();
  m_VolumePropertyHigh = vtkVolumeProperty::New();

    m_VolumePropertyLow->ShadeOn();
    m_VolumePropertyLow->SetAmbient(0.1f);
    m_VolumePropertyLow->SetDiffuse(0.7f);
    m_VolumePropertyLow->SetSpecular(0.2f);
    m_VolumePropertyLow->SetSpecularPower(10.0f);
    
    
                                                /*
    m_VolumePropertyMed->ShadeOn();
    m_VolumePropertyMed->SetAmbient(0.0f);
    m_VolumePropertyMed->SetDiffuse(1.00f);
    m_VolumePropertyMed->SetSpecular(0.0f);
    m_VolumePropertyMed->SetSpecularPower(1.0f);
                                              */
  m_VolumeLOD = vtkVolume::New();

  m_VolumeLOD->SetMapper(   m_T2DMapper );
  m_VolumeLOD->SetProperty(m_VolumePropertyLow );
  
  

  // m_LowResID = m_VolumeLOD->AddLOD(m_T2DMapper,m_VolumePropertyLow,0.0); // TextureMapper2D
  // m_MedResID = m_VolumeLOD->AddLOD(m_HiResMapper,m_VolumePropertyMed,0.0); // RayCast
  // m_HiResID = m_VolumeLOD->AddLOD(m_HiResMapper,m_VolumePropertyHigh,0.0); // RayCast

  m_Resampler = vtkImageResample::New();
  m_Resampler->SetAxisMagnificationFactor(0,0.25);
  m_Resampler->SetAxisMagnificationFactor(1,0.25);
  m_Resampler->SetAxisMagnificationFactor(2,0.25);

  // For abort rendering mechanism
  // m_VolumeLOD->AutomaticLODSelectionOff();

/*

  m_BoundingBox = vtkCubeSource::New();
  m_BoundingBox->SetXLength( 0.0 );
  m_BoundingBox->SetYLength( 0.0 );
  m_BoundingBox->SetZLength( 0.0 );

  m_BoundingBoxMapper = vtkPolyDataMapper::New();
  m_BoundingBoxMapper->SetInput( m_BoundingBox->GetOutput() );

  m_BoundingBoxActor = vtkActor::New();
  m_BoundingBoxActor->SetMapper( m_BoundingBoxMapper );
  m_BoundingBoxActor->GetProperty()->SetColor( 1.0, 1.0, 1.0 );
  m_BoundingBoxActor->GetProperty()->SetRepresentationToWireframe();
  */

  // BoundingBox rendering is not working due to problem with assembly
  // transformation; see bug #454
  // If commenting in the following, do not forget to comment in the
  // m_Prop3DAssembly->Delete() line in the destructor.
  //m_Prop3DAssembly = vtkAssembly::New();
  //m_Prop3DAssembly->AddPart( m_VolumeLOD );
  //m_Prop3DAssembly->AddPart( m_BoundingBoxActor );
  //m_Prop3D = m_Prop3DAssembly;

  m_Prop3D = m_VolumeLOD;
  m_Prop3D->Register(NULL);
                                 /*
  m_ImageCast = vtkImageShiftScale::New();
  m_ImageCast->SetOutputScalarTypeToUnsignedShort();
  m_ImageCast->ClampOverflowOn();
                                   */
  m_UnitSpacingImageFilter = vtkImageChangeInformation::New();
//  m_UnitSpacingImageFilter->SetInput(m_ImageCast->GetOutput());
  m_UnitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );
                                                /*
  m_ImageMaskFilter = vtkImageMask::New();
  m_ImageMaskFilter->SetMaskedOutputValue(0xffff);
                                                  */
  //this->m_Resampler->SetInput( this->m_UnitSpacingImageFilter->GetOutput() );
  this->m_HiResMapper->SetInput( this->m_UnitSpacingImageFilter->GetOutput() );

  m_T2DMapper->SetInput( this->m_UnitSpacingImageFilter->GetOutput() );//m_Resampler->GetOutput());


  this->CreateDefaultTransferFunctions();
}


mitk::GPUVolumeMapper3D::~GPUVolumeMapper3D()
{
  m_UnitSpacingImageFilter->Delete();
  //m_ImageCast->Delete();
  m_T2DMapper->Delete();
  m_HiResMapper->Delete();
  m_Resampler->Delete();
  m_VolumePropertyLow->Delete();
  m_VolumePropertyMed->Delete();
  m_VolumePropertyHigh->Delete();
  m_VolumeLOD->Delete();
//  m_ClippingPlane->Delete();
//  m_PlaneWidget->Delete();
  // m_Prop3DAssembly->Delete();
/*
  m_BoundingBox->Delete();
  m_BoundingBoxMapper->Delete();
  m_BoundingBoxActor->Delete();
  */
  //m_ImageMaskFilter->Delete();
  m_DefaultColorTransferFunction->Delete();
  m_DefaultOpacityTransferFunction->Delete();
  m_DefaultGradientTransferFunction->Delete();

  if (m_Mask)
  {
    m_Mask->Delete();
  }
}

void mitk::GPUVolumeMapper3D::GenerateData( mitk::BaseRenderer *renderer )
{
  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );
  if ( !input || !input->IsInitialized() )
    return;

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();

  bool volumeRenderingEnabled = true;

  if (this->IsVisible(renderer)==false ||
      this->GetDataTreeNode() == NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))==NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))->GetValue() == false
    )
  {
    volumeRenderingEnabled = false;

/*
    // Check if a bounding box should be displayed around the dataset
    // (even if volume rendering is disabled)
    bool hasBoundingBox = false;
    this->GetDataTreeNode()->GetBoolProperty( "bounding box", hasBoundingBox );

    if ( !hasBoundingBox )
    {
      m_BoundingBoxActor->VisibilityOff();
    }
    else
    {
      m_BoundingBoxActor->VisibilityOn();

      const BoundingBox::BoundsArrayType &bounds =
        input->GetTimeSlicedGeometry()->GetBounds();

      m_BoundingBox->SetBounds(
        bounds[0], bounds[1],
        bounds[2], bounds[3],
        bounds[4], bounds[5] );

      ColorProperty *colorProperty;
      if ( this->GetDataTreeNode()->GetProperty(
        colorProperty, "color" ) )
      {
        const mitk::Color &color = colorProperty->GetColor();
        m_BoundingBoxActor->GetProperty()->SetColor(
          color[0], color[1], color[2] );
      }
      else
      {
        m_BoundingBoxActor->GetProperty()->SetColor(
          1.0, 1.0, 1.0 );
      }
    }
    */
  }

  // Don't do anything if VR is disabled
  if ( !volumeRenderingEnabled )
  {
    m_VolumeLOD->VisibilityOff();
    return;
  }

  m_VolumeLOD->VisibilityOn();
  
  // Setting HighResMapper raycast function
  {
    int renderingValue = dynamic_cast<mitk::VtkVolumeRenderingProperty*>(GetDataTreeNode()->GetProperty("volumerendering configuration",renderer))->GetValueAsId();

    switch(renderingValue)
    {
      case  VTK_VOLUME_RAY_CAST_MIP_FUNCTION:
        m_HiResMapper->SetBlendModeToMaximumIntensity();
        break;
      case VTK_RAY_CAST_COMPOSITE_FUNCTION:
        m_HiResMapper->SetBlendModeToComposite();
        break;
      default:
        LOG_WARN << "invalid volume rendering option";
        m_HiResMapper->SetBlendModeToComposite();
        break;
    }
  }

  this->SetPreferences();
                 
  switch ( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) )
  {
    default:
      LOG_WARN << "invalid lod level";
    case 1:
      m_T2DMapper->SetSampleDistance(0.75);
      break;

    case 0:
      m_T2DMapper->SetSampleDistance(2.0);
      break;
  }
              
  const TimeSlicedGeometry* inputtimegeometry = input->GetTimeSlicedGeometry();
  assert(inputtimegeometry!=NULL);

  const Geometry3D* worldgeometry = renderer->GetCurrentWorldGeometry();
  if(worldgeometry==NULL)
  {
    LOG_WARN << "no world geometry found, turning off volumerendering";
    GetDataTreeNode()->SetProperty("volumerendering",mitk::BoolProperty::New(false));
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


  m_UnitSpacingImageFilter->SetInput( inputData );
                          /*
  //If mask exists, process mask before resampling.
  if (this->m_Mask)
  {
    this->m_ImageMaskFilter->SetImageInput(this->m_UnitSpacingImageFilter->GetOutput());
    this->m_Resampler->SetInput(this->m_ImageMaskFilter->GetOutput());
    this->m_HiResMapper->SetInput(this->m_ImageMaskFilter->GetOutput());
  }
  else
  {
    this->m_Resampler->SetInput(this->m_UnitSpacingImageFilter->GetOutput());
    this->m_HiResMapper->SetInput(this->m_UnitSpacingImageFilter->GetOutput());
  }
                            */
  this->UpdateTransferFunctions( renderer );

/*

  vtkRenderWindowInteractor *interactor = renderWindow->GetInteractor();
  
  float frameRate;
  if( this->GetDataTreeNode()->GetFloatProperty( "framerate", frameRate ) && frameRate > 0 && frameRate <= 60)
  {
    interactor->SetDesiredUpdateRate(  frameRate );
    interactor->SetStillUpdateRate( frameRate );
  }
  else if( frameRate > 60 )
  {
    this->GetDataTreeNode()->SetProperty( "framerate",mitk::FloatProperty::New(60));
    interactor->SetDesiredUpdateRate(  60 );
    interactor->SetStillUpdateRate( 60 );
  }
  else
  {
    this->GetDataTreeNode()->SetProperty( "framerate",mitk::FloatProperty::New(0.00001));
    interactor->SetDesiredUpdateRate(  0.00001 );
    interactor->SetStillUpdateRate( 0.00001 );
  }
  
  */
             /*
  if ( m_RenderWindowInitialized.find( renderWindow ) == m_RenderWindowInitialized.end() )
  {
    m_RenderWindowInitialized.insert( renderWindow );

    mitk::RenderingManager::GetInstance()->SetNextLOD( 0, renderer );

    mitk::RenderingManager::GetInstance()->SetShading( true, 0 );
    mitk::RenderingManager::GetInstance()->SetShading( true, 1 );
   // mitk::RenderingManager::GetInstance()->SetShading( true, 2 );

    mitk::RenderingManager::GetInstance()->SetShadingValues(
      m_VolumePropertyHigh->GetAmbient(),
      m_VolumePropertyHigh->GetDiffuse(),
      m_VolumePropertyHigh->GetSpecular(),
      m_VolumePropertyHigh->GetSpecularPower());

    mitk::RenderingManager::GetInstance()->SetClippingPlaneStatus(false);
  }

  this->SetClippingPlane( interactor );    */
}


void mitk::GPUVolumeMapper3D::CreateDefaultTransferFunctions()
{
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
  vtkPiecewiseFunction *opacityTransferFunction = NULL;
  vtkPiecewiseFunction *gradientTransferFunction = NULL;
  vtkColorTransferFunction *colorTransferFunction = NULL;

  mitk::LookupTableProperty::Pointer lookupTableProp;
  lookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataTreeNode()->GetProperty("LookupTable"));
  mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction"));

  if ( transferFunctionProp.IsNotNull() )   {

    opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
    gradientTransferFunction = transferFunctionProp->GetValue()->GetGradientOpacityFunction();
    colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction();
  }
  /*
  else if (lookupTableProp.IsNotNull() )
  {
    lookupTableProp->GetLookupTable()->CreateOpacityTransferFunction(opacityTransferFunction);
    opacityTransferFunction->ClampingOn();
    lookupTableProp->GetLookupTable()->CreateGradientTransferFunction(gradientTransferFunction);
    gradientTransferFunction->ClampingOn();
    lookupTableProp->GetLookupTable()->CreateColorTransferFunction(colorTransferFunction);
    colorTransferFunction->ClampingOn();
  } */
  else
  {
    opacityTransferFunction = m_DefaultOpacityTransferFunction;
    gradientTransferFunction = m_DefaultGradientTransferFunction;
    colorTransferFunction = m_DefaultColorTransferFunction;

    float rgb[3]={1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    if(GetColor(rgb, renderer))
    {
      colorTransferFunction->AddRGBPoint( 0.0, 0.0, 0.0, 0.0 );
      colorTransferFunction->AddRGBPoint( 127.5, rgb[0], rgb[1], rgb[2] );
      colorTransferFunction->AddRGBPoint( 255.0, rgb[0], rgb[1], rgb[2] );
    }
  }

  if (this->m_Mask)
  {
    opacityTransferFunction->AddPoint(0xffff, 0.0);
  }

  m_VolumePropertyLow->SetColor( colorTransferFunction );
  m_VolumePropertyLow->SetScalarOpacity( opacityTransferFunction );
//  m_VolumePropertyLow->SetGradientOpacity( gradientTransferFunction );
  m_VolumePropertyLow->SetInterpolationTypeToLinear();

/*
  m_VolumePropertyMed->SetColor( colorTransferFunction );
  m_VolumePropertyMed->SetScalarOpacity( opacityTransferFunction );
  m_VolumePropertyMed->SetGradientOpacity( gradientTransferFunction );
  m_VolumePropertyMed->SetInterpolationTypeToLinear();

  m_VolumePropertyHigh->SetColor( colorTransferFunction );
  m_VolumePropertyHigh->SetScalarOpacity( opacityTransferFunction );
  m_VolumePropertyHigh->SetGradientOpacity( gradientTransferFunction );
  m_VolumePropertyHigh->SetInterpolationTypeToLinear();
*/
}

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* Shading enabled / disabled */
void mitk::GPUVolumeMapper3D::SetPreferences()
{
/*
    LOG_INFO << mitk::RenderingManager::GetInstance()->GetShadingValues()[0] << "\n"
             << mitk::RenderingManager::GetInstance()->GetShadingValues()[1] << "\n"
             << mitk::RenderingManager::GetInstance()->GetShadingValues()[2] << "\n"
             << mitk::RenderingManager::GetInstance()->GetShadingValues()[3] << "\n";
*/    
/*
  //LOD 0
  if( mitk::RenderingManager::GetInstance()->GetShading(0))
  {
    m_VolumePropertyLow->ShadeOn();
    m_VolumePropertyLow->SetAmbient(0.0f);
    m_VolumePropertyLow->SetDiffuse(1.00f);
    m_VolumePropertyLow->SetSpecular(0.0f);
    m_VolumePropertyLow->SetSpecularPower(1.0f);
  }
  else
  {
    m_VolumePropertyLow->ShadeOff();
  }

  //LOD 1
  
  if( mitk::RenderingManager::GetInstance()->GetShading(1))
  {
    m_VolumePropertyMed->ShadeOn();
    m_VolumePropertyMed->SetAmbient(0.0f);
    m_VolumePropertyMed->SetDiffuse(1.00f);
    m_VolumePropertyMed->SetSpecular(0.0f);
    m_VolumePropertyMed->SetSpecularPower(1.0f);
  }
  else   
  {
    m_VolumePropertyMed->ShadeOff();
  }
//*                  
  //LOD 2
  if(mitk::RenderingManager::GetInstance()->GetShading(2))
  {
    m_VolumePropertyHigh->ShadeOn();
    //Shading Properties
    m_VolumePropertyHigh->SetAmbient(mitk::RenderingManager::GetInstance()->GetShadingValues()[0]);
    m_VolumePropertyHigh->SetDiffuse(mitk::RenderingManager::GetInstance()->GetShadingValues()[1]);
    m_VolumePropertyHigh->SetSpecular(mitk::RenderingManager::GetInstance()->GetShadingValues()[2]);
    m_VolumePropertyHigh->SetSpecularPower(mitk::RenderingManager::GetInstance()->GetShadingValues()[3]);
  }
  else 
  {
    m_VolumePropertyHigh->ShadeOff();
  }
                    */
}

/* Adds A Clipping Plane to the Mapper */
void mitk::GPUVolumeMapper3D::SetClippingPlane(vtkRenderWindowInteractor* interactor)
{
/*
  if(mitk::RenderingManager::GetInstance()->GetClippingPlaneStatus()) //if clipping plane is enabled
  {

    if(!m_PlaneSet)
    {
    m_PlaneWidget->SetInteractor(interactor);
    m_PlaneWidget->SetPlaceFactor(1.0);
    m_PlaneWidget->SetInput(m_UnitSpacingImageFilter->GetOutput());
    m_PlaneWidget->OutlineTranslationOff(); //disables scaling of the bounding box
#if (VTK_MAJOR_VERSION >= 5)
    m_PlaneWidget->ScaleEnabledOff(); //disables scaling of the bounding box
#endif
    m_PlaneWidget->DrawPlaneOff(); //clipping plane is transparent
    mitk::Image* input  = const_cast<mitk::Image *>(this->GetInput());

  //
    m_PlaneWidget->PlaceWidget(
        input->GetGeometry()->GetOrigin()[0],(input->GetGeometry()->GetOrigin()[0])+(input->GetDimension(0))*(input->GetVtkImageData()->GetSpacing()[0]),                               input->GetGeometry()->GetOrigin()[1],(input->GetGeometry()->GetOrigin()[1])+(input->GetDimension(1))*(input->GetVtkImageData()->GetSpacing()[1]),                               input->GetGeometry()->GetOrigin()[2],(input->GetGeometry()->GetOrigin()[2])+(input->GetDimension(2))*(input->GetVtkImageData()->GetSpacing()[2]));

    m_T2DMapper->AddClippingPlane(m_ClippingPlane);
    m_HiResMapper->AddClippingPlane(m_ClippingPlane);
    }

    m_PlaneWidget->GetPlane(m_ClippingPlane);
    m_PlaneSet = true;
  }
  else //if clippingplane is disabled
  {
    if(m_PlaneSet) //if plane exists
    {
    DelClippingPlane();
    }
  }
    */
}


/* Removes the clipping plane */
void mitk::GPUVolumeMapper3D::DelClippingPlane()
{
/*
  m_T2DMapper->RemoveAllClippingPlanes();
  m_HiResMapper->RemoveAllClippingPlanes();
  m_PlaneSet = false;
*/
}

void mitk::GPUVolumeMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* /*renderer*/)
{

}

void mitk::GPUVolumeMapper3D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
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
    if((overwrite) || (node->GetProperty("LookupTable", renderer)==NULL))
    {
      // add a default rainbow lookup table for color mapping
      mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
      vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
      vtkLut->SetHueRange(0.6667, 0.0);
      vtkLut->SetTableRange(0.0, 20.0);
      vtkLut->Build();
      mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
      mitkLutProp->SetLookupTable(mitkLut);
      node->SetProperty( "LookupTable", mitkLutProp );
    }
    if((overwrite) || (node->GetProperty("TransferFunction", renderer)==NULL))
    {
      // add a default transfer function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();

      /* //old Transfer Function Version
      float m_Min = image->GetScalarValueMin();
      float m_Max = image->GetScalarValueMax();

      tf->GetScalarOpacityFunction()->Initialize();
      tf->GetScalarOpacityFunction()->AddPoint ( m_Min, 0 );
      tf->GetScalarOpacityFunction()->AddPoint ( m_Max, 1 );
      tf->GetColorTransferFunction()->AddRGBPoint(m_Min,1,0,0);
      tf->GetColorTransferFunction()->AddRGBPoint(m_Max,1,1,0);
      tf->GetGradientOpacityFunction()->Initialize();
      tf->GetGradientOpacityFunction()->AddPoint(m_Min,1.0);
      tf->GetGradientOpacityFunction()->AddPoint(0.0,1.0);
      tf->GetGradientOpacityFunction()->AddPoint((m_Max*0.25),1.0);
      tf->GetGradientOpacityFunction()->AddPoint(m_Max,1.0);
      */

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION >= 2)  )
      
      //CT_AAA for VTK Version >= 5.2
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

#else
      //CT_AAA for VTK Version < 5.2
      tf->GetColorTransferFunction()->AddRGBPoint( -3024, 0, 0, 0 );
      tf->GetColorTransferFunction()->AddRGBPoint( 143.556, 0.615686, 0.356863, 0.184314 );
      tf->GetColorTransferFunction()->AddRGBPoint( 166.222, 0.882353, 0.603922, 0.290196 );
      tf->GetColorTransferFunction()->AddRGBPoint( 214.389, 1, 1, 1 );
      tf->GetColorTransferFunction()->AddRGBPoint( 419.736, 1, 0.937033, 0.954531 );
      tf->GetColorTransferFunction()->AddRGBPoint( 3071, 0.827451, 0.658824, 1 );

      tf->GetScalarOpacityFunction()->Initialize();
      tf->GetScalarOpacityFunction()->AddPoint( -3024, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 143.556, 0 );
      tf->GetScalarOpacityFunction()->AddPoint( 166.222, 0.686275 );
      tf->GetScalarOpacityFunction()->AddPoint( 214.389, 0.696078 );
      tf->GetScalarOpacityFunction()->AddPoint( 419.736, 0.833333 );
      tf->GetScalarOpacityFunction()->AddPoint( 3071, 0.803922 );

      tf->GetGradientOpacityFunction()->Initialize();
      tf->GetGradientOpacityFunction()->AddPoint( 0, 1 );
      tf->GetGradientOpacityFunction()->AddPoint( 255, 1 );

#endif

      node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );
    }
  }

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}


bool mitk::GPUVolumeMapper3D::IsLODEnabled( mitk::BaseRenderer *renderer ) const
{
  // Volume mapper is LOD enabled if volumerendering is enabled
  return
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer)) != NULL &&
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))->GetValue() == true;
}


void mitk::GPUVolumeMapper3D::EnableMask()
{
  if (!this->m_Mask)
  {
    const Image *orig_image = this->GetInput();
    unsigned int *dimensions = orig_image->GetDimensions();

    this->m_Mask = vtkImageData::New();
    this->m_Mask->SetDimensions(dimensions[0], dimensions[1], dimensions[2]);
    this->m_Mask->SetScalarTypeToUnsignedChar();
    this->m_Mask->SetNumberOfScalarComponents(1);
    this->m_Mask->AllocateScalars();

    unsigned char *mask_data = static_cast<unsigned char*>(this->m_Mask->GetScalarPointer());
    unsigned int size = dimensions[0] * dimensions[1] * dimensions[2];

    for (unsigned int i = 0u; i < size; ++i)
    {
      *mask_data++ = 1u;
    }
    this->m_ImageMaskFilter->SetMaskInput(this->m_Mask);
    this->m_ImageMaskFilter->Modified();
  }
}

void mitk::GPUVolumeMapper3D::DisableMask()
{
  if (this->m_Mask)
  {
    this->m_Mask->Delete();
    this->m_Mask = 0;
  }
}

mitk::Image::Pointer mitk::GPUVolumeMapper3D::GetMask()
{
  if (this->m_Mask)
  {
    Image::Pointer mask = Image::New();

    mask->Initialize(this->m_Mask);
    mask->SetImportVolume(this->m_Mask->GetScalarPointer(), 0, 0, Image::ReferenceMemory);
    mask->SetGeometry(this->GetInput()->GetGeometry());
    return mask;
  }

  return 0;
}

void mitk::GPUVolumeMapper3D::UpdateMask()
{
  if (this->m_Mask)
  {
    this->m_ImageMaskFilter->Modified();
  }
}

bool mitk::GPUVolumeMapper3D::SetMask(const mitk::Image* mask)
{
  if (this->m_Mask)
  {
    if (mask->GetPixelType() == PixelType(typeid(unsigned char)))
    {
      Image *img = const_cast<Image*>(mask);

      this->m_Mask->DeepCopy(img->GetVtkImageData());
      this->m_ImageMaskFilter->Modified();
      return true;
    }
  }

  return false;
}
