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
#include <vtkImageCast.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageWriter.h>
#include <vtkVolumeTextureMapper2D.h> 


const mitk::Image* mitk::VolumeDataVtkMapper3D::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Image*> ( GetData() );
}


void mitk::VolumeDataVtkMapper3D::GenerateData()
{
  
 
}


void mitk::VolumeDataVtkMapper3D::GenerateOutputInformation()
{
}


mitk::VolumeDataVtkMapper3D::VolumeDataVtkMapper3D()
{
  //m_VtkVolumeMapper = vtkVolumeRayCastMapper::New();
	m_VtkVolumeMapper = vtkVolumeTextureMapper2D::New();

  //vtkVolumeRayCastCompositeFunction* compositeFunction = vtkVolumeRayCastCompositeFunction::New();
  //m_VtkVolumeMapper->SetVolumeRayCastFunction(compositeFunction);
  //compositeFunction->Delete();
  vtkFiniteDifferenceGradientEstimator* gradientEstimator = 
  vtkFiniteDifferenceGradientEstimator::New();
  m_VtkVolumeMapper->SetGradientEstimator(gradientEstimator);
  gradientEstimator->Delete();

  m_Volume = vtkVolume::New();
  m_Volume->SetMapper( m_VtkVolumeMapper );
  m_Prop3D = m_Volume;
  m_Prop3D->Register(NULL); 
 
  m_VolumeProperty = vtkVolumeProperty::New();
  m_Volume->SetProperty(m_VolumeProperty); 

  m_ImageCast = vtkImageCast::New(); 
  m_ImageCast->SetOutputScalarTypeToUnsignedShort();
  m_ImageCast->ClampOverflowOn();

  m_UnitSpacingImageFilter = vtkImageChangeInformation::New();
  m_UnitSpacingImageFilter->SetInput(m_ImageCast->GetOutput());
  m_UnitSpacingImageFilter->SetOutputSpacing(1.0,1.0,1.0);
}


mitk::VolumeDataVtkMapper3D::~VolumeDataVtkMapper3D()
{
  m_VtkVolumeMapper->Delete();
  m_Volume->Delete();
  m_UnitSpacingImageFilter->Delete();
  m_ImageCast->Delete();
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

void mitk::VolumeDataVtkMapper3D::Update(mitk::BaseRenderer* renderer)
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
  m_VtkVolumeMapper->SetInput( m_UnitSpacingImageFilter->GetOutput() );
  m_Volume->SetMapper( m_VtkVolumeMapper );

  vtkPiecewiseFunction *opacityTransferFunction;
  vtkColorTransferFunction* colorTransferFunction;

  opacityTransferFunction  = vtkPiecewiseFunction::New();
  colorTransferFunction    = vtkColorTransferFunction::New();

  mitk::LookupTableProperty::Pointer LookupTableProp;
  LookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataTreeNode()->GetProperty("LookupTable").GetPointer());
  if (LookupTableProp.IsNotNull() )
  {
    LookupTableProp->GetLookupTable().CreateColorTransferFunction(colorTransferFunction);
    colorTransferFunction->ClampingOn();
    LookupTableProp->GetLookupTable().CreateOpacityTransferFunction(opacityTransferFunction);
    opacityTransferFunction->ClampingOn();

  }
  else
  {
    mitk::LevelWindow levelWindow;
    int lw_min,lw_max;
    if (GetLevelWindow(levelWindow,renderer)) {
      lw_min = (int)levelWindow.GetMin();
      lw_max = (int)levelWindow.GetMax();
      //      std::cout << "levwin:" << levelWindow << std::endl;
    } else {
      lw_min = 0;
      lw_max = 255;
    }
    //opacityTransferFunction->AddPoint( lw_min, 0.0 );
    //opacityTransferFunction->AddPoint( lw_max, 0.8 );
    opacityTransferFunction->AddPoint( (lw_min+lw_max)/2, 0.0 );
    opacityTransferFunction->AddPoint( lw_max, 0.5 );
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

  m_Volume->Update();
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
  StandardUpdate();

}


void mitk::VolumeDataVtkMapper3D::Update()
{
}

void mitk::VolumeDataVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{

}
