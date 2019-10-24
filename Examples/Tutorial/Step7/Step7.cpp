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

#include "Step7.h"

#include <QmitkRenderWindow.h>
#include <QmitkSliceWidget.h>

#include <mitkProperties.h>

#include <mitkSurface.h>
#include <mitkRenderingManager.h>
#include <mitkImageVtkAccessor.h>

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

  std::cout << "7";
  if(m_ResultImage.IsNotNull())
  {
    m_ResultNode->SetProperty("volumerendering", mitk::BoolProperty::New(false));

    vtkMarchingCubes* surfaceCreator = vtkMarchingCubes::New();
    mitk::ImageVtkAccessor accessor(m_ResultImage);
    surfaceCreator->SetInputData(accessor.getVtkImageData());
    surfaceCreator->SetValue(0, 1);
    surfaceCreator->Update();

    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(surfaceCreator->GetOutput()); //VTK6_TODO

    mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
    surfaceNode->SetData(surface);

    m_DataStorage->Add(surfaceNode);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    std::cout << "8";
    surfaceCreator->Delete();
  }

  std::cout << "9";

}

/**
\example Step7.cpp
*/
