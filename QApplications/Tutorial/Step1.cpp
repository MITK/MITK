
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "QmitkRegisterClasses.h"

#include "mitkPicFileReader.h"

#include <itksys/SystemTools.hxx>
#include <qapplication.h>

//##Documentation
//## @brief Load image (pic format) and display it in a 2D view
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename] \n\n", 
      itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  // Create a tree
  mitk::DataTree::Pointer tree=mitk::DataTree::New();

  // Create an iterator on the tree
  mitk::DataTreePreOrderIterator it(tree);

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
  // Part III: Put the data into the tree
  //*************************************************************************

  // Create a node and add the Image (which is read from the file) to it
  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  node->SetData(reader->GetOutput());

  // Use the iterator to add the node to the tree
  it.Add(node);


  //*************************************************************************
  // Part IV: Create window and pass the tree to it
  //*************************************************************************

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;
  
  // Tell the RenderWindow which (part of) the tree to render
  renderWindow.GetRenderer()->SetData(&it);

  // Initialize the RenderWindow
  mitk::RenderingManager::GetInstance()->InitializeViews( &it );

  // Select a slice
  renderWindow.GetSliceNavigationController()->GetSlice()->SetPos( 0 );

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  qtapplication.setMainWidget(&renderWindow);
  renderWindow.show();
  renderWindow.resize( 256, 256 );

  
  // for testing
  #include "QtTesting.h"
  if(strcmp(argv[argc-1], "-testing")!=0)
    return qtapplication.exec();
  else
    return QtTesting();
}

/**
\example Step1.cpp
*/

