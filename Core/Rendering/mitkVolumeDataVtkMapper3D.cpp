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
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>
#include <vtkImageShiftScale.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageWriter.h>
#include <vtkVolumeTextureMapper2D.h> 
#include <vtkImageData.h>
#include <vtkLODProp3D.h>
#include <vtkImageResample.h>
#include <mitkVtkRenderWindow.h>

#include <itkMultiThreader.h>

const mitk::Image* mitk::VolumeDataVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Image*> ( GetData() );
}

mitk::VolumeDataVtkMapper3D::VolumeDataVtkMapper3D()
{
  //  m_VtkVolumeMapper = vtkVolumeRayCastMapper::New();
	// m_VtkVolumeMapper = vtkVolumeTextureMapper2D::New();

  m_LowResMapper =  vtkVolumeTextureMapper2D::New();
  m_MedResMapper =  vtkVolumeRayCastMapper::New();
  m_HiResMapper = vtkVolumeRayCastMapper::New();


  //m_MedResMapper->AutoAdjustSampleDistancesOff();
  //m_MedResMapper->SetImageSampleDistance(2.5);

  //m_HiResMapper->AutoAdjustSampleDistancesOff();
  //m_HiResMapper->SetImageSampleDistance(0.5);
  m_HiResMapper->SetMaximumImageSampleDistance(10.0);
  m_HiResMapper->SetMinimumImageSampleDistance(1.0);
  m_HiResMapper->IntermixIntersectingGeometryOn();
  m_HiResMapper->SetNumberOfThreads( itk::MultiThreader::GetGlobalDefaultNumberOfThreads() );

  vtkVolumeRayCastCompositeFunction* compositeFunction = vtkVolumeRayCastCompositeFunction::New();
  m_MedResMapper->SetVolumeRayCastFunction(compositeFunction);
  m_HiResMapper->SetVolumeRayCastFunction(compositeFunction);
//  compositeFunction->Delete();
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

  m_VolumeLOD->AddLOD(m_HiResMapper,m_VolumeProperty,0.0);
  m_VolumeLOD->AddLOD(m_MedResMapper,m_VolumeProperty,0.0);
  m_VolumeLOD->AddLOD(m_LowResMapper,m_VolumeProperty,0.0);
  
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
void mitk::VolumeDataVtkMapper3D::AbortCallback(vtkObject *caller, unsigned long , void *, void *) {
  // std::cout << "abort test called" << std::endl;
  vtkRenderWindow* renWin = dynamic_cast<vtkRenderWindow*>(caller);
  assert(renWin);
   int foo=renWin->GetEventPending();
   if(foo!=0) 
   {
     renWin->SetAbortRender(1);
   }
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
  if((input==NULL) || (input->IsInitialized()==false))
    return;

  // FIXME: const_cast; maybe GetVtkImageData can be made const by using volatile    
  const TimeSlicedGeometry* inputtimegeometry = input->GetTimeSlicedGeometry();
  assert(inputtimegeometry!=NULL);

  const Geometry3D* worldgeometry = renderer->GetCurrentWorldGeometry();

  assert(worldgeometry!=NULL);

  int timestep=0;
  ScalarType time = worldgeometry->GetTimeBounds()[0];
  if(time>-ScalarTypeNumericTraits::max())
    timestep = inputtimegeometry->MSToTimeStep(time);

  if(inputtimegeometry->IsValidTime(timestep)==false)
    return;

  vtkImageData* inputData = input->GetVtkImageData(timestep);
  if(inputData==NULL)
    return;

  m_ImageCast->SetInput( inputData );

//  m_ImageCast->Update();    

  //trying to avoid update-problem, when size of input changed. Does not really help much.
  if(m_ImageCast->GetOutput()!=NULL)
  {
    int inputWE[6]; 
    inputData->GetWholeExtent(inputWE);
    int * outputWE=m_UnitSpacingImageFilter->GetOutput()->GetExtent();
    if(m_ImageCast->GetOutput()!=NULL && memcmp(inputWE, outputWE,sizeof(int)*6) != 0)
    {
//      m_ImageCast->GetOutput()->SetUpdateExtentToWholeExtent();
//      m_UnitSpacingImageFilter->GetOutput()->SetUpdateExtentToWholeExtent();
      m_UnitSpacingImageFilter->GetOutput()->SetUpdateExtent(inputWE);
      m_UnitSpacingImageFilter->UpdateWholeExtent();
    }
  }

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

/*  
mitk::TransferFunctionProperty::Pointer tranferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction").GetPointer());
  if (tranferFunctionProp.IsNull()) {
   // create one
    mitk::TransferFunction::Pointer newTF = mitk::TransferFunction::New();
    newTF->InitializeByMitkImage(input);
    this->GetDataTreeNode()->SetProperty("TransferFunction", new TransferFunctionProperty(newTF.GetPointer()));
  } 
  */
    
  mitk::LookupTableProperty::Pointer lookupTableProp;
  lookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataTreeNode()->GetProperty("LookupTable").GetPointer());
  mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction").GetPointer());
  if ( transferFunctionProp.IsNotNull() )   {

    opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
    colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction();
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

    bool binary=false;
    GetDataTreeNode()->GetBoolProperty("binary", binary, renderer);
    if(binary)
    {
      lw_min=0; lw_max=2;
    }
    else
    if(GetLevelWindow(levelWindow,renderer,"levelWindow") || GetLevelWindow(levelWindow,renderer))
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
  m_VolumeProperty->SetDiffuse(0.2);
  m_VolumeProperty->SetAmbient(0.9);
  m_VolumeProperty->ShadeOn();

  mitk::OpenGLRenderer* openGlRenderer = dynamic_cast<mitk::OpenGLRenderer*>(renderer);
  assert(openGlRenderer);
  vtkRenderWindow* vtkRendWin = dynamic_cast<vtkRenderWindow*>(openGlRenderer->GetVtkRenderWindow());
  if (vtkRendWin) {
//    vtkRendWin->SetDesiredUpdateRate(25.0);
    vtkRenderWindowInteractor* interactor = vtkRendWin->GetInteractor();
    interactor->SetDesiredUpdateRate(50000.0);
    interactor->SetStillUpdateRate(0.0001);

    vtkCallbackCommand* cbc = vtkCallbackCommand::New(); 
    cbc->SetCallback(mitk::VolumeDataVtkMapper3D::AbortCallback); 
    vtkRendWin->AddObserver(vtkCommand::AbortCheckEvent,cbc); 

  } else {
    std::cout << "no vtk renderwindow" << std::endl;
  }
  //m_Volume->Update();
  // m_VolumeLOD->Update();
  // m_Prop3D = m_Volume;

  // now add an AbortCheckEvent callback for cancelling the rendering 

//  colorTransferFunction->Delete();
//  opacityTransferFunction->Delete();
}

void mitk::VolumeDataVtkMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* /*renderer*/)
{

}
