#include "Step6.h"

#include <QmitkRenderWindow.h>
#include <ComfortGLWidget.h>

#include <mitkDataTreeNodeFactory.h>
#include <mitkProperties.h>
#include <mitkMultiplexUpdateController.h>

#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>

#include <itkConfidenceConnectedImageFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkDataTreeHelper.h>

#include <itksys/SystemTools.hxx>
#include <qapplication.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>

//##Documentation
//## @brief Start region-grower at interactively added points
Step6::Step6( int argc, char* argv[], QWidget *parent, const char *name )
  : QMainWindow ( parent, name ), m_FirstImage(NULL)
{
  // create the tree: this is now a member
  m_Tree=mitk::DataTree::New();

  // load data as in the previous steps; a reference to the first loaded 
  // image is kept in the member m_FirstImage and used as input for the
  // region growing
  Load(argc, argv);

  // setup the widgets as in the previous steps, but with an additional
  // QVBox for a button to start the segmentation
  SetupWidgets();

  // create button to start the segmentation and connect its clicked()
  // signal to method StartRegionGrowing
  QPushButton* startButton = new QPushButton("start region growing", m_TopLevelWidget, "start button");
  connect(startButton, SIGNAL(clicked()), this, SLOT(StartRegionGrowing()));
  if(m_FirstImage.IsNull()) startButton->setEnabled(false);

  // as in Step5, create Point>Set (now as a member m_Seeds) and 
  // associate a interactor to it
  mitk::DataTreePreOrderIterator it(m_Tree);
  m_Seeds = mitk::PointSet::New();
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
  pointSetNode->SetData(m_Seeds);
  pointSetNode->SetProperty("layer", new mitk::IntProperty(2));
  it.Add(pointSetNode);
  mitk::GlobalInteraction::GetGlobalInteraction()->AddInteractor(
    new mitk::PointSetInteractor("pointsetinteractor", pointSetNode));
}

template < typename TPixel, unsigned int VImageDimension > 
void Step6::RegionGrowing( itk::Image<typename TPixel, VImageDimension>* itkImage, mitk::Geometry3D* geometry)
{
  typedef itk::Image<typename TPixel, VImageDimension> ImageType;

  // create itk::ConfidenceConnectedImageFilter and set itkImage as input
  typedef itk::ConfidenceConnectedImageFilter<ImageType, ImageType> ConfidenceConnectedFilterType;
  typedef ConfidenceConnectedFilterType::IndexType IndexType;
  ConfidenceConnectedFilterType::Pointer regGrowFilter = ConfidenceConnectedFilterType::New();
  regGrowFilter->SetInput(itkImage);

  // convert the points in the PointSet m_Seeds (in world-coordinates) to
  // "index" values, i.e. points in pixel coordinates, and add these as seeds
  // to the ConfidenceConnectedImageFilter
  mitk::PointSet::PointsConstIterator pit, pend = m_Seeds->GetPointSet()->GetPoints()->End();
  IndexType seedIndex;
  for(pit=m_Seeds->GetPointSet()->GetPoints()->Begin();pit!=pend;++pit)
  {
    geometry->WorldToIndex(pit.Value(), seedIndex);
    regGrowFilter->AddSeed( seedIndex );
  }

  // add output of ConfidenceConnectedImageFilter to tree
  mitk::DataTreePreOrderIterator it(m_Tree);
  mitk::DataTreeNode::Pointer node = mitk::DataTreeHelper::AddItkImageToDataTree(regGrowFilter->GetOutput(), &it, "segmentation");
  // set some additional properties
  node->SetProperty("binary", new mitk::BoolProperty(true));
  node->SetProperty("color", new mitk::ColorProperty(1.0,0.0,0.0));
  node->SetProperty("volumerendering", new mitk::BoolProperty(true));
  node->SetProperty("layer", new mitk::IntProperty(1));
}

void Step6::StartRegionGrowing()
{
  AccessByItk_1(m_FirstImage, RegionGrowing, m_FirstImage->GetGeometry());
  
  mitk::RenderWindow::UpdateAllInstances();
}

void Step6::Load(int argc, char* argv[])
{
  //Part I: Basic initialization
  // create a tree
  m_Tree=mitk::DataTree::New();
  // create an iterator on the tree
  mitk::DataTreePreOrderIterator it(m_Tree);

  //Part II: Create some data by reading files
  unsigned int i;
  for(i=1; i<argc; ++i)
  {
    // create a DataTreeNodeFactory to read a data format supported
    // by the DataTreeNodeFactory (many image formats, surface formats, etc.)
    mitk::DataTreeNodeFactory::Pointer nodeReader=mitk::DataTreeNodeFactory::New();
    const char * filename = argv[i];
    try
    {
      nodeReader->SetFileName(filename);
      nodeReader->Update();
      //Part III: Put the data into the tree
      // Since the DataTreeNodeFactory directly creates a node,
      // use the iterator to add the read node to the tree
      mitk::DataTreeNode::Pointer node = nodeReader->GetOutput();
      it.Add(node);

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
  //Part IV: initialize interaction mechanism
  // load interaction patterns from XML-file
  if(mitk::GlobalInteraction::StandardInteractionSetup()==false)
  {
    fprintf( stderr, "Could initialize interaction.\n");
    fprintf( stderr, "Maybe StateMachine.xml was not found. Copy it in \n");
    fprintf( stderr, "of executable or set MITKCONF environment.\n\n");
    exit(3);
  }

  // create an iterator on the tree
  mitk::DataTreePreOrderIterator it(m_Tree);

  //Part VI: Create windows and pass the tree to it
  // create toplevel widget with vertical layout
  m_TopLevelWidget = new QVBox(this);
  // create viewParent widget with horizontal layout
  QHBox* viewParent = new QHBox(m_TopLevelWidget);
  // Part VIa: 3D view
  // create a renderwindow
  mitk::QmitkRenderWindow* renderWindow = new mitk::QmitkRenderWindow(viewParent);
  // tell the renderwindow which (part of) the tree to render
  renderWindow->GetRenderer()->SetData(&it);
  // use it as a 3D view
  renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Part VIb: 2D view for slicing transversally
  // create ComfortGLWidget, which is based on the class 
  // QmitkRenderWindow, but additionally provides sliders
  ComfortGLWidget *view2=new ComfortGLWidget(viewParent);
  // tell the ComfortGLWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2->SetData(&it);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view2->GetRenderer()->GetCurrentWorldGeometry2DNode());

  // Part VIc: 2D view for slicing sagitally
  // create ComfortGLWidget, which is based on the class 
  // QmitkRenderWindow, but additionally provides sliders
  ComfortGLWidget *view3=new ComfortGLWidget(viewParent);
  // tell the ComfortGLWidget which (part of) the tree to render
  // and to slice sagitally
  view3->SetData(&it, mitk::SliceNavigationController::Sagittal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view3->GetRenderer()->GetCurrentWorldGeometry2DNode());

  //Part VII: handle updates: To avoid unnecessary updates, we have to
  //define, when to update. The SliceNavigationController of each
  //2D view sends an event, when the slice was changed. Connect this
  //an update-controller to this event, and all renderwindows to
  //the update-controller.
  // create update-controller
  mitk::MultiplexUpdateController::Pointer updateController= new mitk::MultiplexUpdateController;
  // connect SliceNavigationController of each 2D View to the update-controller
  view2->GetSliceController()->ConnectRepaintRequest(updateController.GetPointer());
  view3->GetSliceController()->ConnectRepaintRequest(updateController.GetPointer());

  // tell the update-controller which renderwindows to update when
  // an update/repaint is requested
  updateController->AddRenderWindow(renderWindow);
  updateController->AddRenderWindow(view2->GetRenderWindow());
  updateController->AddRenderWindow(view3->GetRenderWindow());

  setCentralWidget(m_TopLevelWidget);
}

#include <qapplication.h>
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  Step6 mainWindow(argc, argv, NULL, "mainwindow");
  qtapplication.setMainWidget(&mainWindow);
  mainWindow.show();

  return qtapplication.exec();
}
