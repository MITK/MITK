#include "QmitkDataManagerView.h"

//# Own Includes
//## mitk
#include "mitkDataStorageEditorInput.h"
#include "mitkIDataStorageReference.h"
#include "mitkNodePredicateDataType.h"
#include "mitkCoreObjectFactory.h"
#include "mitkPACSPlugin.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkCommon.h"
#include "mitkDelegateManager.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNOT.h"
//## Qmitk
#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageTableModel.h>
#include <QmitkPropertiesTableEditor.h>
#include <QmitkStdMultiWidgetEditor.h>
#include <QmitkCommonFunctionality.h>
//## Cherry
#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

//# Toolkit Includes
#include <QTableView>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QApplication>
#include <QCursor>
#include <QHeaderView>
#include <QSplitter>
#include <QPushButton>
#include <QMotifStyle>
#include <QFileDialog>
#include <QMessageBox>

QmitkDataManagerView::QmitkDataManagerView()
{
/*
  mitk::DelegateManager::GetInstance()->SetCommand("Show Node Info"
    , new mitk::MessageDelegate<QmitkDataManagerView>( this, &QmitkDataManagerView::ShowNodeInfo ));
*/
}


QmitkDataManagerView::~QmitkDataManagerView()
{
/*  mitk::DelegateManager::GetInstance()->RemoveCommand("Show Node Info");*/
}

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  //# Dim/Get

  //# Base
  QGridLayout* parentLayout = new QGridLayout;
  m_BasePane = new QWidget(parent);
  QGridLayout* _BasePaneLayout = new QGridLayout;

  //# DataStorageSelection
  QGroupBox* _DataStorageSelectionGroupBox = new QGroupBox("Data Storage Selection", m_BasePane); 
  QGridLayout* _DataStorageSelectionLayout = new QGridLayout;
  m_DataStorageSelectionComboBox = new QComboBox(_DataStorageSelectionGroupBox);

  //# setup qsplitter
  QSplitter *_SplitterNodeViewPropertiesView = new QSplitter(m_BasePane);
  _SplitterNodeViewPropertiesView->setMargin(0);

  //# NodeView
  QGroupBox* _NodeViewGroupBox = new QGroupBox("Selected Nodes (Use Right Mouse Button for a Context Menu)", m_BasePane);
  QVBoxLayout* _NodeViewLayout = new QVBoxLayout;
  m_NodeTableView = new QTableView(_NodeViewGroupBox);
  // Show only nodes that really contain data
  mitk::NodePredicateData::Pointer nullDataPredicate = mitk::NodePredicateData::New(0);
  mitk::NodePredicateNOT::Pointer notNullDataPredicate = mitk::NodePredicateNOT::New(nullDataPredicate);
  m_NodeTableModel = new QmitkDataStorageTableModel(this->GetDefaultDataStorage(), notNullDataPredicate, m_NodeTableView);

  //# Node actions: buttons, context menu
  QWidget* _PaneNodeButton = new QWidget(_NodeViewGroupBox);
  QHBoxLayout* _NodeButtonLayout = new QHBoxLayout;
  m_BtnLoad = new QPushButton(QIcon(":/datamanager/load.xpm"), "Load", _PaneNodeButton);
  m_BtnGlobalReinit = new QPushButton(QIcon(":/datamanager/refresh.xpm"), "Global Reinit", _PaneNodeButton);

  m_NodeMenu = new QMenu(m_NodeTableView);
  m_SaveAction = m_NodeMenu->addAction(QIcon(":/datamanager/save.xpm"), "Save");

  mitk::PACSPlugin::PACSPluginCapability pacsCapabilities = mitk::PACSPlugin::GetInstance()->GetPluginCapabilities();
  if ( pacsCapabilities.IsPACSFunctional && pacsCapabilities.HasSaveCapability )
  {
    m_SaveAction = m_NodeMenu->addAction(QIcon(":/datamanager/save-chili.xpm"), "Save to PACS");
  }

  m_RemoveAction = m_NodeMenu->addAction(QIcon(":/datamanager/remove.xpm"), "Remove");
  m_ReinitAction = m_NodeMenu->addAction(QIcon(":/datamanager/refresh.xpm"), "Reinit");

  // NodeProperties
  QGroupBox* _NodePropertiesGroupBox = new QGroupBox("Properties", m_BasePane); 
  QHBoxLayout* _NodePropertiesLayout = new QHBoxLayout;
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor(0, _NodePropertiesGroupBox);

  //// Set

  // Base
  parent->setLayout(parentLayout);
  //parentLayout->setMargin(0);
  parentLayout->addWidget(m_BasePane, 0, 0);
  m_BasePane->setLayout(_BasePaneLayout);
  _BasePaneLayout->addWidget(_DataStorageSelectionGroupBox, 0, 0);
  _SplitterNodeViewPropertiesView->setOrientation(Qt::Vertical);
  _SplitterNodeViewPropertiesView->setHandleWidth(5);
  //_SplitterNodeViewPropertiesView->setMargin(5);
  // just to show a visible splitter handle!
  _SplitterNodeViewPropertiesView->setStyle(new QMotifStyle);
  _SplitterNodeViewPropertiesView->addWidget(_NodeViewGroupBox);
  _SplitterNodeViewPropertiesView->addWidget(_NodePropertiesGroupBox);
  _SplitterNodeViewPropertiesView->setStretchFactor(_SplitterNodeViewPropertiesView->indexOf(_NodeViewGroupBox), 1);
  _SplitterNodeViewPropertiesView->setStretchFactor(_SplitterNodeViewPropertiesView->indexOf(_NodePropertiesGroupBox), 2);

  _BasePaneLayout->addWidget(_SplitterNodeViewPropertiesView, 1, 0);
   //_BasePaneLayout->addWidget(_NodeSelectionGroupBox, 1, 0);
//   _BasePaneLayout->addWidget(_NodeViewGroupBox, 1, 0);
//   _BasePaneLayout->addWidget(_NodePropertiesGroupBox, 2, 0);

  // DataStorageSelection
  _DataStorageSelectionGroupBox->setLayout(_DataStorageSelectionLayout);  
  _DataStorageSelectionLayout->addWidget(m_DataStorageSelectionComboBox);
  m_DataStorageSelectionComboBox->setEditable(false);
  m_DataStorageSelectionComboBox->addItem("Default DataStorage");
  m_DataStorageSelectionComboBox->addItem("Active DataStorage");

  // NodeSelection
//   _NodeSelectionGroupBox->setLayout(_NodeSelectionLayout);
//   _NodeSelectionLayout->addWidget(m_PredicateEditor);

  // NodeView
  _NodeViewGroupBox->setLayout(_NodeViewLayout);
  _NodeViewLayout->addWidget(m_NodeTableView);
  _NodeViewLayout->addWidget(_PaneNodeButton);
  m_NodeTableView->setContextMenuPolicy(Qt::CustomContextMenu);  
  m_NodeTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_NodeTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_NodeTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  //m_NodeTableView->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  m_NodeTableView->setAlternatingRowColors(true);
  //m_NodeTableView->setItemDelegate(new QmitkDataStorageDelegate(2, m_NodeTableView));
  m_NodeTableView->setModel(m_NodeTableModel);

  _NodeButtonLayout->addWidget(m_BtnLoad);
  _NodeButtonLayout->addWidget(m_BtnGlobalReinit);
  _PaneNodeButton->setLayout(_NodeButtonLayout);

  // NodeProperties
  _NodePropertiesGroupBox->setLayout(_NodePropertiesLayout);
  _NodePropertiesLayout->addWidget(m_NodePropertiesTableEditor);

  // Connections
  QObject::connect( m_DataStorageSelectionComboBox, SIGNAL(currentIndexChanged(const QString&))
    , this, SLOT( DataStorageSelectionChanged( const QString& ) ) );

  QObject::connect( m_NodeTableView, SIGNAL(clicked(const QModelIndex&))
    , this, SLOT( NodeTableViewClicked( const QModelIndex& )) );

  QObject::connect( m_NodeTableView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex&, const QModelIndex& ))
    , this, SLOT( NodeTableViewSelectionChanged( const QModelIndex&, const QModelIndex& )) );
  
  QObject::connect( m_NodeTableView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );

  QObject::connect( m_SaveAction, SIGNAL( triggered(bool) )
    , this, SLOT( SaveActionTriggered(bool) ) );

  QObject::connect( m_SaveAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionSaveToPacsTriggered(bool) ) );

  QObject::connect( m_ReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionReinitTriggered(bool) ) );

  QObject::connect( m_RemoveAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionRemoveTriggered(bool) ) );

  QObject::connect( m_BtnLoad, SIGNAL( clicked(bool) )
    , this, SLOT( BtnLoadClicked(bool) ) );

  QObject::connect( m_BtnGlobalReinit, SIGNAL( clicked(bool) )
    , this, SLOT( BtnGlobalReinitClicked(bool) ) );

}

void QmitkDataManagerView::Activated()
{
  // emulate click action to show properties of selected node
  if(m_NodeTableView && m_NodeTableView->currentIndex().isValid())
    NodeTableViewClicked(m_NodeTableView->currentIndex());
  else
    if(m_NodePropertiesTableEditor)
      m_NodePropertiesTableEditor->SetPropertyList(0);
}

void QmitkDataManagerView::Deactivated()
{
}

void QmitkDataManagerView::DataStorageSelectionChanged(const QString & text)
{
  //std::cout << "DataStorageSelectionChanged\n";
  //this->m_NodeTableView->set
  QmitkDataStorageTableModel* _NodeTableModel = 0;

  // default datastorage
  if(m_NodeTableView->currentIndex().row() == 0)
  {
    _NodeTableModel = new QmitkDataStorageTableModel(this->GetDefaultDataStorage(), 0, m_NodeTableView);
  }
  else
  {
    _NodeTableModel = new QmitkDataStorageTableModel(this->GetDataStorage(), 0, m_NodeTableView);
  }

  m_NodeTableView->setModel(_NodeTableModel);
  delete m_NodeTableModel;
  m_NodeTableModel = _NodeTableModel;
}

void QmitkDataManagerView::NodeTableViewClicked( const QModelIndex & index )
{
  // select this item if not already selected
  if(m_NodeTableView->currentIndex() != index)
  {
    m_NodeTableView->setCurrentIndex(index);
  }
}


void QmitkDataManagerView::NodeTableViewSelectionChanged( const QModelIndex & current, const QModelIndex & previous )
{
  mitk::DataTreeNode::Pointer selectedNode = m_NodeTableModel->GetNode(current);
  if(selectedNode.IsNotNull())
    m_NodePropertiesTableEditor->SetPropertyList(selectedNode->GetPropertyList());
}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  m_NodeMenu->popup(QCursor::pos());
  QModelIndex selected = m_NodeTableView->indexAt(pos);
  m_NodeTableView->selectRow(selected.row());
}

void QmitkDataManagerView::SaveActionTriggered(bool checked)
{
  QModelIndex indexOfSelectedRow = m_NodeTableView->currentIndex();
  if(!indexOfSelectedRow.isValid()) return;

  mitk::DataTreeNode* node = m_NodeTableModel->GetNode(indexOfSelectedRow);
  if ( !node ) return;

  mitk::BaseData::Pointer data = node->GetData();

  if (data.IsNotNull())
  {
    CommonFunctionality::SaveBaseData( data.GetPointer(), node->GetName().c_str() );
  }
}

void QmitkDataManagerView::ActionReinitTriggered( bool checked /*= false */ )
{
  QModelIndex indexOfSelectedRow = m_NodeTableView->currentIndex();
  if(indexOfSelectedRow.isValid())
  {
    mitk::DataTreeNode* node = m_NodeTableModel->GetNode(indexOfSelectedRow);
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      if (basedata.IsNotNull())
      {
        mitk::RenderingManager::GetInstance()->InitializeViews(
          basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
}

void QmitkDataManagerView::ActionRemoveTriggered( bool checked /*= false */ )
{
  QModelIndex indexOfSelectedRow = m_NodeTableView->currentIndex();
  if(!indexOfSelectedRow.isValid()) 
    return;

  mitk::DataTreeNode* node = m_NodeTableModel->GetNode(indexOfSelectedRow);
  if ( !node )
    return;

  switch(QMessageBox::question(m_Parent, tr("DataManager")
    , tr("Do you really want to delete the item '").append(node->GetName().c_str()).append("' ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) 
  {
    case QMessageBox::Yes: //Remove the item from view and tree
      this->GetDataStorage()->Remove(node);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;

    case QMessageBox::No: 
    case QMessageBox::Cancel:
	default:
	 break;
  }
}

void QmitkDataManagerView::BtnLoadClicked( bool checked /*= false */ )
{
  QModelIndex indexOfSelectedRow = m_NodeTableView->currentIndex();

  if(!indexOfSelectedRow.isValid()) return;

  mitk::DataTreeNode* node = m_NodeTableModel->GetNode(indexOfSelectedRow);
  if ( !node )
    return;

  QStringList fileNames = QFileDialog::getOpenFileNames(mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(), NULL);
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    FileOpen((*it).ascii(), node);
  }
}

void QmitkDataManagerView::FileOpen( const char * fileName, mitk::DataTreeNode* parentNode )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    factory->SetFileName( fileName );

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    factory->Update();

    for ( unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i )
    {
      mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
      if ( ( node.IsNotNull() ) && ( node->GetData() != NULL ) )
      {
        this->GetDataStorage()->Add(node, parentNode);
      }
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
  }

  QApplication::restoreOverrideCursor();
}

void QmitkDataManagerView::BtnGlobalReinitClicked( bool checked /*= false */ )
{
  mitk::RenderingManager::GetInstance()->InitializeViews( this->GetDefaultDataStorage() );
}

void QmitkDataManagerView::ActionSaveToPacsTriggered ( bool checked )
{
  QModelIndex indexOfSelectedRow = m_NodeTableView->currentIndex();
  if(!indexOfSelectedRow.isValid()) 
    return;

  mitk::DataTreeNode* node = m_NodeTableModel->GetNode(indexOfSelectedRow);
  if ( !node )
    return;

  std::vector<mitk::DataTreeNode*> nodesToSave;
  nodesToSave.clear();
  nodesToSave.push_back( node );

  // TODO
/*
  QmitkPACSSaveDialog dialog(this);
  dialog.SetDataTreeNodesToSave( nodesToSave );
  dialog.setAllowSeveralObjectsIntoOneSeries(true); // just cannot select more than one object here :-(
  dialog.exec();

  if ( dialog.result() == QDialog::Accepted )
  {
    std::cout << "PACS export dialog reported success." << std::endl;
  }
  else
  {
    std::cout << "PACS export dialog cancelled by user." << std::endl;
  }*/

}

void QmitkDataManagerView::ShowNodeInfo()
{
  QModelIndex current = m_NodeTableView->currentIndex();
  if(!current.isValid())
    return;

  mitk::DataTreeNode::Pointer selectedNode = m_NodeTableModel->GetNode(current);
  QString nodeName = QString::fromStdString(selectedNode->GetName());
  bool nodeIsVisible = false;
  selectedNode->GetVisibility(nodeIsVisible, 0);

  QMessageBox msgBox;
  msgBox.setWindowTitle("Node Info");
  
  QString info("node name: %1\nvisible: %2");
  info = info.arg(nodeName).arg(nodeIsVisible);
  msgBox.setText(info);
  msgBox.exec();
}

void QmitkDataManagerView::SingleNodeSelection::SetNode( mitk::DataTreeNode* _SelectedNode )
{
  m_Node = _SelectedNode;
}

mitk::DataTreeNode* QmitkDataManagerView::SingleNodeSelection::GetNode() const
{
  return m_Node;
}

bool QmitkDataManagerView::SingleNodeSelection::IsEmpty() const
{
  return ( m_Node == 0 );
}