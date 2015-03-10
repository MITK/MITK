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

#define SMW_INFO MITK_INFO("widget.stdmulti")

#include "QmitkStdMultiWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <qsplitter.h>
#include <QList>
#include <QMouseEvent>
#include <QTimer>

#include <mitkProperties.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkGlobalInteraction.h>
#include <mitkDisplayInteractor.h>
#include <mitkPointSet.h>
#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>
#include <mitkLine.h>
#include <mitkInteractionConst.h>
#include <mitkDataStorage.h>
#include <mitkOverlayManager.h>
#include <mitkNodePredicateBase.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkStatusBar.h>
#include <mitkImage.h>
#include <mitkVtkLayerController.h>

#include <iomanip>

QmitkStdMultiWidget::QmitkStdMultiWidget(QWidget* parent, Qt::WindowFlags f, mitk::RenderingManager* renderingManager, mitk::BaseRenderer::RenderingMode::Type renderingMode, const QString& name)
: QWidget(parent, f),
mitkWidget1(NULL),
mitkWidget2(NULL),
mitkWidget3(NULL),
mitkWidget4(NULL),
levelWindowWidget(NULL),
QmitkStdMultiWidgetLayout(NULL),
m_Layout(LAYOUT_DEFAULT),
m_PlaneMode(PLANE_MODE_SLICING),
m_RenderingManager(renderingManager),
m_GradientBackgroundFlag(true),
m_TimeNavigationController(NULL),
m_MainSplit(NULL),
m_LayoutSplit(NULL),
m_SubSplit1(NULL),
m_SubSplit2(NULL),
mitkWidget1Container(NULL),
mitkWidget2Container(NULL),
mitkWidget3Container(NULL),
mitkWidget4Container(NULL),
m_PendingCrosshairPositionEvent(false),
m_CrosshairNavigationEnabled(false)
{
  /******************************************************
   * Use the global RenderingManager if none was specified
   * ****************************************************/
  if (m_RenderingManager == NULL)
  {
    m_RenderingManager = mitk::RenderingManager::GetInstance();
  }
  m_TimeNavigationController = m_RenderingManager->GetTimeNavigationController();

  /*******************************/
  //Create Widget manually
  /*******************************/

  //create Layouts
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this );
  QmitkStdMultiWidgetLayout->setContentsMargins(0,0,0,0);

  //Set Layout to widget
  this->setLayout(QmitkStdMultiWidgetLayout);

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( Qt::Vertical, m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //creae Widget Container
  mitkWidget1Container = new QWidget(m_SubSplit1);
  mitkWidget2Container = new QWidget(m_SubSplit1);
  mitkWidget3Container = new QWidget(m_SubSplit2);
  mitkWidget4Container = new QWidget(m_SubSplit2);

  mitkWidget1Container->setContentsMargins(0,0,0,0);
  mitkWidget2Container->setContentsMargins(0,0,0,0);
  mitkWidget3Container->setContentsMargins(0,0,0,0);
  mitkWidget4Container->setContentsMargins(0,0,0,0);

  //create Widget Layout
  QHBoxLayout *mitkWidgetLayout1 = new QHBoxLayout(mitkWidget1Container);
  QHBoxLayout *mitkWidgetLayout2 = new QHBoxLayout(mitkWidget2Container);
  QHBoxLayout *mitkWidgetLayout3 = new QHBoxLayout(mitkWidget3Container);
  QHBoxLayout *mitkWidgetLayout4 = new QHBoxLayout(mitkWidget4Container);

  mitkWidgetLayout1->setMargin(0);
  mitkWidgetLayout2->setMargin(0);
  mitkWidgetLayout3->setMargin(0);
  mitkWidgetLayout4->setMargin(0);

  //set Layout to Widget Container
  mitkWidget1Container->setLayout(mitkWidgetLayout1);
  mitkWidget2Container->setLayout(mitkWidgetLayout2);
  mitkWidget3Container->setLayout(mitkWidgetLayout3);
  mitkWidget4Container->setLayout(mitkWidgetLayout4);

  //set SizePolicy
  mitkWidget1Container->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  mitkWidget2Container->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  mitkWidget3Container->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  mitkWidget4Container->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  //insert Widget Container into the splitters
  m_SubSplit1->addWidget( mitkWidget1Container );
  m_SubSplit1->addWidget( mitkWidget2Container );

  m_SubSplit2->addWidget( mitkWidget3Container );
  m_SubSplit2->addWidget( mitkWidget4Container );

  //  m_RenderingManager->SetGlobalInteraction( mitk::GlobalInteraction::GetInstance() );

  //Create RenderWindows 1
  mitkWidget1 = new QmitkRenderWindow(mitkWidget1Container, name + ".widget1", NULL, m_RenderingManager,renderingMode);
  mitkWidget1->setMaximumSize(2000,2000);
  mitkWidget1->SetLayoutIndex( AXIAL );
  mitkWidgetLayout1->addWidget(mitkWidget1);

  //Create RenderWindows 2
  mitkWidget2 = new QmitkRenderWindow(mitkWidget2Container, name + ".widget2", NULL, m_RenderingManager,renderingMode);
  mitkWidget2->setMaximumSize(2000,2000);
  mitkWidget2->setEnabled( true );
  mitkWidget2->SetLayoutIndex( SAGITTAL );
  mitkWidgetLayout2->addWidget(mitkWidget2);

  //Create RenderWindows 3
  mitkWidget3 = new QmitkRenderWindow(mitkWidget3Container, name + ".widget3", NULL, m_RenderingManager,renderingMode);
  mitkWidget3->setMaximumSize(2000,2000);
  mitkWidget3->SetLayoutIndex( CORONAL );
  mitkWidgetLayout3->addWidget(mitkWidget3);

  //Create RenderWindows 4
  mitkWidget4 = new QmitkRenderWindow(mitkWidget4Container, name + ".widget4", NULL, m_RenderingManager,renderingMode);
  mitkWidget4->setMaximumSize(2000,2000);
  mitkWidget4->SetLayoutIndex( THREE_D );
  mitkWidgetLayout4->addWidget(mitkWidget4);

  //create SignalSlot Connection
  connect( mitkWidget1, SIGNAL( SignalLayoutDesignChanged(int) ), this, SLOT( OnLayoutDesignChanged(int) ) );
  connect( mitkWidget1, SIGNAL( ResetView() ), this, SLOT( ResetCrosshair() ) );
  connect( mitkWidget1, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SLOT( SetWidgetPlaneMode(int) ) );
  connect( this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget1, SLOT(OnWidgetPlaneModeChanged(int)) );

  connect( mitkWidget2, SIGNAL( SignalLayoutDesignChanged(int) ), this, SLOT( OnLayoutDesignChanged(int) ) );
  connect( mitkWidget2, SIGNAL( ResetView() ), this, SLOT( ResetCrosshair() ) );
  connect( mitkWidget2, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SLOT( SetWidgetPlaneMode(int) ) );
  connect( this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget2, SLOT(OnWidgetPlaneModeChanged(int)) );

  connect( mitkWidget3, SIGNAL( SignalLayoutDesignChanged(int) ), this, SLOT( OnLayoutDesignChanged(int) ) );
  connect( mitkWidget3, SIGNAL( ResetView() ), this, SLOT( ResetCrosshair() ) );
  connect( mitkWidget3, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SLOT( SetWidgetPlaneMode(int) ) );
  connect( this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget3, SLOT(OnWidgetPlaneModeChanged(int)) );

  connect( mitkWidget4, SIGNAL( SignalLayoutDesignChanged(int) ), this, SLOT( OnLayoutDesignChanged(int) ) );
  connect( mitkWidget4, SIGNAL( ResetView() ), this, SLOT( ResetCrosshair() ) );
  connect( mitkWidget4, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SLOT( SetWidgetPlaneMode(int) ) );
  connect( this, SIGNAL(WidgetNotifyNewCrossHairMode(int)), mitkWidget4, SLOT(OnWidgetPlaneModeChanged(int)) );

  //Create Level Window Widget
  levelWindowWidget = new QmitkLevelWindowWidget( m_MainSplit ); //this
  levelWindowWidget->setObjectName(QString::fromUtf8("levelWindowWidget"));
  QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(levelWindowWidget->sizePolicy().hasHeightForWidth());
  levelWindowWidget->setSizePolicy(sizePolicy);
  levelWindowWidget->setMaximumSize(QSize(50, 2000));

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //resize Image.
  this->resize( QSize(364, 477).expandedTo(minimumSizeHint()) );

  //Initialize the widgets.
  this->InitializeWidget();

  //Activate Widget Menu
  this->ActivateMenuWidget( true );
}

void QmitkStdMultiWidget::InitializeWidget()
{
  m_PositionTracker = NULL;

  //Make all black and overwrite renderwindow 4
  this->FillGradientBackgroundWithBlack();
  //This is #191919 in hex
  float tmp1[3] = { 0.098f, 0.098f, 0.098f};
  //This is #7F7F7F in hex
  float tmp2[3] = { 0.498f, 0.498f, 0.498f};
  m_GradientBackgroundColors[3] = std::make_pair(mitk::Color(tmp1), mitk::Color(tmp2));

  //Yellow is default color for widget4
  m_DecorationColorWidget4[0] = 1.0f;
  m_DecorationColorWidget4[1] = 1.0f;
  m_DecorationColorWidget4[2] = 0.0f;

  // transfer colors in WorldGeometry-Nodes of the associated Renderer
  mitk::IntProperty::Pointer  layer;
  // of widget 1
  m_PlaneNode1 = mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode1->SetColor(GetDecorationColor(0));
  layer = mitk::IntProperty::New(1000);
  m_PlaneNode1->SetProperty("layer",layer);

  // ... of widget 2
  m_PlaneNode2 = mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode2->SetColor(GetDecorationColor(1));
  layer = mitk::IntProperty::New(1000);
  m_PlaneNode2->SetProperty("layer",layer);

  // ... of widget 3
  m_PlaneNode3 = mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetColor(GetDecorationColor(2));
  layer = mitk::IntProperty::New(1000);
  m_PlaneNode3->SetProperty("layer",layer);

  //The parent node
  m_ParentNodeForGeometryPlanes = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetCurrentWorldPlaneGeometryNode();
  layer = mitk::IntProperty::New(1000);
  m_ParentNodeForGeometryPlanes->SetProperty("layer",layer);

  mitk::OverlayManager::Pointer OverlayManager = mitk::OverlayManager::New();
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->SetOverlayManager(OverlayManager);
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->SetOverlayManager(OverlayManager);
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->SetOverlayManager(OverlayManager);
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->SetOverlayManager(OverlayManager);

  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->SetMapperID(mitk::BaseRenderer::Standard3D);
  // Set plane mode (slicing/rotation behavior) to slicing (default)
  m_PlaneMode = PLANE_MODE_SLICING;

  // Set default view directions for SNCs
  mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Axial );
  mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Sagittal );
  mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Frontal );
  mitkWidget4->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Original );

  SetCornerAnnotation("Axial", GetDecorationColor(0), 0);
  SetCornerAnnotation("Sagittal", GetDecorationColor(1), 1);
  SetCornerAnnotation("Coronal", GetDecorationColor(2), 2);
  SetCornerAnnotation("3D", GetDecorationColor(3), 3);

  // create a slice rotator
  m_SlicesRotator = mitk::SlicesRotator::New("slices-rotator");
  m_SlicesRotator->AddSliceController(
    mitkWidget1->GetSliceNavigationController() );
  m_SlicesRotator->AddSliceController(
    mitkWidget2->GetSliceNavigationController() );
  m_SlicesRotator->AddSliceController(
    mitkWidget3->GetSliceNavigationController() );

  // create a slice swiveller (using the same state-machine as SlicesRotator)
  m_SlicesSwiveller = mitk::SlicesSwiveller::New("slices-rotator");
  m_SlicesSwiveller->AddSliceController(
    mitkWidget1->GetSliceNavigationController() );
  m_SlicesSwiveller->AddSliceController(
    mitkWidget2->GetSliceNavigationController() );
  m_SlicesSwiveller->AddSliceController(
    mitkWidget3->GetSliceNavigationController() );

  //connect to the "time navigation controller": send time via sliceNavigationControllers
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget1->GetSliceNavigationController() , false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget2->GetSliceNavigationController() , false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget3->GetSliceNavigationController() , false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget4->GetSliceNavigationController() , false);
  mitkWidget1->GetSliceNavigationController()
    ->ConnectGeometrySendEvent(mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));

  //reverse connection between sliceNavigationControllers and m_TimeNavigationController
  mitkWidget1->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  mitkWidget2->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  mitkWidget3->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  mitkWidget4->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController, false);

  m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();

  m_LastLeftClickPositionSupplier =
    mitk::CoordinateSupplier::New("navigation", NULL);
  mitk::GlobalInteraction::GetInstance()->AddListener(
    m_LastLeftClickPositionSupplier
    );

  // setup the department logo rendering
  m_LogoRendering = mitk::LogoOverlay::New();
  mitk::BaseRenderer::Pointer renderer4 = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow());
  m_LogoRendering->SetOpacity(0.5);
  mitk::Point2D offset;
  offset.Fill(0.03);
  m_LogoRendering->SetOffsetVector(offset);
  m_LogoRendering->SetRelativeSize(0.15);
  m_LogoRendering->SetCornerPosition(1);
  renderer4->GetOverlayManager()->AddOverlay(m_LogoRendering.GetPointer(),renderer4);

  // setup gradient background and renderwindow rectangle frame
  for(unsigned int i = 0; i < 4; ++i)
  {
    m_GradientBackground[i] = mitk::GradientBackground::New();
    m_GradientBackground[i]->SetRenderWindow(GetRenderWindow(i)->GetVtkRenderWindow());
    m_GradientBackground[i]->Enable();
    m_RectangleRendering[i] = mitk::RenderWindowFrame::New();
    m_RectangleRendering[i]->SetRenderWindow(GetRenderWindow(i)->GetVtkRenderWindow());
    m_RectangleRendering[i]->Enable(GetDecorationColor(i)[0], GetDecorationColor(i)[1], GetDecorationColor(i)[2]);
  }
}

void QmitkStdMultiWidget::FillGradientBackgroundWithBlack()
{
  //We have 4 widgets and ...
  for(unsigned int i = 0; i < 4; ++i)
  {
    float black[3] = {0.0f, 0.0f, 0.0f};
    m_GradientBackgroundColors[i] = std::make_pair(mitk::Color(black), mitk::Color(black));
  }
}

std::pair<mitk::Color, mitk::Color> QmitkStdMultiWidget::GetGradientColors(unsigned int widgetNumber)
{
  if(widgetNumber > 3)
  {
    MITK_ERROR << "Decoration color for unknown widget!";
    float black[3] = { 0.0f, 0.0f, 0.0f};
    return std::make_pair(mitk::Color(black), mitk::Color(black));
  }
  return m_GradientBackgroundColors[widgetNumber];
}

mitk::Color QmitkStdMultiWidget::GetDecorationColor(unsigned int widgetNumber)
{
  //The implementation looks a bit messy here, but it avoids
  //synchronization of the color of the geometry nodes and an
  //internal member here.
  //Default colors were chosen for decent visibitliy.
  //Feel free to change your preferences in the workbench.
  float tmp[3] = {0.0f,0.0f,0.0f};
  switch (widgetNumber) {
  case 0:
  {
    if(m_PlaneNode1.IsNotNull())
    {
      if(m_PlaneNode1->GetColor(tmp))
      {
        return dynamic_cast<mitk::ColorProperty*>(
              m_PlaneNode1->GetProperty("color"))->GetColor();
      }
    }
    float red[3] = { 0.753f, 0.0f, 0.0f};//This is #C00000 in hex
    return mitk::Color(red);
  }
  case 1:
  {
    if(m_PlaneNode2.IsNotNull())
    {
      if(m_PlaneNode2->GetColor(tmp))
      {
        return dynamic_cast<mitk::ColorProperty*>(
              m_PlaneNode2->GetProperty("color"))->GetColor();
      }
    }
    float green[3] = { 0.0f, 0.69f, 0.0f};//This is #00B000 in hex
    return mitk::Color(green);
  }
  case 2:
  {
    if(m_PlaneNode3.IsNotNull())
    {
      if(m_PlaneNode3->GetColor(tmp))
      {
        return dynamic_cast<mitk::ColorProperty*>(
              m_PlaneNode3->GetProperty("color"))->GetColor();
      }
    }
    float blue[3] = { 0.0, 0.502f, 1.0f};//This is #0080FF in hex
    return mitk::Color(blue);
  }
  case 3:
  {
    return m_DecorationColorWidget4;
  }
  default:
    MITK_ERROR << "Decoration color for unknown widget!";
    float black[3] = { 0.0f, 0.0f, 0.0f};
    return mitk::Color(black);
  }
}

std::string QmitkStdMultiWidget::GetCornerAnnotationText(unsigned int widgetNumber)
{
  if(widgetNumber > 3)
  {
    MITK_ERROR << "Decoration color for unknown widget!";
    return std::string("");
  }
  return std::string(m_CornerAnnotations[widgetNumber].cornerText->GetText(0));
}

QmitkStdMultiWidget::~QmitkStdMultiWidget()
{
  DisablePositionTracking();
  DisableNavigationControllerEventListening();

  m_TimeNavigationController->Disconnect(mitkWidget1->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget2->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget3->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget4->GetSliceNavigationController());
}

QmitkStdMultiWidget::CornerAnnotation QmitkStdMultiWidget::CreateCornerAnnotation(std::string text, mitk::Color color)
{
  CornerAnnotation annotation;
  annotation.cornerText = vtkSmartPointer<vtkCornerAnnotation>::New();
  annotation.cornerText->SetText(0, text.c_str());
  annotation.cornerText->SetMaximumFontSize(12);
  annotation.textProp = vtkSmartPointer<vtkTextProperty>::New();
  annotation.textProp->SetColor( color[0],color[1],color[2] );
  annotation.cornerText->SetTextProperty( annotation.textProp );
  annotation.ren = vtkSmartPointer<vtkRenderer>::New();
  annotation.ren->AddActor(annotation.cornerText);
  annotation.ren->InteractiveOff();
  return annotation;
}

void QmitkStdMultiWidget::RemovePlanesFromDataStorage()
{
  if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull() && m_PlaneNode3.IsNotNull() && m_ParentNodeForGeometryPlanes.IsNotNull())
  {
    if(m_DataStorage.IsNotNull())
    {
      m_DataStorage->Remove(m_PlaneNode1);
      m_DataStorage->Remove(m_PlaneNode2);
      m_DataStorage->Remove(m_PlaneNode3);
      m_DataStorage->Remove(m_ParentNodeForGeometryPlanes);
    }
  }
}

void QmitkStdMultiWidget::AddPlanesToDataStorage()
{
  if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull() && m_PlaneNode3.IsNotNull() && m_ParentNodeForGeometryPlanes.IsNotNull())
  {
    if (m_DataStorage.IsNotNull())
    {
      m_DataStorage->Add(m_ParentNodeForGeometryPlanes);
      m_DataStorage->Add(m_PlaneNode1, m_ParentNodeForGeometryPlanes);
      m_DataStorage->Add(m_PlaneNode2, m_ParentNodeForGeometryPlanes);
      m_DataStorage->Add(m_PlaneNode3, m_ParentNodeForGeometryPlanes);
    }
  }
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesUp()
{
  SMW_INFO << "changing layout to 2D images up... " << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //Set Layout to widget
  this->setLayout(QmitkStdMultiWidgetLayout);

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( Qt::Vertical, m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //insert Widget Container into splitter top
  m_SubSplit1->addWidget( mitkWidget1Container );
  m_SubSplit1->addWidget( mitkWidget2Container );
  m_SubSplit1->addWidget( mitkWidget3Container );

  //set SplitterSize for splitter top
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_SubSplit1->setSizes( splitterSize );

  //insert Widget Container into splitter bottom
  m_SubSplit2->addWidget( mitkWidget4Container );

  //set SplitterSize for splitter m_LayoutSplit
  splitterSize.clear();
  splitterSize.push_back(400);
  splitterSize.push_back(1000);
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt
  m_MainSplit->show();

  //show Widget if hidden
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  //Change Layout Name
  m_Layout = LAYOUT_2D_IMAGES_UP;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_2D_IMAGES_UP );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_2D_IMAGES_UP );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_2D_IMAGES_UP );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_2D_IMAGES_UP );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesLeft()
{
  SMW_INFO << "changing layout to 2D images left... " << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( Qt::Vertical, m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //insert Widget into the splitters
  m_SubSplit1->addWidget( mitkWidget1Container );
  m_SubSplit1->addWidget( mitkWidget2Container );
  m_SubSplit1->addWidget( mitkWidget3Container );

  //set splitterSize of SubSplit1
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_SubSplit1->setSizes( splitterSize );

  m_SubSplit2->addWidget( mitkWidget4Container );

  //set splitterSize of Layout Split
  splitterSize.clear();
  splitterSize.push_back(400);
  splitterSize.push_back(1000);
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show Widget if hidden
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  //update Layout Name
  m_Layout = LAYOUT_2D_IMAGES_LEFT;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_2D_IMAGES_LEFT );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_2D_IMAGES_LEFT );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_2D_IMAGES_LEFT );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_2D_IMAGES_LEFT );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::SetCornerAnnotation( std::string text,
                                               mitk::Color color, int widgetNumber)
{
  if( widgetNumber > 3)
  {
    MITK_ERROR << "Unknown render window for annotation.";
    return;
  }
  mitk::VtkLayerController* layercontroller = mitk::VtkLayerController::GetInstance(this->GetRenderWindow(widgetNumber)->GetRenderWindow());
  //remove the old renderer, because the layercontroller holds a list (vector) of all renderes
  //which needs to be updated
  if(m_CornerAnnotations[widgetNumber].ren != NULL)
  {
    layercontroller->RemoveRenderer(m_CornerAnnotations[widgetNumber].ren);
  }
  //make a new one
  m_CornerAnnotations[widgetNumber] = this->CreateCornerAnnotation(text, color);
  //add it to the list
  layercontroller->InsertForegroundRenderer(m_CornerAnnotations[widgetNumber].ren,true);
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow(unsigned int number)
{
  switch (number) {
  case 0:
    return this->GetRenderWindow1();
  case 1:
    return this->GetRenderWindow2();
  case 2:
    return this->GetRenderWindow3();
  case 3:
    return this->GetRenderWindow4();
  default:
    MITK_ERROR << "Requested unknown render window";
    break;
  }
  return NULL;
}

void QmitkStdMultiWidget::changeLayoutToDefault()
{
  SMW_INFO << "changing layout to default... " << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( Qt::Vertical, m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //insert Widget container into the splitters
  m_SubSplit1->addWidget( mitkWidget1Container );
  m_SubSplit1->addWidget( mitkWidget2Container );

  m_SubSplit2->addWidget( mitkWidget3Container );
  m_SubSplit2->addWidget( mitkWidget4Container );

  //set splitter Size
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_SubSplit1->setSizes( splitterSize );
  m_SubSplit2->setSizes( splitterSize );
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show Widget if hidden
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_DEFAULT;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_DEFAULT );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_DEFAULT );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_DEFAULT );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_DEFAULT );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToBig3D()
{
  SMW_INFO << "changing layout to big 3D ..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //add widget Splitter to main Splitter
  m_MainSplit->addWidget( mitkWidget4Container );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  mitkWidget1->hide();
  mitkWidget2->hide();
  mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_BIG_3D;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_BIG_3D );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_BIG_3D );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_BIG_3D );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_BIG_3D );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToWidget1()
{
  SMW_INFO << "changing layout to big Widget1 ..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //add widget Splitter to main Splitter
  m_MainSplit->addWidget( mitkWidget1Container );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  mitkWidget2->hide();
  mitkWidget3->hide();
  mitkWidget4->hide();

  m_Layout = LAYOUT_WIDGET1;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_WIDGET1 );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_WIDGET1 );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_WIDGET1 );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_WIDGET1 );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToWidget2()
{
  SMW_INFO << "changing layout to big Widget2 ..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //add widget Splitter to main Splitter
  m_MainSplit->addWidget( mitkWidget2Container );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  mitkWidget1->hide();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  mitkWidget3->hide();
  mitkWidget4->hide();

  m_Layout = LAYOUT_WIDGET2;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_WIDGET2 );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_WIDGET2 );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_WIDGET2 );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_WIDGET2 );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToWidget3()
{
  SMW_INFO << "changing layout to big Widget3 ..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //add widget Splitter to main Splitter
  m_MainSplit->addWidget( mitkWidget3Container );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  mitkWidget4->hide();

  m_Layout = LAYOUT_WIDGET3;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_WIDGET3 );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_WIDGET3 );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_WIDGET3 );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_WIDGET3 );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToRowWidget3And4()
{
  SMW_INFO << "changing layout to Widget3 and 4 in a Row..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( Qt::Vertical, m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //add Widgets to splitter
  m_LayoutSplit->addWidget( mitkWidget3Container );
  m_LayoutSplit->addWidget( mitkWidget4Container );

  //set Splitter Size
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_ROW_WIDGET_3_AND_4;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_ROW_WIDGET_3_AND_4 );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_ROW_WIDGET_3_AND_4 );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_ROW_WIDGET_3_AND_4 );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_ROW_WIDGET_3_AND_4 );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToColumnWidget3And4()
{
  SMW_INFO << "changing layout to Widget3 and 4 in one Column..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //add Widgets to splitter
  m_LayoutSplit->addWidget( mitkWidget3Container );
  m_LayoutSplit->addWidget( mitkWidget4Container );

  //set SplitterSize
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_COLUMN_WIDGET_3_AND_4;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_COLUMN_WIDGET_3_AND_4 );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_COLUMN_WIDGET_3_AND_4 );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_COLUMN_WIDGET_3_AND_4 );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_COLUMN_WIDGET_3_AND_4 );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToRowWidgetSmall3andBig4()
{
  SMW_INFO << "changing layout to Widget3 and 4 in a Row..." << std::endl;

  this->changeLayoutToRowWidget3And4();

  m_Layout = LAYOUT_ROW_WIDGET_SMALL3_AND_BIG4;
}

void QmitkStdMultiWidget::changeLayoutToSmallUpperWidget2Big3and4()
{
  SMW_INFO << "changing layout to Widget3 and 4 in a Row..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( Qt::Vertical, m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( Qt::Vertical, m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //insert Widget into the splitters
  m_SubSplit1->addWidget( mitkWidget2Container );

  m_SubSplit2->addWidget( mitkWidget3Container );
  m_SubSplit2->addWidget( mitkWidget4Container );

  //set Splitter Size
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_SubSplit2->setSizes( splitterSize );
  splitterSize.clear();
  splitterSize.push_back(500);
  splitterSize.push_back(1000);
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt
  m_MainSplit->show();

  //show Widget if hidden
  mitkWidget1->hide();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4 );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4 );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4 );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4 );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutTo2x2Dand3DWidget()
{
  SMW_INFO << "changing layout to 2 x 2D and 3D Widget" << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( Qt::Vertical, m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //add Widgets to splitter
  m_SubSplit1->addWidget( mitkWidget1Container );
  m_SubSplit1->addWidget( mitkWidget2Container );
  m_SubSplit2->addWidget( mitkWidget4Container );

  //set Splitter Size
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_SubSplit1->setSizes( splitterSize );
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_2X_2D_AND_3D_WIDGET;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_2X_2D_AND_3D_WIDGET );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_2X_2D_AND_3D_WIDGET );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_2X_2D_AND_3D_WIDGET );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_2X_2D_AND_3D_WIDGET );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutToLeft2Dand3DRight2D()
{
  SMW_INFO << "changing layout to 2D and 3D left, 2D right Widget" << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( Qt::Vertical, m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //add Widgets to splitter
  m_SubSplit1->addWidget( mitkWidget1Container );
  m_SubSplit1->addWidget( mitkWidget4Container );
  m_SubSplit2->addWidget( mitkWidget2Container );

  //set Splitter Size
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_SubSplit1->setSizes( splitterSize );
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET );

  //update Alle Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::changeLayoutTo2DUpAnd3DDown()
{
  SMW_INFO << "changing layout to 2D up and 3D down" << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout ;

  //create Main Layout
  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );

  //Set Layout to widget
  this->setLayout(QmitkStdMultiWidgetLayout);

  //create main splitter
  m_MainSplit = new QSplitter( this );
  QmitkStdMultiWidgetLayout->addWidget( m_MainSplit );

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter( Qt::Vertical, m_MainSplit );
  m_MainSplit->addWidget( m_LayoutSplit );

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget( levelWindowWidget );

  //create m_SubSplit1 and m_SubSplit2
  m_SubSplit1 = new QSplitter( m_LayoutSplit );
  m_SubSplit2 = new QSplitter( m_LayoutSplit );

  //insert Widget Container into splitter top
  m_SubSplit1->addWidget( mitkWidget1Container );

  //set SplitterSize for splitter top
   QList<int> splitterSize;
  //insert Widget Container into splitter bottom
  m_SubSplit2->addWidget( mitkWidget4Container );
  //set SplitterSize for splitter m_LayoutSplit
  splitterSize.clear();
  splitterSize.push_back(700);
  splitterSize.push_back(700);
  m_LayoutSplit->setSizes( splitterSize );

  //show mainSplitt
  m_MainSplit->show();

  //show/hide Widgets
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  mitkWidget2->hide();
  mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  m_Layout = LAYOUT_2D_UP_AND_3D_DOWN;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged( LAYOUT_2D_UP_AND_3D_DOWN );
  mitkWidget2->LayoutDesignListChanged( LAYOUT_2D_UP_AND_3D_DOWN );
  mitkWidget3->LayoutDesignListChanged( LAYOUT_2D_UP_AND_3D_DOWN );
  mitkWidget4->LayoutDesignListChanged( LAYOUT_2D_UP_AND_3D_DOWN );

  //update all Widgets
  this->UpdateAllWidgets();
}

void QmitkStdMultiWidget::SetDataStorage( mitk::DataStorage* ds )
{
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->SetDataStorage(ds);
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->SetDataStorage(ds);
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->SetDataStorage(ds);
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->SetDataStorage(ds);
  m_DataStorage = ds;
}

void QmitkStdMultiWidget::Fit()
{
  vtkSmartPointer<vtkRenderer> vtkrenderer;
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetDisplayGeometry()->Fit();

  int w = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL )
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL )
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL )
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL )
    vtkrenderer->ResetCamera();

  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::InitPositionTracking()
{
  //PoinSetNode for MouseOrientation
  m_PositionTrackerNode = mitk::DataNode::New();
  m_PositionTrackerNode->SetProperty("name", mitk::StringProperty::New("Mouse Position"));
  m_PositionTrackerNode->SetData( mitk::PointSet::New() );
  m_PositionTrackerNode->SetColor(1.0,0.33,0.0);
  m_PositionTrackerNode->SetProperty("layer", mitk::IntProperty::New(1001));
  m_PositionTrackerNode->SetVisibility(true);
  m_PositionTrackerNode->SetProperty("inputdevice", mitk::BoolProperty::New(true) );
  m_PositionTrackerNode->SetProperty("BaseRendererMapperID", mitk::IntProperty::New(0) );//point position 2D mouse
  m_PositionTrackerNode->SetProperty("baserenderer", mitk::StringProperty::New("N/A"));
}

void QmitkStdMultiWidget::AddDisplayPlaneSubTree()
{
  // add the displayed planes of the multiwidget to a node to which the subtree
  // @a planesSubTree points ...

  mitk::PlaneGeometryDataMapper2D::Pointer mapper;

  // ... of widget 1
  mitk::BaseRenderer* renderer1 = mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow());
  m_PlaneNode1 = renderer1->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode1->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode1->SetProperty("name", mitk::StringProperty::New(std::string(renderer1->GetName()) + ".plane"));
  m_PlaneNode1->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode1->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 2
  mitk::BaseRenderer* renderer2 = mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow());
  m_PlaneNode2 = renderer2->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode2->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("name", mitk::StringProperty::New(std::string(renderer2->GetName()) + ".plane"));
  m_PlaneNode2->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode2->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode2->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 3
  mitk::BaseRenderer* renderer3 = mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow());
  m_PlaneNode3 = renderer3->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("name", mitk::StringProperty::New(std::string(renderer3->GetName()) + ".plane"));
  m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode3->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  m_ParentNodeForGeometryPlanes = mitk::DataNode::New();
  m_ParentNodeForGeometryPlanes->SetProperty("name", mitk::StringProperty::New("Widgets"));
  m_ParentNodeForGeometryPlanes->SetProperty("helper object", mitk::BoolProperty::New(true));
}

mitk::SliceNavigationController* QmitkStdMultiWidget::GetTimeNavigationController()
{
  return m_TimeNavigationController;
}

void QmitkStdMultiWidget::EnableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  levelWindowWidget->SetDataStorage(mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetDataStorage());
  levelWindowWidget->show();
}

void QmitkStdMultiWidget::DisableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  levelWindowWidget->hide();
}

// CAUTION: Legacy code for enabling Qt-signal-controlled view initialization.
// Use RenderingManager::InitializeViews() instead.
bool QmitkStdMultiWidget::InitializeStandardViews( const mitk::Geometry3D * geometry )
{
  return m_RenderingManager->InitializeViews( geometry );
}

void QmitkStdMultiWidget::RequestUpdate()
{
  m_RenderingManager->RequestUpdate(mitkWidget1->GetRenderWindow());
  m_RenderingManager->RequestUpdate(mitkWidget2->GetRenderWindow());
  m_RenderingManager->RequestUpdate(mitkWidget3->GetRenderWindow());
  m_RenderingManager->RequestUpdate(mitkWidget4->GetRenderWindow());
}

void QmitkStdMultiWidget::ForceImmediateUpdate()
{
  m_RenderingManager->ForceImmediateUpdate(mitkWidget1->GetRenderWindow());
  m_RenderingManager->ForceImmediateUpdate(mitkWidget2->GetRenderWindow());
  m_RenderingManager->ForceImmediateUpdate(mitkWidget3->GetRenderWindow());
  m_RenderingManager->ForceImmediateUpdate(mitkWidget4->GetRenderWindow());
}

void QmitkStdMultiWidget::wheelEvent( QWheelEvent * e )
{
  emit WheelMoved( e );
}

void QmitkStdMultiWidget::mousePressEvent(QMouseEvent * e)
{
   if (e->button() == Qt::LeftButton) {
       mitk::Point3D pointValue = this->GetLastLeftClickPosition();
       emit LeftMouseClicked(pointValue);
   }
}

void QmitkStdMultiWidget::moveEvent( QMoveEvent* e )
{
  QWidget::moveEvent( e );

  // it is necessary to readjust the position of the overlays as the StdMultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

void QmitkStdMultiWidget::leaveEvent ( QEvent * /*e*/  )
{
  //set cursor back to initial state
  m_SlicesRotator->ResetMouseCursor();
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow1() const
{
  return mitkWidget1;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow2() const
{
  return mitkWidget2;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow3() const
{
  return mitkWidget3;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow4() const
{
  return mitkWidget4;
}

const mitk::Point3D& QmitkStdMultiWidget::GetLastLeftClickPosition() const
{
  return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}

const mitk::Point3D QmitkStdMultiWidget::GetCrossPosition() const
{
  const mitk::PlaneGeometry *plane1 =
    mitkWidget1->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane2 =
    mitkWidget2->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane3 =
    mitkWidget3->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ( (plane1 != NULL) && (plane2 != NULL)
    && (plane1->IntersectionLine( plane2, line )) )
  {
    mitk::Point3D point;
    if ( (plane3 != NULL)
      && (plane3->IntersectionPoint( line, point )) )
    {
      return point;
    }
  }
  return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}

void QmitkStdMultiWidget::EnablePositionTracking()
{
  if (!m_PositionTracker)
  {
    m_PositionTracker = mitk::PositionTracker::New("PositionTracker", NULL);
  }
  mitk::GlobalInteraction* globalInteraction = mitk::GlobalInteraction::GetInstance();
  if (globalInteraction)
  {
    if(m_DataStorage.IsNotNull())
      m_DataStorage->Add(m_PositionTrackerNode);
    globalInteraction->AddListener(m_PositionTracker);
  }
}

void QmitkStdMultiWidget::DisablePositionTracking()
{
  mitk::GlobalInteraction* globalInteraction =
    mitk::GlobalInteraction::GetInstance();

  if(globalInteraction)
  {
    if (m_DataStorage.IsNotNull())
      m_DataStorage->Remove(m_PositionTrackerNode);
    globalInteraction->RemoveListener(m_PositionTracker);
  }
}

void QmitkStdMultiWidget::EnsureDisplayContainsPoint(
  mitk::DisplayGeometry* displayGeometry, const mitk::Point3D& p)
{
  mitk::Point2D pointOnPlane;
  displayGeometry->Map( p, pointOnPlane );

  // point minus origin < width or height ==> outside ?
  mitk::Vector2D pointOnRenderWindow_MM;
  pointOnRenderWindow_MM = pointOnPlane.GetVectorFromOrigin()
    - displayGeometry->GetOriginInMM();

  mitk::Vector2D sizeOfDisplay( displayGeometry->GetSizeInMM() );

  if (   sizeOfDisplay[0] < pointOnRenderWindow_MM[0]
  ||                0 > pointOnRenderWindow_MM[0]
  || sizeOfDisplay[1] < pointOnRenderWindow_MM[1]
  ||                0 > pointOnRenderWindow_MM[1] )
  {
    // point is not visible -> move geometry
    mitk::Vector2D offset( (pointOnRenderWindow_MM - sizeOfDisplay / 2.0)
      / displayGeometry->GetScaleFactorMMPerDisplayUnit() );

    displayGeometry->MoveBy( offset );
  }
}

void QmitkStdMultiWidget::MoveCrossToPosition(const mitk::Point3D& newPosition)
{
  // create a PositionEvent with the given position and
  // tell the slice navigation controllers to move there
  mitk::Point2D p2d;
  mitk::PositionEvent event( mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow()), 0, 0, 0,
    mitk::Key_unknown, p2d, newPosition );
  mitk::StateEvent stateEvent(mitk::EIDLEFTMOUSEBTN, &event);
  mitk::StateEvent stateEvent2(mitk::EIDLEFTMOUSERELEASE, &event);

  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    mitkWidget1->GetSliceNavigationController()->HandleEvent( &stateEvent );
    mitkWidget2->GetSliceNavigationController()->HandleEvent( &stateEvent );
    mitkWidget3->GetSliceNavigationController()->HandleEvent( &stateEvent );

    // just in case SNCs will develop something that depends on the mouse
    // button being released again
    mitkWidget1->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    mitkWidget2->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    mitkWidget3->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    break;

  case PLANE_MODE_ROTATION:
    m_SlicesRotator->HandleEvent( &stateEvent );

    // just in case SNCs will develop something that depends on the mouse
    // button being released again
    m_SlicesRotator->HandleEvent( &stateEvent2 );
    break;

  case PLANE_MODE_SWIVEL:
    m_SlicesSwiveller->HandleEvent( &stateEvent );

    // just in case SNCs will develop something that depends on the mouse
    // button being released again
    m_SlicesSwiveller->HandleEvent( &stateEvent2 );
    break;
  }

  // determine if cross is now out of display
  // if so, move the display window
  EnsureDisplayContainsPoint( mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())
    ->GetDisplayGeometry(), newPosition );
  EnsureDisplayContainsPoint( mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())
    ->GetDisplayGeometry(), newPosition );
  EnsureDisplayContainsPoint( mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())
    ->GetDisplayGeometry(), newPosition );

  // update displays
  m_RenderingManager->RequestUpdateAll();
}

void QmitkStdMultiWidget::HandleCrosshairPositionEvent()
{
  if(!m_PendingCrosshairPositionEvent)
  {
    m_PendingCrosshairPositionEvent=true;
    QTimer::singleShot(0,this,SLOT( HandleCrosshairPositionEventDelayed() ) );
  }
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes)
{
  mitk::Point3D crosshairPos = this->GetCrossPosition();
  mitk::DataNode::Pointer node;
  int  maxlayer = -32768;

  if(nodes.IsNotNull())
  {
    mitk::BaseRenderer* baseRenderer = this->mitkWidget1->GetSliceNavigationController()->GetRenderer();
    // find node with largest layer, that is the node shown on top in the render window
    for (unsigned int x = 0; x < nodes->size(); x++)
    {
    if ( (nodes->at(x)->GetData()->GetGeometry() != NULL) &&
         nodes->at(x)->GetData()->GetGeometry()->IsInside(crosshairPos) )
    {
      int layer = 0;
      if(!(nodes->at(x)->GetIntProperty("layer", layer))) continue;
      if(layer > maxlayer)
      {
        if( static_cast<mitk::DataNode::Pointer>(nodes->at(x))->IsVisible( baseRenderer ) )
        {
          node = nodes->at(x);
          maxlayer = layer;
        }
      }
    }
    }
  }
  return node;
}

void QmitkStdMultiWidget::HandleCrosshairPositionEventDelayed()
{
  m_PendingCrosshairPositionEvent = false;

  // find image with highest layer
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->m_DataStorage->GetSubset(isImageData).GetPointer();

  mitk::DataNode::Pointer node;
  mitk::DataNode::Pointer topSourceNode;
  mitk::Image::Pointer image;
  bool isBinary = false;
  node = this->GetTopLayerNode(nodes);
  int component = 0;
  if(node.IsNotNull())
  {
    node->GetBoolProperty("binary",isBinary);
    if(isBinary)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = m_DataStorage->GetSources(node, NULL, true);
      if(!sourcenodes->empty())
      {
        topSourceNode = this->GetTopLayerNode(sourcenodes);
      }
      if(topSourceNode.IsNotNull())
      {
        image = dynamic_cast<mitk::Image*>(topSourceNode->GetData());
        topSourceNode->GetIntProperty("Image.Displayed Component", component);
      }
      else
      {
        image = dynamic_cast<mitk::Image*>(node->GetData());
        node->GetIntProperty("Image.Displayed Component", component);
      }
    }
    else
    {
      image = dynamic_cast<mitk::Image*>(node->GetData());
      node->GetIntProperty("Image.Displayed Component", component);
    }
  }

  mitk::Point3D crosshairPos = this->GetCrossPosition();
  std::string statusText;
  std::stringstream stream;
  itk::Index<3> p;
  mitk::BaseRenderer* baseRenderer = this->mitkWidget1->GetSliceNavigationController()->GetRenderer();
  unsigned int timestep = baseRenderer->GetTimeStep();

  if(image.IsNotNull() && (image->GetTimeSteps() > timestep ))
  {
    image->GetGeometry()->WorldToIndex(crosshairPos, p);
    stream.precision(2);
    stream<<"Position: <" << std::fixed <<crosshairPos[0] << ", " << std::fixed << crosshairPos[1] << ", " << std::fixed << crosshairPos[2] << "> mm";
    stream<<"; Index: <"<<p[0] << ", " << p[1] << ", " << p[2] << "> ";
    mitk::ScalarType pixelValue = image->GetPixelValueByIndex(p, timestep, component);
    if (fabs(pixelValue)>1000000 || fabs(pixelValue) < 0.01)
    {
      stream<<"; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: "<< std::scientific<< pixelValue <<"  ";
    }
    else
    {
      stream<<"; Time: " << baseRenderer->GetTime() << " ms; Pixelvalue: "<< pixelValue <<"  ";
    }
  }
  else
  {
    stream << "No image information at this position!";
  }

  statusText = stream.str();
  mitk::StatusBar::GetInstance()->DisplayGreyValueText(statusText.c_str());
}

void QmitkStdMultiWidget::EnableNavigationControllerEventListening()
{
  // Let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();

  //// Listen for SliceNavigationController
  //TODO 18735 can this be deleted ??
  //mitkWidget1->GetSliceNavigationController()->crosshairPositionEvent.AddListener( mitk::MessageDelegate<QmitkStdMultiWidget>( this, &QmitkStdMultiWidget::HandleCrosshairPositionEvent ) );
  //mitkWidget2->GetSliceNavigationController()->crosshairPositionEvent.AddListener( mitk::MessageDelegate<QmitkStdMultiWidget>( this, &QmitkStdMultiWidget::HandleCrosshairPositionEvent ) );
  //mitkWidget3->GetSliceNavigationController()->crosshairPositionEvent.AddListener( mitk::MessageDelegate<QmitkStdMultiWidget>( this, &QmitkStdMultiWidget::HandleCrosshairPositionEvent ) );

  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    gi->AddListener( mitkWidget1->GetSliceNavigationController() );
    gi->AddListener( mitkWidget2->GetSliceNavigationController() );
    gi->AddListener( mitkWidget3->GetSliceNavigationController() );
    gi->AddListener( mitkWidget4->GetSliceNavigationController() );
    break;

  case PLANE_MODE_ROTATION:
    gi->AddListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    gi->AddListener( m_SlicesSwiveller );
    break;
  }

  gi->AddListener( m_TimeNavigationController );
  m_CrosshairNavigationEnabled = true;
}

void QmitkStdMultiWidget::DisableNavigationControllerEventListening()
{
  // Do not let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();

  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    gi->RemoveListener( mitkWidget1->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget2->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget3->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget4->GetSliceNavigationController() );
    break;

  case PLANE_MODE_ROTATION:
    m_SlicesRotator->ResetMouseCursor();
    gi->RemoveListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    m_SlicesSwiveller->ResetMouseCursor();
    gi->RemoveListener( m_SlicesSwiveller );
    break;
  }

  gi->RemoveListener( m_TimeNavigationController );
  m_CrosshairNavigationEnabled = false;
}

int QmitkStdMultiWidget::GetLayout() const
{
  return m_Layout;
}

bool QmitkStdMultiWidget::GetGradientBackgroundFlag() const
{
  return m_GradientBackgroundFlag;
}

void QmitkStdMultiWidget::EnableGradientBackground()
{
  // gradient background is by default only in widget 4, otherwise
  // interferences between 2D rendering and VTK rendering may occur.
  for(unsigned int i = 0; i < 4; ++i)
  {
    m_GradientBackground[i]->Enable();
  }
  m_GradientBackgroundFlag = true;
}

void QmitkStdMultiWidget::DisableGradientBackground()
{
  for(unsigned int i = 0; i < 4; ++i)
  {
    m_GradientBackground[i]->Disable();
  }
  m_GradientBackgroundFlag = false;
}

void QmitkStdMultiWidget::EnableDepartmentLogo()
{
  m_LogoRendering->SetVisibility(true);
}

void QmitkStdMultiWidget::DisableDepartmentLogo()
{
  m_LogoRendering->SetVisibility(false);
}

bool QmitkStdMultiWidget::IsDepartmentLogoEnabled() const
{
  return m_LogoRendering->IsVisible(mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));
}

bool QmitkStdMultiWidget::IsCrosshairNavigationEnabled() const
{
  return m_CrosshairNavigationEnabled;
}

mitk::SlicesRotator * QmitkStdMultiWidget::GetSlicesRotator() const
{
  return m_SlicesRotator;
}

mitk::SlicesSwiveller * QmitkStdMultiWidget::GetSlicesSwiveller() const
{
  return m_SlicesSwiveller;
}

void QmitkStdMultiWidget::SetWidgetPlaneVisibility(const char* widgetName, bool visible, mitk::BaseRenderer *renderer)
{
  if (m_DataStorage.IsNotNull())
  {
    mitk::DataNode* n = m_DataStorage->GetNamedNode(widgetName);
    if (n != NULL)
      n->SetVisibility(visible, renderer);
  }
}

void QmitkStdMultiWidget::SetWidgetPlanesVisibility(bool visible, mitk::BaseRenderer *renderer)
{
  if (m_PlaneNode1.IsNotNull())
  {
    m_PlaneNode1->SetVisibility(visible, renderer);
  }
  if (m_PlaneNode2.IsNotNull())
  {
    m_PlaneNode2->SetVisibility(visible, renderer);
  }
  if (m_PlaneNode3.IsNotNull())
  {
    m_PlaneNode3->SetVisibility(visible, renderer);
  }
  m_RenderingManager->RequestUpdateAll();
}

void QmitkStdMultiWidget::SetWidgetPlanesLocked(bool locked)
{
  //do your job and lock or unlock slices.
  GetRenderWindow1()->GetSliceNavigationController()->SetSliceLocked(locked);
  GetRenderWindow2()->GetSliceNavigationController()->SetSliceLocked(locked);
  GetRenderWindow3()->GetSliceNavigationController()->SetSliceLocked(locked);
}

void QmitkStdMultiWidget::SetWidgetPlanesRotationLocked(bool locked)
{
  //do your job and lock or unlock slices.
  GetRenderWindow1()->GetSliceNavigationController()->SetSliceRotationLocked(locked);
  GetRenderWindow2()->GetSliceNavigationController()->SetSliceRotationLocked(locked);
  GetRenderWindow3()->GetSliceNavigationController()->SetSliceRotationLocked(locked);
}

void QmitkStdMultiWidget::SetWidgetPlanesRotationLinked( bool link )
{
  m_SlicesRotator->SetLinkPlanes( link );
  m_SlicesSwiveller->SetLinkPlanes( link );
  emit WidgetPlanesRotationLinked( link );
}

void QmitkStdMultiWidget::SetWidgetPlaneMode( int userMode )
{
  MITK_DEBUG << "Changing crosshair mode to " << userMode;

  // first of all reset left mouse button interaction to default if PACS interaction style is active
  m_MouseModeSwitcher->SelectMouseMode( mitk::MouseModeSwitcher::MousePointer );

  emit WidgetNotifyNewCrossHairMode( userMode );

  int mode = m_PlaneMode;
  bool link = false;

  // Convert user interface mode to actual mode
  {
    switch(userMode)
    {
      case 0:
        mode = PLANE_MODE_SLICING;
        link = false;
        break;

      case 1:
        mode = PLANE_MODE_ROTATION;
        link = false;
        break;

      case 2:
        mode = PLANE_MODE_ROTATION;
        link = true;
        break;

      case 3:
        mode = PLANE_MODE_SWIVEL;
        link = false;
        break;
    }
  }

  // Slice rotation linked
  m_SlicesRotator->SetLinkPlanes( link );
  m_SlicesSwiveller->SetLinkPlanes( link );

  // Do nothing if mode didn't change
  if ( m_PlaneMode == mode )
  {
    return;
  }

  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();

  // Remove listeners of previous mode
  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    // Notify MainTemplate GUI that this mode has been deselected
    emit WidgetPlaneModeSlicing( false );

    gi->RemoveListener( mitkWidget1->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget2->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget3->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget4->GetSliceNavigationController() );
    break;

  case PLANE_MODE_ROTATION:
    // Notify MainTemplate GUI that this mode has been deselected
    emit WidgetPlaneModeRotation( false );

    m_SlicesRotator->ResetMouseCursor();
    gi->RemoveListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    // Notify MainTemplate GUI that this mode has been deselected
    emit WidgetPlaneModeSwivel( false );

    m_SlicesSwiveller->ResetMouseCursor();
    gi->RemoveListener( m_SlicesSwiveller );
    break;
  }

  // Set new mode and add corresponding listener to GlobalInteraction
  m_PlaneMode = mode;
  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    // Notify MainTemplate GUI that this mode has been selected
    emit WidgetPlaneModeSlicing( true );

    // Add listeners
    gi->AddListener( mitkWidget1->GetSliceNavigationController() );
    gi->AddListener( mitkWidget2->GetSliceNavigationController() );
    gi->AddListener( mitkWidget3->GetSliceNavigationController() );
    gi->AddListener( mitkWidget4->GetSliceNavigationController() );

    m_RenderingManager->InitializeViews();
    break;

  case PLANE_MODE_ROTATION:
    // Notify MainTemplate GUI that this mode has been selected
    emit WidgetPlaneModeRotation( true );

    // Add listener
    gi->AddListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    // Notify MainTemplate GUI that this mode has been selected
    emit WidgetPlaneModeSwivel( true );

    // Add listener
    gi->AddListener( m_SlicesSwiveller );
    break;
  }
  // Notify MainTemplate GUI that mode has changed
  emit WidgetPlaneModeChange(m_PlaneMode);
}

void QmitkStdMultiWidget::SetGradientBackgroundColorForRenderWindow( const mitk::Color & upper, const mitk::Color & lower, unsigned int widgetNumber )
{

  if(widgetNumber > 3)
  {
    MITK_ERROR << "Gradientbackground for unknown widget!";
    return;
  }
  m_GradientBackgroundColors[widgetNumber].first = upper;
  m_GradientBackgroundColors[widgetNumber].second = lower;
  m_GradientBackground[widgetNumber]->SetGradientColors(upper, lower);
  m_GradientBackgroundFlag = true;
}

void QmitkStdMultiWidget::SetGradientBackgroundColors( const mitk::Color & upper, const mitk::Color & lower )
{
  for(unsigned int i = 0; i < 4; ++i)
  {
    m_GradientBackground[i]->SetGradientColors(upper, lower);
  }
  m_GradientBackgroundFlag = true;
}

void QmitkStdMultiWidget::SetDepartmentLogoPath( const char * path )
{
  m_LogoRendering->SetLogoImagePath(path);
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow());
  m_LogoRendering->Update(renderer);
  RequestUpdate();
}

void QmitkStdMultiWidget::SetWidgetPlaneModeToSlicing( bool activate )
{
  if ( activate )
  {
    this->SetWidgetPlaneMode( PLANE_MODE_SLICING );
  }
}

void QmitkStdMultiWidget::SetWidgetPlaneModeToRotation( bool activate )
{
  if ( activate )
  {
    this->SetWidgetPlaneMode( PLANE_MODE_ROTATION );
  }
}

void QmitkStdMultiWidget::SetWidgetPlaneModeToSwivel( bool activate )
{
  if ( activate )
  {
    this->SetWidgetPlaneMode( PLANE_MODE_SWIVEL );
  }
}

void QmitkStdMultiWidget::OnLayoutDesignChanged( int layoutDesignIndex )
{
  switch( layoutDesignIndex )
  {
  case LAYOUT_DEFAULT:
    {
      this->changeLayoutToDefault();
      break;
    }
  case LAYOUT_2D_IMAGES_UP:
    {
      this->changeLayoutTo2DImagesUp();
      break;
    }
  case LAYOUT_2D_IMAGES_LEFT:
    {
      this->changeLayoutTo2DImagesLeft();
      break;
    }
  case LAYOUT_BIG_3D:
    {
      this->changeLayoutToBig3D();
      break;
    }
  case LAYOUT_WIDGET1:
    {
      this->changeLayoutToWidget1();
      break;
    }
  case LAYOUT_WIDGET2:
    {
      this->changeLayoutToWidget2();
      break;
    }
  case LAYOUT_WIDGET3:
    {
      this->changeLayoutToWidget3();
      break;
    }
  case LAYOUT_2X_2D_AND_3D_WIDGET:
    {
      this->changeLayoutTo2x2Dand3DWidget();
      break;
    }
  case LAYOUT_ROW_WIDGET_3_AND_4:
    {
      this->changeLayoutToRowWidget3And4();
      break;
    }
  case LAYOUT_COLUMN_WIDGET_3_AND_4:
    {
      this->changeLayoutToColumnWidget3And4();
      break;
    }
  case LAYOUT_ROW_WIDGET_SMALL3_AND_BIG4:
    {
      this->changeLayoutToRowWidgetSmall3andBig4();
      break;
    }
  case LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4:
    {
      this->changeLayoutToSmallUpperWidget2Big3and4();
      break;
    }
  case LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET:
    {
      this->changeLayoutToLeft2Dand3DRight2D();
      break;
    }
  };
}

void QmitkStdMultiWidget::UpdateAllWidgets()
{
  mitkWidget1->resize( mitkWidget1Container->frameSize().width()-1, mitkWidget1Container->frameSize().height() );
  mitkWidget1->resize( mitkWidget1Container->frameSize().width(), mitkWidget1Container->frameSize().height() );

  mitkWidget2->resize( mitkWidget2Container->frameSize().width()-1, mitkWidget2Container->frameSize().height() );
  mitkWidget2->resize( mitkWidget2Container->frameSize().width(), mitkWidget2Container->frameSize().height() );

  mitkWidget3->resize( mitkWidget3Container->frameSize().width()-1, mitkWidget3Container->frameSize().height() );
  mitkWidget3->resize( mitkWidget3Container->frameSize().width(), mitkWidget3Container->frameSize().height() );

  mitkWidget4->resize( mitkWidget4Container->frameSize().width()-1, mitkWidget4Container->frameSize().height() );
  mitkWidget4->resize( mitkWidget4Container->frameSize().width(), mitkWidget4Container->frameSize().height() );
}

void QmitkStdMultiWidget::HideAllWidgetToolbars()
{
  mitkWidget1->HideRenderWindowMenu();
  mitkWidget2->HideRenderWindowMenu();
  mitkWidget3->HideRenderWindowMenu();
  mitkWidget4->HideRenderWindowMenu();
}

void QmitkStdMultiWidget::ActivateMenuWidget( bool state )
{
  mitkWidget1->ActivateMenuWidget( state, this );
  mitkWidget2->ActivateMenuWidget( state, this );
  mitkWidget3->ActivateMenuWidget( state, this );
  mitkWidget4->ActivateMenuWidget( state, this );
}

bool QmitkStdMultiWidget::IsMenuWidgetEnabled() const
{
  return mitkWidget1->GetActivateMenuWidgetFlag();
}

void QmitkStdMultiWidget::SetDecorationColor(unsigned int widgetNumber, mitk::Color color)
{
  switch (widgetNumber) {
  case 0:
    if(m_PlaneNode1.IsNotNull())
    {
      m_PlaneNode1->SetColor(color);
    }
    break;
  case 1:
    if(m_PlaneNode2.IsNotNull())
    {
      m_PlaneNode2->SetColor(color);
    }
    break;
  case 2:
    if(m_PlaneNode3.IsNotNull())
    {
      m_PlaneNode3->SetColor(color);
    }
    break;
  case 3:
    m_DecorationColorWidget4 = color;
    break;
  default:
    MITK_ERROR << "Decoration color for unknown widget!";
    break;
  }
}

void QmitkStdMultiWidget::ResetCrosshair()
{
  if (m_DataStorage.IsNotNull())
  {
    m_RenderingManager->InitializeViewsByBoundingObjects(m_DataStorage);
    //m_RenderingManager->InitializeViews( m_DataStorage->ComputeVisibleBoundingGeometry3D() );
    // reset interactor to normal slicing
    this->SetWidgetPlaneMode(PLANE_MODE_SLICING);
  }
}

void QmitkStdMultiWidget::EnableColoredRectangles()
{
  for(unsigned int i = 0; i < 4; ++i)
  {
    m_RectangleRendering[i]->Enable(
          GetDecorationColor(i)[0],
          GetDecorationColor(i)[1],
          GetDecorationColor(i)[2]);
  }
}

void QmitkStdMultiWidget::DisableColoredRectangles()
{
  m_RectangleRendering[0]->Disable();
  m_RectangleRendering[1]->Disable();
  m_RectangleRendering[2]->Disable();
  m_RectangleRendering[3]->Disable();
}

bool QmitkStdMultiWidget::IsColoredRectanglesEnabled() const
{
  return m_RectangleRendering[0]->IsEnabled();
}

mitk::MouseModeSwitcher* QmitkStdMultiWidget::GetMouseModeSwitcher()
{
  return m_MouseModeSwitcher;
}

void QmitkStdMultiWidget::MouseModeSelected( mitk::MouseModeSwitcher::MouseMode mouseMode )
{
  if ( mouseMode == 0 )
  {
    this->EnableNavigationControllerEventListening();
  }
  else
  {
    this->DisableNavigationControllerEventListening();
  }
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane1()
{
  return this->m_PlaneNode1;
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane2()
{
  return this->m_PlaneNode2;
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane3()
{
  return this->m_PlaneNode3;
}

mitk::DataNode::Pointer QmitkStdMultiWidget::GetWidgetPlane(int id)
{
  switch(id)
  {
    case 1: return this->m_PlaneNode1;
    break;
    case 2: return this->m_PlaneNode2;
    break;
    case 3: return this->m_PlaneNode3;
    break;
    default: return NULL;
  }
}
