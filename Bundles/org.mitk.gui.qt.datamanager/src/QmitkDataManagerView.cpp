#include "QmitkDataManagerView.h"


#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageReference.h>
#include <mitkNodePredicateDataType.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageTableModel.h>
#include <QmitkPropertiesTableEditor.h>
#include <QmitkStdMultiWidgetEditor.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

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

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{

  //// Dim/Get

  // Base
  QGridLayout* parentLayout = new QGridLayout;
  m_BasePane = new QWidget(parent);
  QGridLayout* _BasePaneLayout = new QGridLayout;

  // DataStorageSelection
  QGroupBox* _DataStorageSelectionGroupBox = new QGroupBox("Data Storage Selection", m_BasePane); 
  QGridLayout* _DataStorageSelectionLayout = new QGridLayout;
  m_DataStorageSelectionComboBox = new QComboBox(_DataStorageSelectionGroupBox);

  // NodeSelection
  QGroupBox* _NodeSelectionGroupBox = new QGroupBox("Node Selection", m_BasePane);
  QVBoxLayout* _NodeSelectionLayout = new QVBoxLayout;
  m_NodeSelectionModeComboBox = new QComboBox(_NodeSelectionGroupBox);

  // NodeView
  QGroupBox* _NodeViewGroupBox = new QGroupBox("Selected Nodes", m_BasePane);
  QVBoxLayout* _NodeViewLayout = new QVBoxLayout;
  m_NodeTableView = new QTableView(_NodeViewGroupBox);
  m_NodeTableModel = new QmitkDataStorageTableModel(this->GetDefaultDataStorage(), 0, m_NodeTableView);
  m_ShowDerivedNodesMenu = new QMenu(m_NodeTableView);
  m_ShowDerivedNodesAction = m_ShowDerivedNodesMenu->addAction("Show Derived Nodes");

  // NodeProperties
  QGroupBox* _NodePropertiesGroupBox = new QGroupBox("Properties", m_BasePane); 
  QHBoxLayout* _NodePropertiesLayout = new QHBoxLayout;
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor(0, _NodePropertiesGroupBox);
  
  //// Set

  // Base
  parent->setLayout(parentLayout);
  parentLayout->addWidget(m_BasePane, 0, 0);
  m_BasePane->setLayout(_BasePaneLayout);
  _BasePaneLayout->addWidget(_DataStorageSelectionGroupBox, 0, 0);
  _BasePaneLayout->addWidget(_NodeSelectionGroupBox, 1, 0);
  _BasePaneLayout->addWidget(_NodeViewGroupBox, 2, 0);
  _BasePaneLayout->addWidget(_NodePropertiesGroupBox, 3, 0);

  // DataStorageSelection
  _DataStorageSelectionGroupBox->setLayout(_DataStorageSelectionLayout);  
  _DataStorageSelectionLayout->addWidget(m_DataStorageSelectionComboBox);
  m_DataStorageSelectionComboBox->setEditable(false);
  m_DataStorageSelectionComboBox->addItem("Default DataStorage");
  m_DataStorageSelectionComboBox->addItem("Active DataStorage");

  // NodeSelection
  _NodeSelectionGroupBox->setLayout(_NodeSelectionLayout);
  _NodeSelectionLayout->addWidget(m_NodeSelectionModeComboBox);
  m_NodeSelectionModeComboBox->setEditable(false);
  m_NodeSelectionModeComboBox->addItem("All Nodes");

  // NodeView
  _NodeViewGroupBox->setLayout(_NodeViewLayout);
  _NodeViewLayout->addWidget(m_NodeTableView);
  m_NodeTableView->setContextMenuPolicy(Qt::CustomContextMenu);  
  m_NodeTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_NodeTableView->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_NodeTableView->setModel(m_NodeTableModel);
  m_NodeTableView->horizontalHeader()->setStretchLastSection ( true );

  // NodeProperties
  _NodePropertiesGroupBox->setLayout(_NodePropertiesLayout);
  _NodePropertiesLayout->addWidget(m_NodePropertiesTableEditor);


  // Connections
  QObject::connect( m_DataStorageSelectionComboBox, SIGNAL(currentIndexChanged(const QString&))
    , this, SLOT( DataStorageSelectionChanged( const QString& ) ) );

  QObject::connect( m_NodeSelectionModeComboBox, SIGNAL(currentIndexChanged(const QString&))
    , this, SLOT( NodeSelectionModeChanged( const QString& )) );

  QObject::connect( m_NodeTableView, SIGNAL(clicked(const QModelIndex&))
    , this, SLOT( NodeTableViewClicked( const QModelIndex& )) );
  
  QObject::connect( m_NodeTableView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );

  QObject::connect( m_ShowDerivedNodesAction, SIGNAL( triggered(bool) )
    , this, SLOT( ShowDerivedNodesClicked(bool) ) );

}

void QmitkDataManagerView::NodeAdded(const mitk::DataTreeNode* node)
{
}

void QmitkDataManagerView::Activated()
{

}

void QmitkDataManagerView::Deactivated()
{

}

QmitkDataManagerView::~QmitkDataManagerView()
{
  //this->GetSite()->GetPage()->RemovePartListener(m_MultiWidgetListener);
}

void QmitkDataManagerView::DataStorageSelectionChanged(const QString & text)
{
  std::cout << "DataStorageSelectionChanged\n";
}

void QmitkDataManagerView::NodeSelectionModeChanged(const QString & text)
{
  std::cout << "NodeSelectionModeChanged\n";
}

void QmitkDataManagerView::NodeTableViewClicked( const QModelIndex & index )
{
  mitk::DataTreeNode::Pointer selectedNode = m_NodeTableModel->getNode(index);
  if(selectedNode.IsNotNull())
    m_NodePropertiesTableEditor->setNode(selectedNode);

  std::cout << "NodeTableViewClicked\n";
}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  m_ShowDerivedNodesMenu->popup(pos);
}

void QmitkDataManagerView::ShowDerivedNodesClicked(bool checked)
{
  std::cout << "ShowDerivedNodesClicked\n";
}