#include "Step7.h"

#include <QmitkRenderWindow.h>
#include <QmitkSliceWidget.h>

#include <mitkProperties.h>

#include <mitkSurface.h>
#include <mitkRenderingManager.h>

#include <vtkMarchingCubes.h>
#include <vtkSTLWriter.h>
#include <vtkImageData.h>



//##Documentation
//## @brief Convert result of region-grower into a surface
//## by means of a VTK filter
Step7::Step7( int argc, char* argv[], QWidget *parent )
  : Step6( argc, argv, parent )
{
}
void Step7::StartRegionGrowing()
{
  Step6::StartRegionGrowing();

  if(m_ResultImage.IsNotNull())
  {
    m_ResultNode->SetProperty("volumerendering", mitk::BoolProperty::New(false));

    vtkMarchingCubes* surfaceCreator = vtkMarchingCubes::New();
    surfaceCreator->SetInput(m_ResultImage->GetVtkImageData());
    surfaceCreator->SetValue(0, 1);

    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(surfaceCreator->GetOutput());

    mitk::DataTreeNode::Pointer surfaceNode = mitk::DataTreeNode::New();
    surfaceNode->SetData(surface);

    m_DataStorage->Add(surfaceNode);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

/**
\example Step7.cpp
*/
