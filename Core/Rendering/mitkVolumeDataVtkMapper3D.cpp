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

#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkLevelWindow.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunctionProperty.h"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkFiniteDifferenceGradientEstimator.h>
#include <vtkRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkImageShiftScale.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageWriter.h>
#include <vtkVolumeTextureMapper2D.h> 
#include <vtkLODProp3D.h>
#include <vtkImageResample.h>
#include <vtkRenderWindow.h>
#include <mitkVtkRenderWindow.h>


const mitk::Image* mitk::VolumeDataVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Image*> ( GetData() );
}

mitk::VolumeDataVtkMapper3D::VolumeDataVtkMapper3D()
{
  
  
  
  //  m_VtkVolumeMapper = vtkVolumeRayCastMapper::New();
	// m_VtkVolumeMapper = vtkVolumeTextureMapper2D::New();

  m_LowResMapper =  vtkVolumeTextureMapper2D::New();
  m_MedResMapper =  vtkVolumeTextureMapper2D::New();
  m_HiResMapper = vtkVolumeRayCastMapper::New();
        
  vtkVolumeRayCastCompositeFunction* compositeFunction = vtkVolumeRayCastCompositeFunction::New();
  m_HiResMapper->SetVolumeRayCastFunction(compositeFunction);
  compositeFunction->Delete();
  vtkFiniteDifferenceGradientEstimator* gradientEstimator = 
  vtkFiniteDifferenceGradientEstimator::New();
/* if (dynamic_cast<vtkVolumeRayCastMapper*>(m_VtkVolumeMapper)){ 
  dynamic_cast<vtkVolumeRayCastMapper*>(m_VtkVolumeMapper)->SetGradientEstimator(gradientEstimator);
  } else if (dynamic_cast<vtkVolumeTextureMapper2D*>(m_VtkVolumeMapper)){ 
  dynamic_cast<vtkVolumeTextureMapper2D*>(m_VtkVolumeMapper)->SetGradientEstimator(gradientEstimator);
  } */ 
  m_LowResMapper->SetGradientEstimator(gradientEstimator);
  m_MedResMapper->SetGradientEstimator(gradientEstimator);
  m_HiResMapper->SetGradientEstimator(gradientEstimator);

  gradientEstimator->Delete();

  m_VolumeProperty = vtkVolumeProperty::New();
  m_VolumeLOD = vtkLODProp3D::New();
  m_VolumeLOD->AddLOD(m_LowResMapper,m_VolumeProperty,0.0);
  m_VolumeLOD->AddLOD(m_MedResMapper,m_VolumeProperty,0.0);
  m_VolumeLOD->AddLOD(m_HiResMapper,m_VolumeProperty,0.0);
  
  m_Resampler = vtkImageResample::New();
  m_Resampler->SetAxisMagnificationFactor(0,0.5);
  m_Resampler->SetAxisMagnificationFactor(1,0.5);
  m_Resampler->SetAxisMagnificationFactor(2,0.5);
  
  //m_Volume = vtkVolume::New();
  // m_Volume->SetMapper( m_VtkVolumeMapper );
  // m_Volume->SetProperty(m_VolumeProperty); 
  // m_Prop3D = m_Volume;
  m_Prop3D = m_VolumeLOD;
  m_Prop3D->Register(NULL); 
  
  m_ImageCast = vtkImageShiftScale::New(); 
  m_ImageCast->SetOutputScalarTypeToUnsignedShort();
  m_ImageCast->ClampOverflowOn();

  m_UnitSpacingImageFilter = vtkImageChangeInformation::New();
  m_UnitSpacingImageFilter->SetInput(m_ImageCast->GetOutput());
  m_UnitSpacingImageFilter->SetOutputSpacing(1.0,1.0,1.0);
}


mitk::VolumeDataVtkMapper3D::~VolumeDataVtkMapper3D()
{
  // m_VtkVolumeMapper->Delete();
  //m_Volume->Delete();
  m_UnitSpacingImageFilter->Delete();
  m_ImageCast->Delete();
  m_LowResMapper->Delete();
  m_MedResMapper->Delete();
  m_HiResMapper->Delete();
  m_Resampler->Delete();
  m_VolumeProperty->Delete();
  m_VolumeLOD->Delete();
}
void mitk::VolumeDataVtkMapper3D::AbortCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata) {
  // std::cout << "abort test called" << std::endl;
  vtkRenderWindow* renWin = dynamic_cast<vtkRenderWindow*>(caller);
  assert(renWin);
  // FIXME: qApp->hasPendingEvents is always true, renWin->GetEventPending is
  // always false. So aborting the render doesn't work this way.

  // if (
  //   qApp->hasPendingEvents()
  // )
  // {
  // std::cout << "setting abort render" << std::endl;
  //   renWin->SetAbortRender(1);    
  // }
}

void mitk::VolumeDataVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{

  if(IsVisible(renderer)==false ||
      GetDataTreeNode() == NULL || 
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer).GetPointer())==NULL ||  
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer).GetPointer())->GetValue() == false 
    )
  {
    //  FIXME: don't understand this 
    if (m_Prop3D) {
      //      std::cout << "visibility off" <<std::endl;

      m_Prop3D->VisibilityOff();
    }
    return;
  }
  if (m_Prop3D) {
    m_Prop3D->VisibilityOn();
  }

  mitk::Image* input  = const_cast<mitk::Image *>(this->GetInput());

  // FIXME: const_cast; maybe GetVtkImageData can be made const by using volatile    
  const TimeSlicedGeometry* inputtimegeometry = dynamic_cast<const TimeSlicedGeometry*>(input->GetUpdatedGeometry());
  assert(inputtimegeometry!=NULL);

  const Geometry3D* worldgeometry = renderer->GetCurrentWorldGeometry();

  assert(worldgeometry!=NULL);

  int timestep=0;
  ScalarType time = worldgeometry->GetTimeBoundsInMS()[0];
  if(time>-ScalarTypeNumericTraits::max())
    timestep = inputtimegeometry->MSToTimeStep(time);

  if(inputtimegeometry->IsValidTime(timestep)==false)
    return;

  vtkImageData* inputData = input->GetVtkImageData(timestep);
  if(inputData==NULL)
    return;
  m_ImageCast->SetInput( inputData );

//  m_ImageCast->Update();    
  //m_VtkVolumeMapper->SetInput( m_UnitSpacingImageFilter->GetOutput() );
  //m_Volume->SetMapper( m_VtkVolumeMapper );
  m_Resampler->SetInput(m_UnitSpacingImageFilter->GetOutput());  
    
  m_LowResMapper->SetInput(m_Resampler->GetOutput());
  m_MedResMapper->SetInput(m_UnitSpacingImageFilter->GetOutput());
  m_HiResMapper->SetInput(m_UnitSpacingImageFilter->GetOutput());
  
  vtkPiecewiseFunction *opacityTransferFunction;
  vtkColorTransferFunction* colorTransferFunction;

  opacityTransferFunction  = vtkPiecewiseFunction::New();
  colorTransferFunction    = vtkColorTransferFunction::New();

  m_ImageCast->SetShift(0);

  
mitk::TransferFunctionProperty::Pointer tranferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction").GetPointer());
  if (tranferFunctionProp.IsNull()) {
   // create one
    mitk::TransferFunction::Pointer newTF = mitk::TransferFunction::New();
    newTF->InitializeByMitkImage(input);
//    newTF->SetMin(-10);
//    newTF->SetMax(+310);
    this->GetDataTreeNode()->SetProperty("TransferFunction", new TransferFunctionProperty(newTF.GetPointer()));
  } 
    
  mitk::LookupTableProperty::Pointer lookupTableProp;
  lookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataTreeNode()->GetProperty("LookupTable").GetPointer());
 mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction").GetPointer());
  if (transferFunctionProp.IsNotNull() && transferFunctionProp->GetValue()->GetValid())   {

    opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
    colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction(); 
    std::cout << "Updated transfer functions in mapper" << std::endl;
  } else if (lookupTableProp.IsNotNull() )
{
    lookupTableProp->GetLookupTable().CreateColorTransferFunction(colorTransferFunction);
    colorTransferFunction->ClampingOn();
    lookupTableProp->GetLookupTable().CreateOpacityTransferFunction(opacityTransferFunction);
    opacityTransferFunction->ClampingOn();
  }
  else
  {
    mitk::LevelWindow levelWindow;
    int lw_min,lw_max;
    if (GetLevelWindow(levelWindow,renderer) || GetLevelWindow(levelWindow,renderer,"levelWindow"))
    {
      lw_min = (int)levelWindow.GetMin();
      lw_max = (int)levelWindow.GetMax();
      //      std::cout << "levwin:" << levelWindow << std::endl;
      if(lw_min<0)
      {
        m_ImageCast->SetShift(-lw_min);
        lw_max+=-lw_min;
        lw_min=0;
      }
    } 
    else 
    {
      lw_min = 0;
      lw_max = 255;
    }

    opacityTransferFunction->AddPoint( lw_min, 0.0 );
    opacityTransferFunction->AddPoint( lw_max, 0.8 );
    opacityTransferFunction->ClampingOn();

    //colorTransferFunction->AddRGBPoint( lw_min, 0.0, 0.0, 1.0 );
    //colorTransferFunction->AddRGBPoint( (lw_min+lw_max)/2, 1.0, 0.0, 0.0 );
    //colorTransferFunction->AddRGBPoint( lw_max, 0.0, 1.0, 0.0 );

    float rgb[3]={1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    if(GetColor(rgb, renderer))
    {
      colorTransferFunction->AddRGBPoint( lw_min, 0.0, 0.0, 0.0 );
      colorTransferFunction->AddRGBPoint( (lw_min+lw_max)/2, rgb[0], rgb[1], rgb[2] );
      colorTransferFunction->AddRGBPoint( lw_max, rgb[0], rgb[1], rgb[2] );
    }
    else
    {
      colorTransferFunction->AddRGBPoint( lw_min, 0.0, 0.0, 0.0 );
      colorTransferFunction->AddRGBPoint( (lw_min+lw_max)/2, 1, 1, 0.0 );
      colorTransferFunction->AddRGBPoint( lw_max, 0.8, 0.2, 0 );
    }

    colorTransferFunction->ClampingOn();
  }
  
  m_VolumeProperty->SetColor( colorTransferFunction );
  m_VolumeProperty->SetScalarOpacity( opacityTransferFunction ); 
  //m_VolumeProperty->ShadeOn();

  m_VolumeProperty->SetDiffuse(0.99);
  m_VolumeProperty->SetAmbient(0.99);
  mitk::OpenGLRenderer* openGlRenderer = dynamic_cast<mitk::OpenGLRenderer*>(renderer);
  assert(openGlRenderer);
  vtkRenderWindow* vtkRendWin = dynamic_cast<vtkRenderWindow*>(openGlRenderer->GetVtkRenderWindow());
  if (vtkRendWin) {
    vtkRendWin->SetDesiredUpdateRate(20.0);
  } else {
    std::cout << "no vtk renderwindow" << std::endl;
  }
  //m_Volume->Update();
  // m_VolumeLOD->Update();
  // m_Prop3D = m_Volume;

  // now add an AbortCheckEvent callback for cancelling the rendering 
/*
  mitk::OpenGLRenderer* openGlRenderer = dynamic_cast<mitk::OpenGLRenderer*>(renderer);
  assert(openGlRenderer);
  vtkCallbackCommand* cbc = vtkCallbackCommand::New(); 
  cbc->SetCallback(mitk::VolumeDataVtkMapper3D::AbortCallback); 
  vtkRenderWindow* vtkRendWin;
  vtkRendWin = (vtkRenderWindow*)openGlRenderer->GetVtkRenderWindow();
  vtkRendWin->AddObserver(vtkCommand::AbortCheckEvent,cbc); 
*/
//  colorTransferFunction->Delete();
//  opacityTransferFunction->Delete();
}

void mitk::VolumeDataVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{

}
