#include "mitkLevelWindow.h"
#include "mitkVolumeDataVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"
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
#include <vtkImageWriter.h>
#include <vtkImageData.h>
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
  m_VtkVolumeMapper = vtkVolumeRayCastMapper::New();
  vtkVolumeRayCastCompositeFunction* compositeFunction = vtkVolumeRayCastCompositeFunction::New();
  m_VtkVolumeMapper->SetVolumeRayCastFunction(compositeFunction);
  compositeFunction->Delete();
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
}


mitk::VolumeDataVtkMapper3D::~VolumeDataVtkMapper3D()
{

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

void mitk::VolumeDataVtkMapper3D::SetInput(const mitk::DataTreeNode * data) {
   Superclass::SetInput(data);
 
  const mitk::Image* img = (GetInput());
  if (img == NULL) {
    itkWarningMacro("Image == NULL")
  } else {   
   // FIXME: const_cast; maybe GetVtkImageData can be made const by using volatile
    vtkImageCast* imageCast = vtkImageCast::New(); 
    imageCast->SetOutputScalarTypeToUnsignedShort();
    imageCast->SetInput(const_cast<mitk::Image*>(img)->GetVtkImageData()); 
    imageCast->ClampOverflowOn();
    imageCast->Update();
    vtkImageData* castResult = vtkImageData::New();
    castResult->DeepCopy(imageCast->GetOutput());
    
    m_VtkVolumeMapper->SetInput(castResult);
    m_Volume->SetMapper( m_VtkVolumeMapper );
  }
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
  vtkPiecewiseFunction *opacityTransferFunction;
  vtkColorTransferFunction* colorTransferFunction;

  opacityTransferFunction  = vtkPiecewiseFunction::New();
  colorTransferFunction    = vtkColorTransferFunction::New();
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
  opacityTransferFunction->AddPoint( lw_min, 0.0 );
  opacityTransferFunction->AddPoint( lw_max, 0.8 );
  opacityTransferFunction->ClampingOn();

  colorTransferFunction->AddRGBPoint( lw_min, 0.0, 0.0, 1.0 );
  colorTransferFunction->AddRGBPoint( (lw_min+lw_max)/2, 1.0, 0.0, 0.0 );
  colorTransferFunction->AddRGBPoint( lw_max, 0.0, 1.0, 0.0 );
  colorTransferFunction->ClampingOn();
  
  m_VolumeProperty->SetColor( colorTransferFunction );
  m_VolumeProperty->SetScalarOpacity( opacityTransferFunction ); 
  m_VolumeProperty->ShadeOn();
  m_VolumeProperty->SetDiffuse(0.7);
  m_VolumeProperty->SetAmbient(0.31);
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
