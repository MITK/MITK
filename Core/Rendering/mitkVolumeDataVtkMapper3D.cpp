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
//  m_Prop3D
  m_Volume = vtkVolume::New();
//  m_VtkVolumeMapper->SetInput(img->GetVtkImageData()); 
  m_Volume->SetMapper( m_VtkVolumeMapper );
 //  m_Volume->SetProperty(volumeProperty); 
 m_Prop3D = m_Volume;
  m_Prop3D->Register(NULL); 

}


mitk::VolumeDataVtkMapper3D::~VolumeDataVtkMapper3D()
{

}
void mitk::VolumeDataVtkMapper3D::abortTest(void* test) {
  vtkRenderWindow* renWin = reinterpret_cast<vtkRenderWindow*>(test);
  assert(renWin);
  if (renWin->GetEventPending()) {
    renWin->SetAbortRender(1);    
  }
}

void mitk::VolumeDataVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{
 if(IsVisible(renderer)==false)
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
  // FIXME
  mitk::Image* img = const_cast<mitk::Image*>(GetInput());
// dynamic_cast<mitk::Image*>(GetData());
  if (img == NULL) {
    std::cout << "Image == NULL" << std::endl;
  } else {   
  vtkVolumeProperty* volumeProperty = vtkVolumeProperty::New();
  vtkPiecewiseFunction*	opacityTransferFunction, *gradientTransferFunction;
  vtkColorTransferFunction* colorTransferFunction;

  opacityTransferFunction  = vtkPiecewiseFunction::New();
  colorTransferFunction    = vtkColorTransferFunction::New();
//  gradientTransferFunction = vtkPiecewiseFunction::New();
  mitk::LevelWindow levelWindow;
  int lw_min,lw_max;
  if (GetLevelWindow(levelWindow,renderer)) {
    std::cout << "level window found" << std::endl;
    lw_min = (int)levelWindow.GetMin();
    lw_max = (int)levelWindow.GetMax();
  } else {
    lw_min = 0;
    lw_max = 255;
  }

  opacityTransferFunction->AddPoint( lw_min, 0.0 );
  opacityTransferFunction->AddPoint( lw_max, 0.9 );
//  opacityTransferFunction->AddPoint( max+offset, 0.9 );
  opacityTransferFunction->ClampingOff();
  
  colorTransferFunction->AddRGBPoint( lw_min, 1.0, 1.0, 1.0 );
  colorTransferFunction->AddRGBPoint( lw_max, 1.0, 1.0, 1.0 );
//  colorTransferFunction->AddRGBPoint( max+offset, 1.0, 1.0, 1.0 );
  colorTransferFunction->ClampingOff();
 
  volumeProperty->SetColor( colorTransferFunction );
  volumeProperty->SetScalarOpacity( opacityTransferFunction ); 
  volumeProperty->ShadeOn();
  volumeProperty->SetDiffuse(0.7);
  volumeProperty->SetAmbient(0.01);

  m_Volume = vtkVolume::New();
  m_VtkVolumeMapper->SetInput(img->GetVtkImageData()); 
  m_Volume->SetMapper( m_VtkVolumeMapper );
  m_Volume->SetProperty(volumeProperty); 
  
  m_Prop3D = m_Volume;
  mitk::OpenGLRenderer* openGlRenderer = dynamic_cast<mitk::OpenGLRenderer*>(renderer);
  assert(openGlRenderer);
  
  vtkRenderWindow* vtkRendWin;
  vtkRendWin = (vtkRenderWindow*)openGlRenderer->GetVtkRenderWindow();
  vtkRendWin->SetAbortCheckMethod(mitk::VolumeDataVtkMapper3D::abortTest,vtkRendWin); 
  // mitk::VolumeData::Pointer input  = const_cast<mitk::VolumeData*>(this->GetInput());

  // m_VtkPolyDataMapper->SetInput(input->GetVtkPolyData());

  //apply properties read from the PropertyList
  // ApplyProperties(m_Actor, renderer);

  StandardUpdate();
  }
}


void mitk::VolumeDataVtkMapper3D::Update()
{
}

void mitk::VolumeDataVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{

}


