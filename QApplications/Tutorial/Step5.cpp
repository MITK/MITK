#include <QmitkRenderWindow.h>
#include <ComfortGLWidget.h>

#include <mitkDataTreeNodeFactory.h>
#include <mitkProperties.h>
#include <mitkMultiplexUpdateController.h>

#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>

#include <itksys/SystemTools.hxx>
#include <qapplication.h>
#include <qhbox.h>

//##Documentation
//## @brief Interactively add points
//##
//## As in Step4, load one or more data sets (many image, 
//## surface and other formats) and create 3 views on the data.
//## Additionally, we want to interactively add points. A node containing 
//## a PointSet as data is added to the data tree and a PointSetInteractor
//## is associated with the node, which handles the interaction. The
//## @em interaction @em pattern is defined in a state-machine, stored in an 
//## external XML file. Thus, we need to load a state-machine 
//## The interaction patterns defines the @em events, 
//## on which the interactor reacts (e.g., which mouse buttons are used to 
//## set a point), the @em transition to the next state (e.g., the initial 
//## may be "empty point set") and associated @a actions (e.g., add a point 
//## at the position where the mouse-click occured).
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  //Part I: Basic initialization
  // create a tree
  mitk::DataTree::Pointer tree=mitk::DataTree::New();
  // create an iterator on the tree
  mitk::DataTreePreOrderIterator it(tree);

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
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
  }

  // *******************************************************
  // ****************** START OF NEW PART ******************
  // *******************************************************

  //Part IV: initialize interaction mechanism
  // load interaction patterns from XML-file
  if(mitk::GlobalInteraction::StandardInteractionSetup()==false)
  {
    fprintf( stderr, "Could initialize interaction.\n");
    fprintf( stderr, "Maybe StateMachine.xml was not found. Copy it in \n");
    fprintf( stderr, "of executable or set MITKCONF environment.\n\n");
    exit(3);
  }

  //Part V: For allowing to interactively add points ...
  // create node and set PointSet as data
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
  pointSetNode->SetData(pointSet);
  // add the node to the tree
  it.Add(pointSetNode);
  // create PointSetInteractor, associate to pointSetNode and add as
  // interactor to GlobalInteraction
  mitk::GlobalInteraction::GetGlobalInteraction()->AddInteractor(
    new mitk::PointSetInteractor("pointsetinteractor", pointSetNode));

  // *******************************************************
  // ******************* END OF NEW PART *******************
  // *******************************************************

  //Part VI: Create windows and pass the tree to it
  // create toplevel widget with horizontal layout
  QHBox toplevelWidget;
  // Part VIa: 3D view
  // create a renderwindow
  mitk::QmitkRenderWindow renderWindow(&toplevelWidget);
  // tell the renderwindow which (part of) the tree to render
  renderWindow.GetRenderer()->SetData(&it);
  // use it as a 3D view
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Part VIb: 2D view for slicing transversally
  // create ComfortGLWidget, which is based on the class 
  // QmitkRenderWindow, but additionally provides sliders
  ComfortGLWidget view2(&toplevelWidget);
  // tell the ComfortGLWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2.SetData(&it);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view2.GetRenderer()->GetCurrentWorldGeometry2DNode());

  // Part VIc: 2D view for slicing sagitally
  // create ComfortGLWidget, which is based on the class 
  // QmitkRenderWindow, but additionally provides sliders
  ComfortGLWidget view3(&toplevelWidget);
  // tell the ComfortGLWidget which (part of) the tree to render
  // and to slice sagitally
  view3.SetData(&it, mitk::SliceNavigationController::Sagittal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view3.GetRenderer()->GetCurrentWorldGeometry2DNode());

  //Part VII: handle updates: To avoid unnecessary updates, we have to
  //define, when to update. The SliceNavigationController of each
  //2D view sends an event, when the slice was changed. Connect this
  //an update-controller to this event, and all renderwindows to
  //the update-controller.
  // create update-controller
  mitk::MultiplexUpdateController::Pointer updateController= new mitk::MultiplexUpdateController;
  // connect SliceNavigationController of each 2D View to the update-controller
  view2.GetSliceController()->ConnectRepaintRequest(updateController.GetPointer());
  view3.GetSliceController()->ConnectRepaintRequest(updateController.GetPointer());

  // tell the update-controller which renderwindows to update when
  // an update/repaint is requested
  updateController->AddRenderWindow(&renderWindow);
  updateController->AddRenderWindow(view2.GetRenderWindow());
  updateController->AddRenderWindow(view3.GetRenderWindow());

  //Part VIII: Qt-specific initialization
  qtapplication.setMainWidget(&toplevelWidget);
  toplevelWidget.show();

  return qtapplication.exec();
}
