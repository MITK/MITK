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

#include "Step6.h"

#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

#include "mitkProperties.h"
#include "mitkRenderingManager.h"

#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"

#include "mitkImageAccessByItk.h"

#include "mitkRenderingManager.h"
#include <mitkIOUtil.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

//##Documentation
//## @brief Start region-grower at interactively added points
Step6::Step6(int argc, char* argv[], QWidget *parent)
: QWidget(parent)
{
  // load data as in the previous steps; a reference to the first loaded
  // image is kept in the member m_FirstImage and used as input for the
  // region growing
  Load(argc, argv);
}

void Step6::Initialize()
{
  // setup the widgets as in the previous steps, but with an additional
  // QVBox for a button to start the segmentation
  this->SetupWidgets();

  // Create controlsParent widget with horizontal layout
  QWidget *controlsParent = new QWidget(this);
  this->layout()->addWidget(controlsParent);

  QHBoxLayout* hlayout = new QHBoxLayout(controlsParent);
  hlayout->setSpacing(2);

  QLabel *labelThresholdMin = new QLabel("Lower Threshold:", controlsParent);
  hlayout->addWidget(labelThresholdMin);

  m_LineEditThresholdMin = new QLineEdit("-1000", controlsParent);
  hlayout->addWidget(m_LineEditThresholdMin);

  QLabel *labelThresholdMax = new QLabel("Upper Threshold:", controlsParent);
  hlayout->addWidget(labelThresholdMax);

  m_LineEditThresholdMax = new QLineEdit("-400", controlsParent);
  hlayout->addWidget(m_LineEditThresholdMax);

  // create button to start the segmentation and connect its clicked()
  // signal to method StartRegionGrowing
  QPushButton* startButton = new QPushButton("start region growing",
      controlsParent);
  hlayout->addWidget(startButton);

  connect(startButton, SIGNAL(clicked()), this, SLOT(StartRegionGrowing()));
  if (m_FirstImage.IsNull())
    startButton->setEnabled(false);

  // as in Step5, create PointSet (now as a member m_Seeds) and
  // associate a interactor to it

  m_Seeds = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  pointSetNode->SetData(m_Seeds);
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(2));
  m_DataStorage->Add(pointSetNode);

  // Create PointSetDataInteractor
  mitk::PointSetDataInteractor::Pointer interactor = mitk::PointSetDataInteractor::New();
  interactor->LoadStateMachine("PointSet.xml");
  interactor->SetEventConfig("PointSetConfig.xml");
  interactor->SetDataNode(pointSetNode);
}

int Step6::GetThresholdMin()
{
  return m_LineEditThresholdMin->text().toInt();
}

int Step6::GetThresholdMax()
{
  return m_LineEditThresholdMax->text().toInt();
}

void Step6::StartRegionGrowing()
{
  AccessByItk_1(m_FirstImage, RegionGrowing, this);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void Step6::Load(int argc, char* argv[])
{
  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  m_DataStorage = mitk::StandaloneDataStorage::New();

  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for (i = 1; i < argc; ++i)
  {
    // For testing
    if (strcmp(argv[i], "-testing") == 0)
      continue;

    // Load datanode (eg. many image formats, surface formats, etc.)
    mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(argv[i],*m_DataStorage);

    if(dataNodes->empty())
    {
      fprintf( stderr, "Could not open file %s \n\n", argv[i] );
      exit(2);
    }

    mitk::Image::Pointer image =  dynamic_cast<mitk::Image*> (dataNodes->at(0)->GetData());
    if ((m_FirstImage.IsNull()) && (image.IsNotNull()))
      m_FirstImage = image;
  }
}

void Step6::SetupWidgets()
{
  //*************************************************************************
  // Part I: Create windows and pass the datastorage to it
  //*************************************************************************

  // Create toplevel widget with vertical layout
  QVBoxLayout* vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0);
  vlayout->setSpacing(2);

  // Create viewParent widget with horizontal layout
  QWidget* viewParent = new QWidget(this);
  vlayout->addWidget(viewParent);

  QHBoxLayout* hlayout = new QHBoxLayout(viewParent);
  hlayout->setMargin(0);
  hlayout->setSpacing(2);

  //*************************************************************************
  // Part Ia: 3D view
  //*************************************************************************

  // Create a renderwindow
  QmitkRenderWindow* renderWindow = new QmitkRenderWindow(viewParent);
  hlayout->addWidget(renderWindow);

  // Tell the renderwindow which (part of) the tree to render
  renderWindow->GetRenderer()->SetDataStorage(m_DataStorage);

  // Use it as a 3D view
  renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  //*************************************************************************
  // Part Ib: 2D view for slicing axially
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget *view2 = new QmitkSliceWidget(viewParent);
  hlayout->addWidget(view2);

  // Tell the QmitkSliceWidget which (part of) the tree to render.
  // By default, it slices the data axially
  view2->SetDataStorage(m_DataStorage);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = m_DataStorage->GetAll();
  view2->SetData(rs->Begin(), mitk::SliceNavigationController::Axial);

  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  m_DataStorage->Add(view2->GetRenderer()->GetCurrentWorldGeometry2DNode());

  //*************************************************************************
  // Part Ic: 2D view for slicing sagitally
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget *view3 = new QmitkSliceWidget(viewParent);
  hlayout->addWidget(view3);

  // Tell the QmitkSliceWidget which (part of) the tree to render
  // and to slice sagitally
  view3->SetDataStorage(m_DataStorage);
  view3->SetData(rs->Begin(), mitk::SliceNavigationController::Sagittal);

  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  m_DataStorage->Add(view3->GetRenderer()->GetCurrentWorldGeometry2DNode());

  //*************************************************************************
  // Part II: handle updates: To avoid unnecessary updates, we have to
  //*************************************************************************
  // define when to update. The RenderingManager serves this purpose, and
  // each RenderWindow has to be registered to it.
  /*mitk::RenderingManager *renderingManager =
   mitk::RenderingManager::GetInstance();
   renderingManager->AddRenderWindow( renderWindow );
   renderingManager->AddRenderWindow( view2->GetRenderWindow() );
   renderingManager->AddRenderWindow( view3->GetRenderWindow() );*/
}

/**
 \example Step6.cpp
 */
