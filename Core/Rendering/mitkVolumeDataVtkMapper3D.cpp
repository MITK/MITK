#include <qapplication.h>
#include "mitkVolumeDataVtkMapper3D.h"
#include "DataTreeNode.h"
#include "mitkFloatProperty.h"
#include "mitkBoolProperty.h"
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
// extern QApplication* qApp;

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
  vtkFiniteDifferenceGradientEstimator* gradientEstimator = 
	vtkFiniteDifferenceGradientEstimator::New();
  m_VtkVolumeMapper->SetGradientEstimator(gradientEstimator);
  gradientEstimator->Delete();
  m_Volume = vtkVolume::New();
  m_Volume->SetMapper( m_VtkVolumeMapper );
  m_Prop3D = m_Volume;
  m_Prop3D->Register(NULL); 

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

void mitk::VolumeDataVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{

 if(IsVisible(renderer)==false ||
    GetDataTreeNode() == NULL || 
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer).GetPointer())==NULL ||  
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer).GetPointer())->GetBool() == false 
  )
  {
//  FIXME: don't understand this 
    if (m_Prop3D) {
      std::cout << "visibility off" <<std::endl;

      m_Prop3D->VisibilityOff();
    }
    return;
  }
  if (m_Prop3D) {
    m_Prop3D->VisibilityOn();
  }
  const mitk::Image* img = (GetInput());
  if (img == NULL) {
    itkWarningMacro("Image == NULL")
  } else {   
    vtkVolumeProperty* volumeProperty = vtkVolumeProperty::New();
    vtkPiecewiseFunction *opacityTransferFunction, *gradientTransferFunction;
    vtkColorTransferFunction* colorTransferFunction;

    opacityTransferFunction  = vtkPiecewiseFunction::New();
    colorTransferFunction    = vtkColorTransferFunction::New();
    mitk::LevelWindow levelWindow;
    int lw_min,lw_max;
    if (GetLevelWindow(levelWindow,renderer)) {
      lw_min = (int)levelWindow.GetMin();
      lw_max = (int)levelWindow.GetMax();
    } else {
      lw_min = 0;
      lw_max = 255;
    }

    opacityTransferFunction->AddPoint( lw_min, 0.0 );
    opacityTransferFunction->AddPoint( lw_max, 0.9 );
    opacityTransferFunction->ClampingOff();
    
    colorTransferFunction->AddRGBPoint( lw_min, 1.0, 1.0, 1.0 );
    colorTransferFunction->AddRGBPoint( lw_max, 1.0, 1.0, 1.0 );
    colorTransferFunction->ClampingOff();
   
    volumeProperty->SetColor( colorTransferFunction );
    volumeProperty->SetScalarOpacity( opacityTransferFunction ); 
    volumeProperty->ShadeOn();
    volumeProperty->SetDiffuse(0.7);
    volumeProperty->SetAmbient(0.01);

    m_Volume = vtkVolume::New();
    // FIXME: const_cast; maybe GetVtkImageData can be made const by using volatile
    m_VtkVolumeMapper->SetInput(const_cast<mitk::Image*>(img)->GetVtkImageData()); 
    m_Volume->SetMapper( m_VtkVolumeMapper );
    m_Volume->SetProperty(volumeProperty); 
    
    m_Prop3D = m_Volume;

    // now add an AbortCheckEvent callback for cancelling the rendering 
    mitk::OpenGLRenderer* openGlRenderer = dynamic_cast<mitk::OpenGLRenderer*>(renderer);
    assert(openGlRenderer);
    vtkCallbackCommand* cbc = vtkCallbackCommand::New(); 
    cbc->SetCallback(mitk::VolumeDataVtkMapper3D::AbortCallback); 
    vtkRenderWindow* vtkRendWin;
    vtkRendWin = (vtkRenderWindow*)openGlRenderer->GetVtkRenderWindow();
    vtkRendWin->AddObserver(vtkCommand::AbortCheckEvent,cbc); 

    StandardUpdate();
  }
}


void mitk::VolumeDataVtkMapper3D::Update()
{
}

void mitk::VolumeDataVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{

}
