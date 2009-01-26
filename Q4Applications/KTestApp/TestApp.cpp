
#include "QmitkDataStorageTreeModel.h"
#include "QmitkPropertiesTableModel.h"
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkPicFileReader.h>

#include <itksys/SystemTools.hxx>
#include <QApplication>
#include <ui_TestAppGUI.h>

 #include <QDirModel>

 #include <QAbstractTableModel>

#include <QHBoxLayout>
#include <QHeaderView>

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
  mitk::DataTree::Pointer tree=mitk::DataTree::New();

  mitk::DataStorage* storageTree = mitk::DataStorage::CreateInstance(tree);

  //storageTree->SetManageCompleteTree(false);

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

  //// Use the iterator to add the node to the tree
  storageTree->Add(node);

  //mitk::DataTreeNode::Pointer node2=mitk::DataTreeNode::New();
  //node2->SetData(reader->GetOutput());
  //storageTree->Add(node2,node);


  //*************************************************************************
  // Part IV: Create window and pass the tree to it
  //*************************************************************************

  // Create a RenderWindow
  //QmitkRenderWindow renderWindow;
  Ui::TestAppGUI testGui;
  QWidget* hilfswidget = new QWidget();
  testGui.setupUi(hilfswidget);
  
  // Tell the RenderWindow which (part of) the tree to render
  mitk::DataTreePreOrderIterator it(tree);
  testGui.StdMultiWidget->SetData(&it);

 // // Initialize bottom-right view as 3D view
  testGui.StdMultiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D );

 // // Enable standard handler for levelwindow-slider
 testGui.StdMultiWidget->EnableStandardLevelWindow();

 // // Add the displayed views to the tree to see their positions
 // // in 2D and 3D
  testGui.StdMultiWidget->AddDisplayPlaneSubTree(&it);
 // //renderWindow.GetRenderer()->SetData(&it);

  // Initialize the RenderWindow
  mitk::RenderingManager::GetInstance()->InitializeViews( storageTree );

  // Select a slice
  //testGui.widget//.GetSliceNavigationController()->GetSlice()->SetPos( 0 );

  //*************************************************************************
  // Part V: Qt-specific initialization
  //*************************************************************************
  
  /*QmitkDataStorageTreeModel* listModel = new QmitkDataStorageTreeModel();
  testGui.m_TreeView->setModel(listModel);*/

  QmitkPropertiesTableModel* tableModel = new QmitkPropertiesTableModel();
  testGui.m_TableView->setModel(tableModel);
  
  

  QHBoxLayout* layOnMe = new QHBoxLayout();
  //testGui.m_TableView->repaint();
  //testGui->SetLayout(layOnMe);

  testGui.m_TableView->setLayout( layOnMe );
  testGui.m_TableView->repaint();
  testGui.m_TableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);




  //
  //QVBoxLayout *layout = new QVBoxLayout;
  //layout->addWidget(quit);
  //layout->addWidget(lcd);
  //layout->addWidget(slider);
  //setLayout(layout);
  
//  QDirModel *model = new QDirModel;
//QStringListModel* amodel = new QStringListModel;
//QSortFilterProxyModel* bmodel = new QSortFilterProxyModel;
//QSqlQueryModel* cmodel = new QSqlQueryModel;
//QProxyModel* dmodel = new QProxyModel;
//QStandardItemModel emodel* = new QStandardItemModel;
  //QTreeView *tree = new QTreeView(splitter);
  
  
  hilfswidget->show();
  hilfswidget->resize( 256, 256 );

  return qtapplication.exec();

}

/**
\example Step1.cpp
*/

