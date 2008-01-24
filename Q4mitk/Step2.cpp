#include "QmitkRenderWindow.h"

#include <mitkDataTreeNodeFactory.h>

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
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  //Part I: Basic initialization
  // create a tree
  mitk::DataTree::Pointer tree=mitk::DataTree::New();
  // create an iterator on the tree
  mitk::DataTreePreOrderIterator it(tree);

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
      it.Add(nodeReader->GetOutput());
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
  }

  //Part IV: Create window and pass the tree to it
  // create a renderwindow
  QmitkRenderWindow renderWindow;
  // tell the renderwindow which (part of) the tree to render
  renderWindow.GetRenderer()->SetData(&it);

  //Part V: Qt-specific initialization
  renderWindow.show();

  // for testing
  #include "QtTesting.h"
  if(strcmp(argv[argc-1], "-testing")!=0)
    return qtapplication.exec();
  else
    return QtTesting();
}

/**
\example Step2.cpp
*/
