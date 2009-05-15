#include "Step6.h"

#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

#include "mitkDataTreeNodeFactory.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"

#include "mitkGlobalInteraction.h"
#include "mitkPointSet.h"
#include "mitkPointSetInteractor.h"

#include "mitkImageAccessByItk.h"

#include "mitkRenderingManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

//##Documentation
//## @brief Start region-grower at interactively added points
Step6::Step6( int argc, char* argv[], QWidget *parent )
: QWidget( parent ), m_FirstImage(NULL), m_ResultImage(NULL),
  m_ResultNode(NULL), m_DataStorage(NULL);
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
  this->layout()->addWidget( controlsParent);

  QHBoxLayout* hlayout = new QHBoxLayout( controlsParent );
  hlayout->setSpacing(2);

  QLabel *labelThresholdMin =
    new QLabel( "Lower Threshold:", controlsParent );
  hlayout->addWidget( labelThresholdMin );

  m_LineEditThresholdMin = new QLineEdit( "-1000", controlsParent );
  hlayout->addWidget( m_LineEditThresholdMin );

  QLabel *labelThresholdMax =
    new QLabel( "Upper Threshold:", controlsParent );
  hlayout->addWidget( labelThresholdMax );

  m_LineEditThresholdMax = new QLineEdit( "-400", controlsParent );
  hlayout->addWidget( m_LineEditThresholdMax );

  // create button to start the segmentation and connect its clicked()
  // signal to method StartRegionGrowing
  QPushButton* startButton = new QPushButton( "start region growing", controlsParent );
  hlayout->addWidget( startButton );

  connect(startButton, SIGNAL(clicked()), this, SLOT(StartRegionGrowing()));
  if(m_FirstImage.IsNull()) startButton->setEnabled(false);

  // as in Step5, create PointSet (now as a member m_Seeds) and
  // associate a interactor to it
  m_Seeds = mitk::PointSet::New();
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
  pointSetNode->SetData(m_Seeds);
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(2));
  m_DataStorage->Add(pointSetNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(
    mitk::PointSetInteractor::New("pointsetinteractor", pointSetNode)
  );
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
  for(i=1; i<argc; ++i)
  {
    // For testing
    if(strcmp(argv[i], "-testing")==0) continue;

    // Create a DataTreeNodeFactory to read a data format supported
    // by the DataTreeNodeFactory (many image formats, surface formats, etc.)
    mitk::DataTreeNodeFactory::Pointer nodeReader=mitk::DataTreeNodeFactory::New();
    const char * filename = argv[i];
    try
    {
      nodeReader->SetFileName(filename);
      nodeReader->Update();
      //*********************************************************************
      // Part III: Put the data into the datastorage
      //*********************************************************************

      // Since the DataTreeNodeFactory directly creates a node,
      // use the iterator to add the read node to the tree
      mitk::DataTreeNode::Pointer node = nodeReader->GetOutput();
      m_DataStorage->Add(node);

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
      if((m_FirstImage.IsNull()) && (image.IsNotNull()))
        m_FirstImage = image;
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
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
  // Part Ib: 2D view for slicing transversally
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget *view2 = new QmitkSliceWidget(viewParent);
  hlayout->addWidget(view2);

  // Tell the QmitkSliceWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2->SetDataStorage(m_DataStorage);

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
  view3->SetDataStorage(m_DataStorage, mitk::SliceNavigationController::Sagittal);

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
