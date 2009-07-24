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
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"
//## Qmitk
#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageTableModel.h>
#include <QmitkPropertiesTableEditor.h>
#include <QmitkStdMultiWidgetEditor.h>
#include <QmitkCommonFunctionality.h>
#include <src/internal/QmitkDelKeyFilter.h>
#include <src/internal/QmitkInfoDialog.h>
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
#include <QToolBar>
#include <QKeyEvent>

QmitkDataManagerView::QmitkDataManagerView()
{
}


QmitkDataManagerView::~QmitkDataManagerView()
{
}

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  //# Dim/Get

  //# Base
  QGridLayout* _ParentLayout = new QGridLayout;

  //# DataStorageSelection
  QGroupBox* _DataStorageSelectionGroupBox = new QGroupBox("Data Storage Selection", m_Parent);
  QGridLayout* _DataStorageSelectionLayout = new QGridLayout;
  m_DataStorageSelectionComboBox = new QComboBox(_DataStorageSelectionGroupBox);

  //# setup qsplitter
  QSplitter *_SplitterNodeViewPropertiesView = new QSplitter(m_Parent);
  _SplitterNodeViewPropertiesView->setMargin(0);

  //# NodeView
  QGroupBox* _NodeViewGroupBox = new QGroupBox("Selected Nodes (Use Right Mouse Button for a Context Menu)", m_Parent);
  m_NodeToolbar = new QToolBar(_NodeViewGroupBox);
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
  m_ToggleSelectedVisibility = m_NodeMenu->addAction(QIcon(":/datamanager/data-type-image-mask-invisible-24.png"), "Toggle visibility of selected nodes");
  m_ActionShowInfoDialog = m_NodeMenu->addAction(QIcon(":/datamanager/show-data-info.png"), "Show additional infos for selected nodes");

  // NodeProperties
  QGroupBox* _NodePropertiesGroupBox = new QGroupBox("Properties", m_Parent);
  QHBoxLayout* _NodePropertiesLayout = new QHBoxLayout;
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor(0, _NodePropertiesGroupBox);

  //// Set

  // Base
  m_Parent->setLayout(_ParentLayout);
  _ParentLayout->addWidget(_DataStorageSelectionGroupBox, 0, 0);

  _SplitterNodeViewPropertiesView->setOrientation(Qt::Vertical);
  //_SplitterNodeViewPropertiesView->setHandleWidth(5);
  _SplitterNodeViewPropertiesView->addWidget(_NodeViewGroupBox);
  _SplitterNodeViewPropertiesView->addWidget(_NodePropertiesGroupBox);
  //_SplitterNodeViewPropertiesView->setStretchFactor(_SplitterNodeViewPropertiesView->indexOf(_NodeViewGroupBox), 1);
  //_SplitterNodeViewPropertiesView->setStretchFactor(_SplitterNodeViewPropertiesView->indexOf(_NodePropertiesGroupBox), 2);

  _ParentLayout->addWidget(_SplitterNodeViewPropertiesView, 1, 0);
  // _ParentLayout->addWidget(_NodeSelectionGroupBox, 1, 0);
  //_ParentLayout->addWidget(_NodeViewGroupBox, 1, 0);
  // _ParentLayout->addWidget(_NodePropertiesGroupBox, 2, 0);

  // DataStorageSelection
  _DataStorageSelectionGroupBox->setLayout(_DataStorageSelectionLayout);
  _DataStorageSelectionLayout->addWidget(m_DataStorageSelectionComboBox);
  m_DataStorageSelectionComboBox->setEditable(false);
  m_DataStorageSelectionComboBox->addItem("Default DataStorage");
  m_DataStorageSelectionComboBox->addItem("Active DataStorage");

  // NodeSelection
  // _NodeSelectionGroupBox->setLayout(_NodeSelectionLayout);
  // _NodeSelectionLayout->addWidget(m_PredicateEditor);

  // NodeView
  _NodeViewGroupBox->setLayout(_NodeViewLayout);
  _NodeViewLayout->addWidget(m_NodeToolbar);
  _NodeViewLayout->addWidget(m_NodeTableView);
  _NodeViewLayout->addWidget(_PaneNodeButton);
  m_NodeTableView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_NodeTableView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  m_NodeTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_NodeTableView->horizontalHeader()->setStretchLastSection(true);
  m_NodeTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
  m_NodeTableView->installEventFilter(new QmitkDelKeyFilter(this));
  //m_NodeTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  //m_NodeTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  //m_NodeTableView->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  m_NodeTableView->setAlternatingRowColors(true);
  m_NodeTableView->setSortingEnabled(true);
  //m_NodeTableView->setItemDelegate(new QmitkDataStorageDelegate(2, m_NodeTableView));
  m_NodeTableView->setModel(m_NodeTableModel);

  m_SelectionProvider = new QmitkDataTreeNodeSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_NodeTableView->selectionModel());
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);

  m_NodeToolbar->setIconSize(QSize(16, 16));
  m_NodeToolbar->addAction(m_SaveAction);
  m_NodeToolbar->addAction(m_RemoveAction);
  m_NodeToolbar->addAction(m_ReinitAction);
  m_NodeToolbar->addAction(m_ToggleSelectedVisibility);
  m_NodeToolbar->addAction(m_ActionShowInfoDialog);

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

  QObject::connect( m_ToggleSelectedVisibility, SIGNAL( triggered(bool) )
    , this, SLOT( ActionToggleSelectedVisibilityTriggered(bool) ) );

  QObject::connect( m_ActionShowInfoDialog, SIGNAL( triggered(bool) )
    , this, SLOT( ActionShowInfoDialogTriggered(bool) ) );

  QObject::connect( m_BtnLoad, SIGNAL( clicked(bool) )
    , this, SLOT( BtnLoadClicked(bool) ) );

  QObject::connect( m_BtnGlobalReinit, SIGNAL( clicked(bool) )
    , this, SLOT( BtnGlobalReinitClicked(bool) ) );

  QObject::connect( m_NodePropertiesTableEditor, SIGNAL( destroyed(QObject*) )
    , this, SLOT( QtObjectDestroyed(QObject*) ) );

}

void QmitkDataManagerView::Activated()
{
/*
  // emulate click action to show properties of selected node
  if(m_NodeTableView && m_NodeTableView->currentIndex().isValid())
    NodeTableViewClicked(m_NodeTableView->currentIndex());
  else
    if(m_NodePropertiesTableEditor)
      m_NodePropertiesTableEditor->SetPropertyList(0);*/


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

   QObject::connect( m_NodeTableView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex&, const QModelIndex& ))
    , this, SLOT( NodeTableViewSelectionChanged( const QModelIndex&, const QModelIndex& )) );

}

void QmitkDataManagerView::NodeTableViewClicked( const QModelIndex & index )
{
  // select this item if not already selected
/*
  if(m_NodeTableView->currentIndex() != index)
  {
    m_NodeTableView->setCurrentIndex(index);
  }*/

}


void QmitkDataManagerView::NodeTableViewSelectionChanged( const QModelIndex & current, const QModelIndex & previous )
{
  mitk::DataTreeNode::Pointer selectedNode = m_NodeTableModel->GetNode(current);
  if(selectedNode.IsNotNull() && m_NodePropertiesTableEditor != 0)
    m_NodePropertiesTableEditor->SetPropertyList(selectedNode->GetPropertyList());

}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  m_NodeMenu->popup(QCursor::pos());
  //QModelIndex selected = m_NodeTableView->indexAt(pos);
  //m_NodeTableView->selectRow(selected.row());
}

void QmitkDataManagerView::SaveActionTriggered(bool checked)
{
  QModelIndexList indexesOfSelectedRows = m_NodeTableView->selectionModel()->selectedRows();

  mitk::DataTreeNode* node = 0;
  unsigned int indexesOfSelectedRowsSize = indexesOfSelectedRows.size();
  for (unsigned int i = 0; i<indexesOfSelectedRowsSize; ++i)
  {
    node = m_NodeTableModel->GetNode(indexesOfSelectedRows.at(i));
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node != 0 )
    {
      mitk::BaseData::Pointer data = node->GetData();
      if (data.IsNotNull())
        CommonFunctionality::SaveBaseData( data.GetPointer(), node->GetName().c_str() );
    }
  }

}

void QmitkDataManagerView::ActionReinitTriggered( bool checked /*= false */ )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTableView->selectionModel()->selectedRows();
  std::vector<mitk::DataTreeNode*> selectedNodes;

  mitk::DataTreeNode* node = 0;
  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = m_NodeTableModel->GetNode(*it);
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node != 0 )
      selectedNodes.push_back(node);
  }

  for (std::vector<mitk::DataTreeNode*>::iterator it = selectedNodes.begin()
    ; it != selectedNodes.end(); it++)
  {
    node = *it;
    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
      mitk::RenderingManager::GetInstance()->InitializeViews(
        basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkDataManagerView::ActionRemoveTriggered( bool checked /*= false */ )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTableView->selectionModel()->selectedRows();
  std::vector<mitk::DataTreeNode*> selectedNodes;

  mitk::DataTreeNode* node = 0;
  QString question = tr("Do you really want to delete ");

  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = m_NodeTableModel->GetNode(*it);
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node != 0 /*& strcmp(node->GetData()->GetNameOfClass(), "Geometry2DData") != 0*/ )
    {
      selectedNodes.push_back(node);
      question.append(QString::fromStdString(node->GetName()));
      question.append(", ");
    }
  }
  // remove the last two characters = ", "
  question = question.remove(question.size()-2, 2);
  question.append("?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( m_Parent
    , tr("DataManager")
    , question
    , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if(answerButton == QMessageBox::Yes)
  {
    for (std::vector<mitk::DataTreeNode*>::iterator it = selectedNodes.begin()
      ; it != selectedNodes.end(); it++)
    {
      node = *it;
      this->GetDataStorage()->Remove(node);
      this->BtnGlobalReinitClicked(false);

    }
  }
}

void QmitkDataManagerView::ActionToggleSelectedVisibilityTriggered( bool checked /*= false */ )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTableView->selectionModel()->selectedRows();
  std::vector<mitk::DataTreeNode*> selectedNodes;

  mitk::DataTreeNode* node = 0;
  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = 0;
    node = m_NodeTableModel->GetNode(*it);
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node != 0 )
      selectedNodes.push_back(node);
  }

  bool isVisible = false;
  for (std::vector<mitk::DataTreeNode*>::iterator it = selectedNodes.begin()
    ; it != selectedNodes.end(); it++)
  {
    isVisible = false;
    node = *it;
    node->GetBoolProperty("visible", isVisible);
    node->SetVisibility(!isVisible);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ActionShowInfoDialogTriggered( bool checked /*= false */ )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTableView->selectionModel()->selectedRows();
  std::vector<mitk::DataTreeNode*> selectedNodes;

  mitk::DataTreeNode* node = 0;
  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = 0;
    node = m_NodeTableModel->GetNode(*it);
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node != 0 )
      selectedNodes.push_back(node);
  }

  QmitkInfoDialog _QmitkInfoDialog(selectedNodes, this->m_Parent);
  _QmitkInfoDialog.exec();
}

void QmitkDataManagerView::BtnLoadClicked( bool checked /*= false */ )
{
  QStringList fileNames = QFileDialog::getOpenFileNames(mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(), NULL);
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    FileOpen((*it).ascii(), 0);
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
  /* get all nodes that have not set "includeInBoundingBox" to false */
  mitk::NodePredicateNOT::Pointer pred = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  /* calculate bounding geometry of these nodes */
  mitk::Geometry3D::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs);
  /* initialize the views to the bounding geometry */
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
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

void QmitkDataManagerView::QtObjectDestroyed( QObject * obj /*= 0 */ )
{
  if(obj == m_NodePropertiesTableEditor)
    m_NodePropertiesTableEditor = 0;
}