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

#include <boost/format.hpp>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <qsplitter.h>
#include <QList>
#include <QMouseEvent>
#include <QTimer>
#include <QStringList>
#include <QDir>

#include <mitkProperties.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkPointSet.h>
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
#include <mitkCameraController.h>
#include <mitkDataNodePickingEventObserver.h>

#include <vtkRendererCollection.h>
#include <vtkTextProperty.h>
#include <vtkCornerAnnotation.h>
#include <vtkMitkRectangleProp.h>
#include "mitkPixelTypeMultiplex.h"
#include "mitkImagePixelReadAccessor.h"

#include <PathUtilities.h>

void QmitkStdMultiWidget::UpdateAnnotationFonts()
{
  for (int i = 0; i < 4; i++) {
    m_CornerAnnotations[i]->QueueFontUpdate();
  }
}

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
  m_CrosshairNavigationEnabled(false),
  m_drawTextInStatusBar(true),
  m_ImageMTime(0),
  m_displayMetaInfo(false),
  m_displayMetaInfoEx(false),
  m_displayPatientInfo(true),
  m_displayPatientInfoEx(true),
  m_displayPositionInfo(true),
  m_Name(name)
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

  m_CornerAnnotations[0] = vtkSmartPointer<vtkCornerAnnotation>::New();
  m_CornerAnnotations[1] = vtkSmartPointer<vtkCornerAnnotation>::New();
  m_CornerAnnotations[2] = vtkSmartPointer<vtkCornerAnnotation>::New();
  m_CornerAnnotations[3] = vtkSmartPointer<vtkCornerAnnotation>::New();

  m_RectangleProps[0] = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleProps[1] = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleProps[2] = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleProps[3] = vtkSmartPointer<vtkMitkRectangleProp>::New();

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

  //Create RenderWindows 1
  mitkWidget1 = new QmitkRenderWindow(mitkWidget1Container, name + ".widget1", NULL, m_RenderingManager, renderingMode);
  mitkWidget1->SetLayoutIndex( AXIAL );
  mitkWidgetLayout1->addWidget(mitkWidget1);

  //Create RenderWindows 2
  mitkWidget2 = new QmitkRenderWindow(mitkWidget2Container, name + ".widget2", NULL, m_RenderingManager, renderingMode);
  mitkWidget2->setEnabled( true );
  mitkWidget2->SetLayoutIndex( SAGITTAL );
  mitkWidgetLayout2->addWidget(mitkWidget2);

  //Create RenderWindows 3
  mitkWidget3 = new QmitkRenderWindow(mitkWidget3Container, name + ".widget3", NULL, m_RenderingManager, renderingMode);
  mitkWidget3->SetLayoutIndex( CORONAL );
  mitkWidgetLayout3->addWidget(mitkWidget3);

  //Create RenderWindows 4
  mitkWidget4 = new QmitkRenderWindow(mitkWidget4Container, name + ".widget4", NULL, m_RenderingManager, renderingMode);
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
  levelWindowWidget->setMaximumWidth(50);

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

  m_ResizeTimer.setSingleShot(true);
  connect(&m_ResizeTimer, &QTimer::timeout, this, &QmitkStdMultiWidget::OnResizeStoped);
  connect(mitkWidget1, &QmitkRenderWindow::resized, this, &QmitkStdMultiWidget::OnWindowResized);
  connect(mitkWidget2, &QmitkRenderWindow::resized, this, &QmitkStdMultiWidget::OnWindowResized);
  connect(mitkWidget3, &QmitkRenderWindow::resized, this, &QmitkStdMultiWidget::OnWindowResized);
  connect(mitkWidget4, &QmitkRenderWindow::resized, this, &QmitkStdMultiWidget::OnWindowResized);
}

void QmitkStdMultiWidget::InitializeWidget()
{
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

  std::string axialCornerAnnotation = std::string(tr("Axial").toUtf8().data());
  std::string sagittalCornerAnnotation = std::string(tr("Sagittal").toUtf8().data());
  std::string coronalCornerAnnotation = std::string(tr("Coronal").toUtf8().data());
  std::string cornerAnnotation3D = std::string(tr("3D").toUtf8().data());

  SetDecorationProperties(axialCornerAnnotation, GetDecorationColor(0), 0);
  SetDecorationProperties(sagittalCornerAnnotation, GetDecorationColor(1), 1);
  SetDecorationProperties(coronalCornerAnnotation, GetDecorationColor(2), 2);
  SetDecorationProperties(cornerAnnotation3D, GetDecorationColor(3), 3);

  for(int i = 0; i < 4; i++) {
    cornerText[i] = vtkCornerAnnotation::New();
    textProp[i] = vtkTextProperty::New();
    ren[i] = vtkRenderer::New();
    ren[i]->AddActor(cornerText[i]);
    ren[i]->InteractiveOff();
  }
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow2()->GetRenderWindow())->InsertForegroundRenderer(ren[0], true);
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow3()->GetRenderWindow())->InsertForegroundRenderer(ren[1], true);
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow1()->GetRenderWindow())->InsertForegroundRenderer(ren[2], true);
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow4()->GetRenderWindow())->InsertForegroundRenderer(ren[3], true);

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
  //mitkWidget4->GetSliceNavigationController()
  //  ->ConnectGeometryTimeEvent(m_TimeNavigationController, false);

  m_MouseModeSwitcher = mitk::MouseModeSwitcher::New(mitkWidget1->GetRenderer()->GetName());
  //m_MouseModeSwitcher->AddRenderer(mitkWidget1->GetRenderer()->GetName());
  m_MouseModeSwitcher->AddRenderer(mitkWidget2->GetRenderer()->GetName());
  m_MouseModeSwitcher->AddRenderer(mitkWidget3->GetRenderer()->GetName());
  m_MouseModeSwitcher->AddRenderer(mitkWidget4->GetRenderer()->GetName());

  mitkWidget1->GetSliceNavigationController()->crosshairPositionEvent.AddListener(mitk::MessageDelegate<QmitkStdMultiWidget>(this, &QmitkStdMultiWidget::HandleCrosshairPositionEvent));
  mitkWidget2->GetSliceNavigationController()->crosshairPositionEvent.AddListener(mitk::MessageDelegate<QmitkStdMultiWidget>(this, &QmitkStdMultiWidget::HandleCrosshairPositionEvent));
  mitkWidget3->GetSliceNavigationController()->crosshairPositionEvent.AddListener(mitk::MessageDelegate<QmitkStdMultiWidget>(this, &QmitkStdMultiWidget::HandleCrosshairPositionEvent));
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
  return std::string(m_CornerAnnotations[widgetNumber]->GetText(0));
}

QmitkStdMultiWidget::~QmitkStdMultiWidget()
{
  DisablePositionTracking();
  //DisableNavigationControllerEventListening();

  m_TimeNavigationController->Disconnect(mitkWidget1->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget2->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget3->GetSliceNavigationController());
  m_TimeNavigationController->Disconnect(mitkWidget4->GetSliceNavigationController());

  mitk::VtkLayerController::GetInstance(this->GetRenderWindow2()->GetRenderWindow())->RemoveRenderer( ren[0] );
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow3()->GetRenderWindow())->RemoveRenderer( ren[1] );
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow1()->GetRenderWindow())->RemoveRenderer( ren[2] );
  mitk::VtkLayerController::GetInstance(this->GetRenderWindow4()->GetRenderWindow())->RemoveRenderer( ren[3] );

  for(int i = 0; i < 4; i++) {
    cornerText[i]->Delete();
    textProp[i]->Delete();
    ren[i]->Delete();
  }
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

void QmitkStdMultiWidget::SetDecorationProperties( std::string text, mitk::Color color, int widgetNumber)
{
  if( widgetNumber > 3)
  {
    MITK_ERROR << "Unknown render window for annotation.";
    return;
  }

  vtkRenderer* renderer = this->GetRenderWindow(widgetNumber)->GetRenderer()->GetVtkRenderer();
  if(!renderer)
  {
    return;
  }

  std::string fontPath = Utilities::preferredPath(Utilities::absPath(std::string("Fonts\\DejaVuSans.ttf")));

  vtkSmartPointer<vtkCornerAnnotation> annotation = m_CornerAnnotations[widgetNumber];
  annotation->SetText(0, text.c_str());
  annotation->SetMaximumFontSize(12);
  annotation->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
  annotation->GetTextProperty()->SetFontFile(fontPath.c_str());
  annotation->GetTextProperty()->SetColor( color[0],color[1],color[2] );

  if(!renderer->HasViewProp(annotation))
  {
    renderer->AddViewProp(annotation);
  }

  vtkSmartPointer<vtkMitkRectangleProp> frame = m_RectangleProps[widgetNumber];
  frame->SetColor(color[0],color[1],color[2]);

  // Take render for visual props
  vtkRenderer* frameRenderer = nullptr;
  auto renderers = this->GetRenderWindow(widgetNumber)->GetVtkRenderWindow()->GetRenderers();
  renderers->InitTraversal();
  for (vtkRenderer* r = renderers->GetNextItem(); r != nullptr; r = renderers->GetNextItem()) {
    if (r->GetLayer() == 4) {
      frameRenderer = r;
      break;
    }
  }

  if (frameRenderer == nullptr) {
    frameRenderer = vtkRenderer::New();
    frameRenderer->InteractiveOff();
    mitk::VtkLayerController::GetInstance(this->GetRenderWindow(widgetNumber)->GetVtkRenderWindow())->InsertForegroundRenderer(frameRenderer, true);
  }

  if (!frameRenderer->HasViewProp(frame)) {
    frameRenderer->AddViewProp(frame);
  }
}

void QmitkStdMultiWidget::SetCornerAnnotationVisibility(bool visibility)
{
  for(int i = 0 ; i<4 ; ++i)
  {
    m_CornerAnnotations[i]->SetVisibility(visibility);
  }
}

bool QmitkStdMultiWidget::IsCornerAnnotationVisible(void) const
{
  return m_CornerAnnotations[0]->GetVisibility() > 0;
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

void QmitkStdMultiWidget::changeLayoutToColumnWidget1And2()
{
  SMW_INFO << "changing layout to Widget1 and 2 in one Column..." << std::endl;

  //Hide all Menu Widgets
  this->HideAllWidgetToolbars();

  delete QmitkStdMultiWidgetLayout;

  //create Main Layout
  QmitkStdMultiWidgetLayout = new QHBoxLayout(this);

  //create main splitter
  m_MainSplit = new QSplitter(this);
  QmitkStdMultiWidgetLayout->addWidget(m_MainSplit);

  //create m_LayoutSplit  and add to the mainSplit
  m_LayoutSplit = new QSplitter(m_MainSplit);
  m_MainSplit->addWidget(m_LayoutSplit);

  //add LevelWindow Widget to mainSplitter
  m_MainSplit->addWidget(levelWindowWidget);

  //add Widgets to splitter
  m_LayoutSplit->addWidget(mitkWidget1Container);
  m_LayoutSplit->addWidget(mitkWidget2Container);

  //set SplitterSize
  QList<int> splitterSize;
  splitterSize.push_back(1000);
  splitterSize.push_back(1000);
  m_LayoutSplit->setSizes(splitterSize);

  //show mainSplitt and add to Layout
  m_MainSplit->show();

  //show/hide Widgets
  if (mitkWidget1->isHidden()) mitkWidget1->show();
  if (mitkWidget2->isHidden()) mitkWidget2->show();
  mitkWidget3->hide();
  mitkWidget4->hide();

  m_Layout = LAYOUT_COLUMN_WIDGET_1_AND_2;

  //update Layout Design List
  mitkWidget1->LayoutDesignListChanged(LAYOUT_COLUMN_WIDGET_1_AND_2);
  mitkWidget2->LayoutDesignListChanged(LAYOUT_COLUMN_WIDGET_1_AND_2);
  mitkWidget3->LayoutDesignListChanged(LAYOUT_COLUMN_WIDGET_1_AND_2);
  mitkWidget4->LayoutDesignListChanged(LAYOUT_COLUMN_WIDGET_1_AND_2);

  //update Alle Widgets
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

  mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetCameraController()->Fit();

  int w = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::InitPositionTracking()
{
// TODO POSITIONTRACKER
}

void QmitkStdMultiWidget::AddDisplayPlaneSubTree(const std::string& parentWidget)
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
  m_PlaneNode1->SetProperty("parentWidget", mitk::StringProperty::New(parentWidget));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 2
  mitk::BaseRenderer* renderer2 = mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow());
  m_PlaneNode2 = renderer2->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode2->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("name", mitk::StringProperty::New(std::string(renderer2->GetName()) + ".plane"));
  m_PlaneNode2->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode2->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("parentWidget", mitk::StringProperty::New(parentWidget));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode2->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 3
  mitk::BaseRenderer* renderer3 = mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow());
  m_PlaneNode3 = renderer3->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("name", mitk::StringProperty::New(std::string(renderer3->GetName()) + ".plane"));
  m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("parentWidget", mitk::StringProperty::New(parentWidget));
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode3->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  m_ParentNodeForGeometryPlanes = mitk::DataNode::New();
  m_ParentNodeForGeometryPlanes->SetProperty("name", mitk::StringProperty::New("Widgets"));
  m_ParentNodeForGeometryPlanes->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_ParentNodeForGeometryPlanes->SetProperty("parentWidget", mitk::StringProperty::New(parentWidget));
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
}

void QmitkStdMultiWidget::moveEvent( QMoveEvent* e )
{
  QWidget::moveEvent( e );

  // it is necessary to readjust the position of the overlays as the StdMultiWidget has moved
  // unfortunately it's not done by QmitkRenderWindow::moveEvent -> must be done here
  emit Moved();
}

void QmitkStdMultiWidget::OnWindowResized()
{
  m_ResizeTimer.start(100);
}

void QmitkStdMultiWidget::OnResizeStoped()
{
  UpdateAnnotationFonts();
  for (int i = 0; i < 4; i++) {
    m_CornerAnnotations[i]->Modified();
  }
  RequestUpdate();
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

const mitk::Point3D QmitkStdMultiWidget::GetCrossPosition() const
{
  const mitk::PlaneGeometry *plane1 =
    mitkWidget1->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane2 =
    mitkWidget2->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane3 =
    mitkWidget3->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ((plane1 != NULL) && (plane2 != NULL)
    && (plane1->IntersectionLine(plane2, line)))
  {
    mitk::Point3D point;
    if ((plane3 != NULL)
      && (plane3->IntersectionPoint(line, point)))
    {
      return point;
    }
  }
  // TODO BUG POSITIONTRACKER;
  mitk::Point3D p;
  return p;
  //return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}

void QmitkStdMultiWidget::EnablePositionTracking()
{

}

void QmitkStdMultiWidget::DisablePositionTracking()
{

}

void QmitkStdMultiWidget::EnsureDisplayContainsPoint(
  mitk::BaseRenderer* renderer, const mitk::Point3D& p)
{
  mitk::Point2D pointOnDisplay;
  renderer->WorldToDisplay(p,pointOnDisplay);

  if(pointOnDisplay[0] < renderer->GetVtkRenderer()->GetOrigin()[0]
     || pointOnDisplay[1] < renderer->GetVtkRenderer()->GetOrigin()[1]
     || pointOnDisplay[0] > renderer->GetVtkRenderer()->GetOrigin()[0]+renderer->GetViewportSize()[0]
     || pointOnDisplay[1] > renderer->GetVtkRenderer()->GetOrigin()[1]+renderer->GetViewportSize()[1])
  {
    mitk::Point2D pointOnPlane;
    renderer->GetCurrentWorldPlaneGeometry()->Map(p,pointOnPlane);
    renderer->GetCameraController()->MoveCameraToPoint(pointOnPlane);
  }
}

void QmitkStdMultiWidget::MoveCrossToPosition(const mitk::Point3D& newPosition)
{
  mitkWidget1->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
  mitkWidget2->GetSliceNavigationController()->SelectSliceByPoint(newPosition);
  mitkWidget3->GetSliceNavigationController()->SelectSliceByPoint(newPosition);

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

        bool isBinary = false;
        nodes->at(x)->GetBoolProperty("binary", isBinary);
        if (isBinary) continue;

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

void QmitkStdMultiWidget::setCornerAnnotation(int corner, int i, const char* text)
{
  std::string fontPath = Utilities::preferredPath(Utilities::absPath(std::string("Fonts\\DejaVuSans.ttf")));

  // empty or NULL string breaks renderer
  // and white square appears
  if ((text == NULL) || (text[0] == 0)) {
    text = " ";
  }

  cornerText[i]->SetText(corner, text);
  cornerText[i]->SetMaximumFontSize(15);
  textProp[i]->SetFontFamily(VTK_FONT_FILE);
  textProp[i]->SetFontFile(fontPath.c_str());
  textProp[i]->SetColor(1.0, 1.0, 7.0);
  cornerText[i]->SetTextProperty(textProp[i]);
}

void QmitkStdMultiWidget::setCornerAnnotationMaxText(int corner, int i, const char* text)
{
  cornerText[i]->SetMaximumLengthText(corner, text);
}

void QmitkStdMultiWidget::setDisplayMetaInfo(bool metainfo)
{
  m_displayMetaInfo = metainfo;
  UpdateAnnotationFonts();
  m_ImageMTime = -1;
}

void QmitkStdMultiWidget::setDisplayMetaInfoEx(bool metainfo)
{
  m_displayMetaInfoEx = metainfo;
  UpdateAnnotationFonts();
  m_ImageMTime = -1;
}

void QmitkStdMultiWidget::setDisplayPatientInfo(bool patientinfo)
{
  m_displayPatientInfo = patientinfo;
  UpdateAnnotationFonts();
  m_ImageMTime = -1;
}

void QmitkStdMultiWidget::setDisplayPatientInfoEx(bool patientinfo)
{
  m_displayPatientInfoEx = patientinfo;
  UpdateAnnotationFonts();
  m_ImageMTime = -1;
}

void QmitkStdMultiWidget::setDisplayPositionInfo(bool positioninfo)
{
  m_displayPositionInfo = positioninfo;
  UpdateAnnotationFonts();
  m_ImageMTime = -1;
}

void QmitkStdMultiWidget::setSelectionMode(bool selection)
{
  m_MouseModeSwitcher->SetSelectionMode(selection);
  mitk::DataNodePickingEventObserver::SetEnabled(selection);
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
  node = this->GetTopLayerNode(nodes);
  if (node.IsNotNull()) {
    image = dynamic_cast<mitk::Image*>(node->GetData());
  }

  mitk::Point3D crosshairPos = this->GetCrossPosition();
  std::string statusText;
  std::stringstream stream;
  itk::Index<3> p;
  mitk::BaseRenderer* baseRenderer = this->mitkWidget1->GetSliceNavigationController()->GetRenderer();
  unsigned int timestep = baseRenderer->GetTimeStep();
  auto timeSteps = image.IsNotNull() ? image->GetTimeSteps() : 0;
  unsigned int component = baseRenderer->GetComponent();
  unsigned int componentMax = image.IsNotNull() ? image->GetPixelType().GetNumberOfComponents() : 0;

  if(image.IsNotNull() && (timeSteps > timestep ))
  {
    image->GetGeometry()->WorldToIndex(crosshairPos, p);
    stream.precision(2);
    mitk::BoundingBox::BoundsArrayType bounds = image->GetGeometry()->GetBounds();
    std::stringstream _infoStringStream[3];
    std::string cornerimgtext[3];

    imageProperties = image->GetPropertyList();
    std::string seriesNumber;
    imageProperties->GetStringProperty("dicom.series.SeriesNumber", seriesNumber);

    auto secondaryProp = dynamic_cast<mitk::IntProperty*>(static_cast<mitk::BaseProperty*>(image->GetProperty("autoplan.secondaryAxisIndex")));
    auto tertiaryProp = dynamic_cast<mitk::IntProperty*>(static_cast<mitk::BaseProperty*>(image->GetProperty("autoplan.tertiaryAxisIndex")));
    auto mainProp = dynamic_cast<mitk::IntProperty*>(static_cast<mitk::BaseProperty*>(image->GetProperty("autoplan.mainAxisIndex")));

    int axisIndices[3] = {
      secondaryProp ? secondaryProp->GetValue() : 0,
      tertiaryProp ? tertiaryProp->GetValue() : 1,
      mainProp ? mainProp->GetValue() : 2
    };

    for (int i = 0; i < 3; i++) {
      if (m_displayPositionInfo) {
        _infoStringStream[axisIndices[i]] << "Im: " << (p[i] + 1) << "/" << bounds[(i * 2 + 1)];
        if (timeSteps > 1) {
          _infoStringStream[axisIndices[i]] << "\nT: " << (timestep + 1) << "/" << timeSteps;
        }
        if (componentMax > 1) {
          _infoStringStream[axisIndices[i]] << "\nV: " << (component + 1) << "/" << componentMax;
        }
        if (seriesNumber != "") {
          _infoStringStream[axisIndices[i]] << "\nSe: " << seriesNumber;
        }
      } else {
        _infoStringStream[axisIndices[i]].clear();
      }
      cornerimgtext[axisIndices[i]] = _infoStringStream[axisIndices[i]].str();
      setCornerAnnotation(2, axisIndices[i], cornerimgtext[axisIndices[i]].c_str());

      std::string maxPosText = "";
      maxPosText += "Im: " + std::to_string(bounds[(i * 2 + 1)]) + "/" + std::to_string(bounds[(i * 2 + 1)]);
      if (timeSteps > 1) {
        maxPosText += "\nT: " + std::to_string(timeSteps) + "/" + std::to_string(timeSteps);
      }
      if (componentMax > 1) {
        maxPosText += "\nV: " + std::to_string(componentMax) + "/" + std::to_string(componentMax);
      }
      if (seriesNumber != "") {
        maxPosText += "\nSe: " + seriesNumber;
      }

      setCornerAnnotationMaxText(2, axisIndices[i], maxPosText.c_str());
      setViewDirectionAnnontation(image, p[i], axisIndices[i]);
    }

    unsigned long newImageMTime = image->GetMTime();
    std::string newNodeName = node->GetName();

    // check if image is changed or node is renamed
    if ( m_ImageMTime != newImageMTime
      || m_ImageName != newNodeName ) {

      m_ImageMTime = newImageMTime;
      m_ImageName = newNodeName;

      // seriesDescription replaced by newNodeName
      std::string patient, patientId,
        birthday, sex, institution, studyDate, studyTime,
        studiId/*, seriesDescription*/, studyDescription, exInfo,
        magneticFieldStrength, dicomTR, dicomTE, bodyPart, protocolName,
        sliceThickness, xrayTubeCurrent, kvp, imagePosition, windowCenter, windowWidth;

      imageProperties->GetStringProperty("dicom.patient.PatientsName", patient);
      imageProperties->GetStringProperty("dicom.patient.PatientID", patientId);
      imageProperties->GetStringProperty("dicom.patient.PatientsBirthDate", birthday);
      imageProperties->GetStringProperty("dicom.patient.PatientsSex", sex);
      imageProperties->GetStringProperty("dicom.study.InstitutionName", institution);
      imageProperties->GetStringProperty("dicom.acquisition.Date", studyDate);
      imageProperties->GetStringProperty("dicom.acquisition.Time", studyTime);
      if (studyDate.empty() || studyTime.empty()) {
        imageProperties->GetStringProperty("dicom.study.StudyDate", studyDate);
        imageProperties->GetStringProperty("dicom.study.StudyTime", studyTime);
      }
      imageProperties->GetStringProperty("dicom.study.StudyID", studiId);
      imageProperties->GetStringProperty("dicom.study.StudyDescription", studyDescription);
      //imageProperties->GetStringProperty("dicom.series.SeriesDescription", seriesDescription);
      imageProperties->GetStringProperty("dicom.ExInfo", exInfo);
      imageProperties->GetStringProperty("dicom.MagneticFieldStrength", magneticFieldStrength);
      imageProperties->GetStringProperty("dicom.TR", dicomTR);
      imageProperties->GetStringProperty("dicom.TE", dicomTE);
      imageProperties->GetStringProperty("dicom.series.BodyPartExamined", bodyPart);
      imageProperties->GetStringProperty("dicom.series.ProtocolName", protocolName);
      imageProperties->GetStringProperty("dicom.SliceThickness", sliceThickness);
      imageProperties->GetStringProperty("dicom.XrayTubeCurrent", xrayTubeCurrent);
      imageProperties->GetStringProperty("dicom.Kvp", kvp);
      imageProperties->GetStringProperty("dicom.ImagePosition", imagePosition);
      imageProperties->GetStringProperty("dicom.voilut.WindowCenter", windowCenter);
      imageProperties->GetStringProperty("dicom.voilut.WindowWidth", windowWidth);

      auto it = imagePosition.rfind('\\');
      if (std::string::npos != it) {
        imagePosition.erase(0, it + 1);
      }

      std::stringstream infoStringStream[2];

      char yy[5]; yy[4] = 0;
      char mm[3]; mm[2] = 0;
      char dd[3]; dd[2] = 0;
      char hh[3]; hh[2] = 0;
      char mi[3]; mi[2] = 0;
      char ss[3]; ss[2] = 0;

      if (m_displayMetaInfo) {
        if (m_displayPatientInfo) {
          infoStringStream[0]
            << patient.c_str()
            << "\n" << patientId.c_str();
        } else {
          infoStringStream[0] << "***\n***";
        }
        if (birthday != "") {
          sscanf (birthday.c_str(),"%4c%2c%2c",yy,mm,dd);
          infoStringStream[0] << "\n" << dd << "." << mm << "." << yy << " " << sex.c_str();
        }
        else {
          infoStringStream[0] << '\n';
        }
        infoStringStream[0]
          << "\n" << institution.c_str()
          << "\n" << studiId;
        if (m_displayPatientInfoEx) {
          infoStringStream[0]
            << "\n" << studyDescription
            << "\n" << newNodeName
            << "\n" << exInfo
            << "\n" << bodyPart
            << "\n" << protocolName;
        }
      } else {
        infoStringStream[0].clear();
      }

      if (m_displayMetaInfo) {
        if (m_displayMetaInfoEx) {
          if (!magneticFieldStrength.empty()) {
            infoStringStream[1]
              << "FS: " << magneticFieldStrength;
          }
          if (!xrayTubeCurrent.empty() || !kvp.empty()) {
            infoStringStream[1] << '\n';
          }
          if (!xrayTubeCurrent.empty()) {
            infoStringStream[1] << xrayTubeCurrent << "mA";
          }
          if (!kvp.empty()) {
            infoStringStream[1] << ' ' << kvp << " kV";
          }
          if (!dicomTR.empty() || !dicomTE.empty()) {
            infoStringStream[1] << '\n';
            if (!dicomTR.empty()) {
              infoStringStream[1] << "TR: " << dicomTR;
            }
            if (!dicomTE.empty()) {
              infoStringStream[1] << " TE: " << dicomTE;
            }
          }
          if (!windowCenter.empty() && !windowWidth.empty()) {
            infoStringStream[1] << '\n';
            auto pos = windowCenter.find('\\');
            if (std::string::npos != pos) {
              windowCenter.resize(pos);
            }
            pos = windowWidth.find('\\');
            if (std::string::npos != pos) {
              windowWidth.resize(pos);
            }
            if (!windowCenter.empty()) {
              infoStringStream[1] << "WL: " << windowCenter;
            }
            if (!windowWidth.empty()) {
              infoStringStream[1] << " WW: " << windowWidth;
            }
          }
          if (!sliceThickness.empty() || !imagePosition.empty()) {
            infoStringStream[1] << '\n';
          }
          if (!sliceThickness.empty()) {
            infoStringStream[1] << "T: " << sliceThickness << "mm";
          }
          if (!imagePosition.empty()) {
            infoStringStream[1] << " L: " << imagePosition << "mm";
          }
        }
        if (!studyDate.empty() && !studyTime.empty()) {
          sscanf (studyDate.c_str(),"%4c%2c%2c",yy,mm,dd);
          sscanf (studyTime.c_str(),"%2c%2c%2c",hh,mi,ss);
          infoStringStream[1]
            << "\n" << dd << "." << mm << "." << yy
            << " " << hh << ":" << mi << ":" << ss;
        }
      } else {
        infoStringStream[1].clear();
      }

      auto render_annotation = [&] (int j, int corner) {
        const std::string infoString = infoStringStream[j].str();
        for(int i = 0; i < 4; i++) {
          setCornerAnnotation(corner, i, infoString.c_str());
          setCornerAnnotationMaxText(corner, i, infoString.c_str());
        }
      };
      render_annotation(0, 3);
      render_annotation(1, 1);

      this->GetRenderWindow1()->GetRenderer()->RequestUpdate();
      this->GetRenderWindow2()->GetRenderer()->RequestUpdate();
      this->GetRenderWindow3()->GetRenderer()->RequestUpdate();
      this->GetRenderWindow4()->GetRenderer()->RequestUpdate();
    }
  }
}

void QmitkStdMultiWidget::setDisplayPositionText(bool draw)
{
  m_drawTextInStatusBar = draw;
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
    GetRenderWindow(i)->GetRenderer()->GetVtkRenderer()->GradientBackgroundOn();
  }
  m_GradientBackgroundFlag = true;
}

void QmitkStdMultiWidget::DisableGradientBackground()
{
  for(unsigned int i = 0; i < 4; ++i)
  {
    GetRenderWindow(i)->GetRenderer()->GetVtkRenderer()->GradientBackgroundOff();
  }
  m_GradientBackgroundFlag = false;
}

void QmitkStdMultiWidget::EnableDepartmentLogo()
{
  m_LogoRendering->SetVisibility(true);
  RequestUpdate();
}

void QmitkStdMultiWidget::DisableDepartmentLogo()
{
  m_LogoRendering->SetVisibility(false);
  RequestUpdate();
}

bool QmitkStdMultiWidget::IsDepartmentLogoEnabled() const
{
  return m_LogoRendering->IsVisible(mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));
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
  emit WidgetPlanesRotationLinked( link );
}

void QmitkStdMultiWidget::SetWidgetPlaneMode( int userMode )
{
  MITK_DEBUG << "Changing crosshair mode to " << userMode;

  emit WidgetNotifyNewCrossHairMode( userMode );
    // Convert user interface mode to actual mode
  {
    switch(userMode)
    {
      case 0:
        m_MouseModeSwitcher->SetInteractionScheme(mitk::MouseModeSwitcher::InteractionScheme::MITK);
        break;
      case 1:
        m_MouseModeSwitcher->SetInteractionScheme( mitk::MouseModeSwitcher::InteractionScheme::ROTATION);
        break;

      case 2:
        m_MouseModeSwitcher->SetInteractionScheme( mitk::MouseModeSwitcher::InteractionScheme::ROTATIONLINKED);
        break;

      case 3:
        m_MouseModeSwitcher->SetInteractionScheme( mitk::MouseModeSwitcher::InteractionScheme::SWIVEL);
        break;
    }
  }
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
  vtkRenderer* renderer = GetRenderWindow(widgetNumber)->GetRenderer()->GetVtkRenderer();
  renderer->SetBackground2(upper[0], upper[1], upper[2]);
  renderer->SetBackground(lower[0], lower[1], lower[2]);
  m_GradientBackgroundFlag = true;
}

void QmitkStdMultiWidget::SetGradientBackgroundColors( const mitk::Color & upper, const mitk::Color & lower )
{
  for(unsigned int i = 0; i < 4; ++i)
  {
    vtkRenderer* renderer = GetRenderWindow(i)->GetRenderer()->GetVtkRenderer();
    renderer->SetBackground2(upper[0], upper[1], upper[2]);
    renderer->SetBackground(lower[0], lower[1], lower[2]);
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
  case LAYOUT_COLUMN_WIDGET_1_AND_2:
    {
      this->changeLayoutToColumnWidget1And2();
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

  ResetTransformation(this->GetRenderWindow1()->GetRenderer());
  ResetTransformation(this->GetRenderWindow2()->GetRenderer());
  ResetTransformation(this->GetRenderWindow3()->GetRenderer());
}

void QmitkStdMultiWidget::EnableColoredRectangles()
{
  m_RectangleProps[0]->SetVisibility(1);
  m_RectangleProps[1]->SetVisibility(1);
  m_RectangleProps[2]->SetVisibility(1);
  m_RectangleProps[3]->SetVisibility(1);
}

void QmitkStdMultiWidget::DisableColoredRectangles()
{
  m_RectangleProps[0]->SetVisibility(0);
  m_RectangleProps[1]->SetVisibility(0);
  m_RectangleProps[2]->SetVisibility(0);
  m_RectangleProps[3]->SetVisibility(0);
}

bool QmitkStdMultiWidget::IsColoredRectanglesEnabled() const
{
  return m_RectangleProps[0]->GetVisibility()>0;
}

mitk::MouseModeSwitcher* QmitkStdMultiWidget::GetMouseModeSwitcher()
{
  return m_MouseModeSwitcher;
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

void QmitkStdMultiWidget::setViewDirectionAnnontation(mitk::Image* image, int slice, int i)
{
  auto mainAxis = 2;
  auto secondAxis = 0;
  auto tertiaryAxis = 1;

  auto mainSign = true;
  auto secondSign = true;
  auto tertiarySign = true;

  if (!m_displayMetaInfo) {
    mainAxis = secondAxis = -1;
  }
  else {
    switch (i) {
    case 0:
      std::swap(mainAxis, tertiaryAxis);
      std::swap(mainSign, tertiarySign);
      secondAxis = mainAxis;
      secondSign = mainSign;
      break;
    case 1:
      std::swap(mainAxis, tertiaryAxis);
      std::swap(mainSign, tertiarySign);
      break;
    }
  }

  switch (mainAxis) {
  case 0:
    setCornerAnnotation(vtkCornerAnnotation::UpperEdge, i, mainSign ? "I" : "S");
    setCornerAnnotation(vtkCornerAnnotation::LowerEdge, i, mainSign ? "S" : "I");
    break;
  case 1:
    setCornerAnnotation(vtkCornerAnnotation::UpperEdge, i, mainSign ? "S" : "I");
    setCornerAnnotation(vtkCornerAnnotation::LowerEdge, i, mainSign ? "I" : "S");
    break;
  case 2:
    setCornerAnnotation(vtkCornerAnnotation::UpperEdge, i, mainSign ? "A" : "P");
    setCornerAnnotation(vtkCornerAnnotation::LowerEdge, i, mainSign ? "P" : "A");
    break;
  default:
    setCornerAnnotation(vtkCornerAnnotation::UpperEdge, i, " ");
    setCornerAnnotation(vtkCornerAnnotation::LowerEdge, i, " ");
    break;
  }

  switch (secondAxis) {
  case 0:
    setCornerAnnotation(vtkCornerAnnotation::RightEdge, i, secondSign ? "L" : "R");
    setCornerAnnotation(vtkCornerAnnotation::LeftEdge, i, secondSign ? "R" : "L");
    break;
  case 1:
    setCornerAnnotation(vtkCornerAnnotation::RightEdge, i, secondSign ? "P" : "A");
    setCornerAnnotation(vtkCornerAnnotation::LeftEdge, i, secondSign ? "A" : "P");
    break;
  case 2:
    setCornerAnnotation(vtkCornerAnnotation::RightEdge, i, secondSign ? "L" : "R");
    setCornerAnnotation(vtkCornerAnnotation::LeftEdge, i, secondSign ? "R" : "L");
    break;
  default:
    setCornerAnnotation(vtkCornerAnnotation::RightEdge, i, " ");
    setCornerAnnotation(vtkCornerAnnotation::LeftEdge, i, " ");
    break;
  }
}

std::vector<QmitkStdMultiWidget::FunctionSet> QmitkStdMultiWidget::getFuncSetDisplayAnnotation()
{
  std::vector<FunctionSet> functionsSetAnnotation;

  functionsSetAnnotation.push_back(&QmitkStdMultiWidget::setDisplayMetaInfo);
  functionsSetAnnotation.push_back(&QmitkStdMultiWidget::setDisplayMetaInfoEx);
  functionsSetAnnotation.push_back(&QmitkStdMultiWidget::setDisplayPatientInfo);
  functionsSetAnnotation.push_back(&QmitkStdMultiWidget::setDisplayPatientInfoEx);
  functionsSetAnnotation.push_back(&QmitkStdMultiWidget::setDisplayPositionInfo);

  return functionsSetAnnotation;
}

void QmitkStdMultiWidget::setAnnotationVisibility(std::vector<bool>& visibility)
{
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->m_DataStorage->GetSubset(isImageData).GetPointer();

  mitk::DataNode::Pointer node;
  mitk::Image::Pointer image;
  node = this->GetTopLayerNode(nodes);
  if (node.IsNotNull()) {
    image = dynamic_cast<mitk::Image*>(node->GetData());
  }

  std::string seriesNumber;
  bool imageIsPictureOrNull = false;
  if (image.IsNotNull()) {
    image->GetPropertyList()->GetStringProperty("dicom.series.SeriesNumber", seriesNumber);
    imageIsPictureOrNull = image->GetDimension(2) < 2 && seriesNumber == "";
  } else {
    imageIsPictureOrNull = true;
  }

  auto funcSetList = getFuncSetDisplayAnnotation();
  auto size = funcSetList.size();
  for (size_t i = 0; i < size; i++) {
    (this->*funcSetList[i])(
      (imageIsPictureOrNull || visibility.empty()) ? false : visibility[i]);
  }
  HandleCrosshairPositionEvent();
}

void QmitkStdMultiWidget::setMouseMode(mitk::MouseModeSwitcher::MouseMode mode, const Qt::MouseButton& button)
{
  m_MouseModeSwitcher->SelectMouseMode(mode, button);
}

void QmitkStdMultiWidget::ResetTransformation(mitk::VtkPropRenderer* renderer)
{
  if (renderer) {
    if (renderer->GetCameraRotationController()) {
      renderer->GetCameraRotationController()->ResetTransformationAngles();
    }
  }
}

void QmitkStdMultiWidget::UpdateFullSreenMode()
{
  switch (m_Layout) {
  case LAYOUT_WIDGET1:
    this->GetRenderWindow1()->FullScreenMode(true);
    break;
  case LAYOUT_WIDGET2:
    this->GetRenderWindow2()->FullScreenMode(true);
    break;
  case LAYOUT_WIDGET3:
    this->GetRenderWindow3()->FullScreenMode(true);
    break;
  case LAYOUT_BIG_3D:
    this->GetRenderWindow4()->FullScreenMode(true);
    break;
  }

  m_RenderingManager->RequestUpdateAll();
}
