#include <QmitkRenderWindow.h>
#include <ComfortGLWidget.h>
#include <vtkRenderer.h>

#include <mitkDataTreeNodeFactory.h>
#include <mitkProperties.h>
#include <mitkMultiplexUpdateController.h>

#include <itksys/SystemTools.hxx>
#include <qapplication.h>
#include <qhbox.h>

//##Documentation
//## @brief Use several views to explore data
//##
//## As in Step2 and Step3, load one or more data sets (many image, 
//## surface and other formats), but create 3 views on the data.
//## The QmitkRenderWindow is used for displaying a 3D view as in Step3,
//## but without volume-rendering.
//## Furthermore, we create two 2D views for slicing through the data. 
//## We use the class ComfortGLWidget, which is based on the class 
//## QmitkRenderWindow, but additionally provides sliders 
//## to slice through the data. We create two instances of
//## ComfortGLWidget, one for transversal and one for sagittal slicing.
//## The two slices are also shown at their correct position in 3D as
//## well as intersection-line, each in the other 2D view.
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

  //Part V: Create windows and pass the tree to it
  // create toplevel widget with horizontal layout
  QHBox toplevelWidget;
  // Part Va: 3D view
  // create a renderwindow
  mitk::QmitkRenderWindow renderWindow(&toplevelWidget);
  // tell the renderwindow which (part of) the tree to render
  renderWindow.GetRenderer()->SetData(&it);
  // use it as a 3D view
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // Part Vb: 2D view for slicing transversally
  // create ComfortGLWidget, which is based on the class 
  // QmitkRenderWindow, but additionally provides sliders
  ComfortGLWidget view2(&toplevelWidget);
  // tell the ComfortGLWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2.SetData(&it);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view2.GetRenderer()->GetCurrentWorldGeometry2DNode());

  // Part Vc: 2D view for slicing sagitally
  // create ComfortGLWidget, which is based on the class 
  // QmitkRenderWindow, but additionally provides sliders
  ComfortGLWidget view3(&toplevelWidget);
  // tell the ComfortGLWidget which (part of) the tree to render
  // and to slice sagitally
  view3.SetData(&it, mitk::SliceNavigationController::Sagittal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view3.GetRenderer()->GetCurrentWorldGeometry2DNode());

  //Part VI: handle updates: To avoid unnecessary updates, we have to
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

  //Part VII: Qt-specific initialization
  qtapplication.setMainWidget(&toplevelWidget);
  toplevelWidget.show();

  return qtapplication.exec();
}
