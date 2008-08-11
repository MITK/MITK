
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkPicFileReader.h>

#include <itksys/SystemTools.hxx>
#include <QApplication>

//##Documentation
//## @brief Load image (pic format) and display it in a 2D view
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename] \n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
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
  mitk::DataStorage::CreateInstance(tree);


  //*************************************************************************
  // Part II: Create some data by reading a file
  //*************************************************************************

  // Create a PicFileReader to read a .pic-file
  mitk::PicFileReader::Pointer reader=mitk::PicFileReader::New();
  const char * filename = argv[1];
  try
  {
    reader->SetFileName(filename);
    reader->Update();
  }
  catch(...)
  {
    fprintf( stderr, "Could not open file %s \n\n", filename );
    exit(2);
  }

  //*************************************************************************
  // Part III: Put the data into the datastorage
  //*************************************************************************

  // Create a node and add the Image (which is read from the file) to it
  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  node->SetData(reader->GetOutput());

  // Add the node to the DataStorage
  mitk::DataStorage::GetInstance()->Add(node);


  //*************************************************************************
  // Part IV: Create window and pass the datastorage to it
  //*************************************************************************

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetData(mitk::DataStorage::GetInstance());

  // Initialize the RenderWindow
  mitk::RenderingManager::GetInstance()->InitializeViews( mitk::DataStorage::GetInstance() );

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
\example Step1.cpp
*/

