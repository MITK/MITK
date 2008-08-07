
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include "mitkDataTreeNodeFactory.h"

#include <itksys/SystemTools.hxx>
#include <QApplication>

//##Documentation
//## @brief Load one or more data sets (many image, surface
//## and other formats) and display it in a 2D view
//##
//## Only very slightly different to Step1: Use DataTreeNodeFactory
//## instead of PicFileReader, and read more than one data set.
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", 
      itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  // Create a tree
  // For now we need a DataTree to initialize a DataStorage later on. In the
  // future, the DataStorage will be independent of the DataTree
  mitk::DataTree::Pointer tree=mitk::DataTree::New();

  // Create a data storage object. We will use it as a singleton
  mitk::DataStorage* storage = mitk::DataStorage::CreateInstance(tree);

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
      // Part III: Put the data into the tree
      //*********************************************************************

      // Since the DataTreeNodeFactory directly creates a node,
      // use the datastorage to add the read node
      storage->Add(nodeReader->GetOutput());
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
  }

  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************
  
  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetData(storage);

  // Initialize the RenderWindow
  mitk::RenderingManager::GetInstance()->InitializeViews( storage );

  // Select a slice
  renderWindow.GetSliceNavigationController()->GetSlice()->SetPos( 0 );


  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  renderWindow.show();
  renderWindow.resize( 256, 256 );

  // for testing
  #include "QtTesting.h"
  if(strcmp(argv[argc-1], "-testing")!=0)
    return qtapplication.exec();
  else
    return QtTesting();

  // Release all resources used by the data storage and
  // the datatree
  mitk::DataStorage::ShutdownSingleton();
}

/**
\example Step2.cpp
*/
