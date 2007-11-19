#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

#include "mitkDataTreeNodeFactory.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkDataStorage.h"

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
//## We use the class QmitkSliceWidget, which is based on the class
//## QmitkRenderWindow, but additionally provides sliders
//## to slice through the data. We create two instances of
//## QmitkSliceWidget, one for transversal and one for sagittal slicing.
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
  // create DataStorageInstance
  mitk::DataStorage::CreateInstance(tree);

  //Part II: Create some data by reading files
  int i;
  for(i=1; i<argc; ++i)
  {
    // for testing
    if(strcmp(argv[i], "-testing")==0) continue;

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

  //Part IV: Create windows and pass the tree to it
  // create toplevel widget with horizontal layout
  QHBox toplevelWidget;
  toplevelWidget.setSpacing(2);
  // Part IVa: 3D view
  // create a renderwindow
  QmitkRenderWindow renderWindow(&toplevelWidget);
  // tell the renderwindow which (part of) the tree to render
  renderWindow.GetRenderer()->SetData(&it);
  // use it as a 3D view
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // *******************************************************
  // ****************** START OF NEW PART ******************
  // *******************************************************

  // Part IVb: 2D view for slicing transversally
  // create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view2(&toplevelWidget);
  // tell the QmitkSliceWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2.SetData(&it);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view2.GetRenderer()->GetCurrentWorldGeometry2DNode());

  // Part IVc: 2D view for slicing sagitally
  // create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view3(&toplevelWidget);
  // tell the QmitkSliceWidget which (part of) the tree to render
  // and to slice sagitally
  view3.SetData(&it, mitk::SliceNavigationController::Sagittal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the tree!
  it.Add(view3.GetRenderer()->GetCurrentWorldGeometry2DNode());

  // *******************************************************
  // ******************* END OF NEW PART *******************
  // *******************************************************

  //Part V: Qt-specific initialization
  qtapplication.setMainWidget(&toplevelWidget);
  toplevelWidget.show();

  // for testing
  #include "QtTesting.h"
  if(strcmp(argv[argc-1], "-testing")!=0)
    return qtapplication.exec();
  else
    return QtTesting();
}

/**
\example Step4.cpp
*/
