
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

#include "mitkDataNodeFactory.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"

#include <itksys/SystemTools.hxx>
#include <QApplication>
#include <QHBoxLayout>

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

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  // Create a DataStorage
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();


  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for(i=1; i<argc; ++i)
  {
    // For testing
    if(strcmp(argv[i], "-testing")==0) continue;

    // Create a DataNodeFactory to read a data format supported
    // by the DataNodeFactory (many image formats, surface formats, etc.)
    mitk::DataNodeFactory::Pointer nodeReader=mitk::DataNodeFactory::New();
    const char * filename = argv[i];
    try
    {
      nodeReader->SetFileName(filename);
      nodeReader->Update();
      //*********************************************************************
      //Part III: Put the data into the datastorage
      //*********************************************************************

      // Since the DataNodeFactory directly creates a node,
      // use the datastorage to add the read node
      mitk::DataNode::Pointer node = nodeReader->GetOutput();
      ds->Add(node);
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
  }

  //*************************************************************************
  // Part IV: Create windows and pass the tree to it
  //*************************************************************************

  // Create toplevel widget with horizontal layout
  QWidget toplevelWidget;
  QHBoxLayout layout;
  layout.setSpacing(2);
  layout.setMargin(0);
  toplevelWidget.setLayout(&layout);

  //*************************************************************************
  // Part IVa: 3D view
  //*************************************************************************

  // Create a renderwindow
  QmitkRenderWindow renderWindow(&toplevelWidget);
  layout.addWidget(&renderWindow);

  // Tell the renderwindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(ds);

  // Use it as a 3D view
  renderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  // *******************************************************
  // ****************** START OF NEW PART ******************
  // *******************************************************

  //*************************************************************************
  // Part IVb: 2D view for slicing transversally
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view2(&toplevelWidget);
  layout.addWidget(&view2);
  view2.SetLevelWindowEnabled(true);
  // Tell the QmitkSliceWidget which (part of) the tree to render.
  // By default, it slices the data transversally
  view2.SetDataStorage(ds);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetAll();
  view2.SetData(rs->Begin(),mitk::SliceNavigationController::Transversal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the datastorage!
  ds->Add(view2.GetRenderer()->GetCurrentWorldGeometry2DNode());

  //*************************************************************************
  // Part IVc: 2D view for slicing sagitally
  //*************************************************************************

  // Create QmitkSliceWidget, which is based on the class
  // QmitkRenderWindow, but additionally provides sliders
  QmitkSliceWidget view3(&toplevelWidget);
  layout.addWidget(&view3);
  view3.SetDataStorage(ds);
  // Tell the QmitkSliceWidget which (part of) the datastorage to render
  // and to slice sagitally
  view3.SetData(rs->Begin(), mitk::SliceNavigationController::Sagittal);
  // We want to see the position of the slice in 2D and the
  // slice itself in 3D: add it to the datastorage!
  ds->Add(view3.GetRenderer()->GetCurrentWorldGeometry2DNode());

  // *******************************************************
  // ******************* END OF NEW PART *******************
  // *******************************************************

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
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
