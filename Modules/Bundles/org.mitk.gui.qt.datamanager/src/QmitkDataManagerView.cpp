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
#include "src/internal/QmitkNodeTableViewKeyFilter.h"
#include "src/internal/QmitkInfoDialog.h"
//## Cherry
#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>
#include <cherryIPreferencesService.h>
#include <cherryPlatform.h>

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
  if(m_DataManagerPreferencesNode.IsNotNull())
    m_DataManagerPreferencesNode->OnChanged
    .RemoveListener(cherry::MessageDelegate1<QmitkDataManagerView, const cherry::ICherryPreferences*>(this, &QmitkDataManagerView::OnPreferencesChanged));

}

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  //# Preferences
  cherry::IPreferencesService::Pointer prefService 
    = cherry::Platform::GetServiceRegistry()
    .GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);

  m_DataManagerPreferencesNode = (prefService->GetSystemPreferences()->Node("/DataManager")).Cast<cherry::ICherryPreferences>();
  if(m_DataManagerPreferencesNode.IsNotNull())
    m_DataManagerPreferencesNode->OnChanged
      .AddListener(cherry::MessageDelegate1<QmitkDataManagerView, const cherry::ICherryPreferences*>(this, &QmitkDataManagerView::OnPreferencesChanged));

  //# GUI
  //# Button panel
  QPushButton* _BtnLoad = new QPushButton(QIcon(":/datamanager/load.xpm"), "Load");
  QObject::connect( _BtnLoad, SIGNAL( clicked(bool) )
    , this, SLOT( Load(bool) ) );

  QPushButton* _BtnGlobalReinit = new QPushButton(QIcon(":/datamanager/refresh.xpm"), "Global Reinit");
  QObject::connect( _BtnGlobalReinit, SIGNAL( clicked(bool) )
    , this, SLOT( GlobalReinit(bool) ) );

  QHBoxLayout* _NodeButtonLayout = new QHBoxLayout;
  _NodeButtonLayout->addWidget(_BtnLoad);
  _NodeButtonLayout->addWidget(_BtnGlobalReinit);

  QWidget* _PaneNodeButton = new QWidget;
  _PaneNodeButton->setLayout(_NodeButtonLayout);

  //# m_NodeTableModel
  mitk::NodePredicateData::Pointer nullDataPredicate = mitk::NodePredicateData::New(0);
  mitk::NodePredicateNOT::Pointer notNullDataPredicate 
    = mitk::NodePredicateNOT::New(nullDataPredicate);// Show only nodes that really contain data
  m_NodeTableModel = new QmitkDataStorageTableModel(this->GetDefaultDataStorage(), notNullDataPredicate);

  //# m_NodeTableView
  m_NodeTableView = new QTableView;
  m_NodeTableView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_NodeTableView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  m_NodeTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  //m_NodeTableView->horizontalHeader()->setStretchLastSection(true);
  m_NodeTableView->installEventFilter(new QmitkNodeTableViewKeyFilter(this));
  m_NodeTableView->setAlternatingRowColors(true);
  m_NodeTableView->setSortingEnabled(true);
  m_NodeTableView->setModel(m_NodeTableModel);
  QObject::connect( m_NodeTableView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );

  //# m_SelectionProvider
  m_SelectionProvider = new QmitkDataTreeNodeSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_NodeTableView->selectionModel());
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);

  m_NodeMenu = new QMenu(m_NodeTableView);

  // # Actions
  QAction* _SaveAction = m_NodeMenu->addAction(QIcon(":/datamanager/save.xpm"), "Save selected nodes");
  QObject::connect( _SaveAction, SIGNAL( triggered(bool) )
    , this, SLOT( SaveSelectedNodes(bool) ) );

  QAction* _RemoveAction = m_NodeMenu->addAction(QIcon(":/datamanager/remove.xpm"), "Remove selected nodes");
  QObject::connect( _RemoveAction, SIGNAL( triggered(bool) )
    , this, SLOT( RemoveSelectedNodes(bool) ) );

  QAction* _ReinitAction = m_NodeMenu->addAction(QIcon(":/datamanager/refresh.xpm"), "Reinit selected nodes");
  QObject::connect( _ReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ReinitSelectedNodes(bool) ) );

  QAction* _ShowOnlySelectedNodes 
    = m_NodeMenu->addAction(QIcon(":/datamanager/data-type-image-mask-visible-24.png")
    , "Show only selected nodes");
  QObject::connect( _ShowOnlySelectedNodes, SIGNAL( triggered(bool) )
    , this, SLOT( ShowOnlySelectedNodes(bool) ) );

  QAction* _ToggleSelectedVisibility 
    = m_NodeMenu->addAction(QIcon(":/datamanager/data-type-image-mask-invisible-24.png")
    , "Toggle visibility of selected nodes");
  QObject::connect( _ToggleSelectedVisibility, SIGNAL( triggered(bool) )
    , this, SLOT( ToggleVisibilityOfSelectedNodes(bool) ) );

  QAction* _ActionShowInfoDialog 
    = m_NodeMenu->addAction(QIcon(":/datamanager/show-data-info.png")
    , "Show additional information for selected nodes");
  QObject::connect( _ActionShowInfoDialog, SIGNAL( triggered(bool) )
    , this, SLOT( ShowInfoDialogForSelectedNodes(bool) ) );

  QToolBar* _NodeToolbar = new QToolBar;
  _NodeToolbar->setIconSize(QSize(16, 16));
  _NodeToolbar->addAction(_SaveAction);
  _NodeToolbar->addAction(_RemoveAction);
  _NodeToolbar->addAction(_ReinitAction);
  _NodeToolbar->addAction(_ShowOnlySelectedNodes);
  _NodeToolbar->addAction(_ToggleSelectedVisibility);
  _NodeToolbar->addAction(_ActionShowInfoDialog);

  QVBoxLayout* _NodeViewLayout = new QVBoxLayout;
  _NodeViewLayout->addWidget(_NodeToolbar);
  _NodeViewLayout->addWidget(m_NodeTableView);
  _NodeViewLayout->addWidget(_PaneNodeButton);

  QGroupBox* _NodeViewGroupBox = new QGroupBox("Data Nodes");
  _NodeViewGroupBox->setLayout(_NodeViewLayout);

  QGridLayout* _ParentLayout = new QGridLayout;
  _ParentLayout->addWidget(_NodeViewGroupBox, 0, 0);

  m_Parent->setLayout(_ParentLayout);

  // call preferences changed to enable initial single click editing or not
  this->OnPreferencesChanged(m_DataManagerPreferencesNode.GetPointer());
}


void QmitkDataManagerView::OnPreferencesChanged(const cherry::ICherryPreferences* prefs )
{
  if(prefs->GetBool("Single click property editing", true))
  {
    m_NodeTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | 
      QAbstractItemView::EditKeyPressed);
  }
  else
  {
    m_NodeTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  }
}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  m_NodeMenu->popup(QCursor::pos());
}

void QmitkDataManagerView::SaveSelectedNodes( bool checked /*= false */ )
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

void QmitkDataManagerView::ReinitSelectedNodes( bool checked /*= false */ )
{
  std::vector<mitk::DataTreeNode*> selectedNodes = this->GetSelectedNodes();

  mitk::DataTreeNode* node = 0;
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

void QmitkDataManagerView::RemoveSelectedNodes( bool checked /*= false */ )
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
      this->GlobalReinit(false);

    }
  }
}

void QmitkDataManagerView::MakeAllNodesInvisible( bool checked /*= false */ )
{
  std::vector<mitk::DataTreeNode*> nodes = m_NodeTableModel->GetNodeSet();

  for (std::vector<mitk::DataTreeNode*>::iterator it = nodes.begin()
    ; it != nodes.end(); it++)
  {
    (*it)->SetVisibility(false);
  }
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowOnlySelectedNodes( bool checked )
{
  std::vector<mitk::DataTreeNode*> selectedNodes = this->GetSelectedNodes();
  std::vector<mitk::DataTreeNode*> allNodes = m_NodeTableModel->GetNodeSet();
  mitk::DataTreeNode* node = 0;

  for (std::vector<mitk::DataTreeNode*>::iterator it = allNodes.begin()
    ; it != allNodes.end(); it++)
  {
    node = *it;
    if(std::find(selectedNodes.begin(), selectedNodes.end(), node) == selectedNodes.end())
      node->SetVisibility(false);
    else
      node->SetVisibility(true);
  }
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ToggleVisibilityOfSelectedNodes( bool checked /*= false */ )
{
  std::vector<mitk::DataTreeNode*> selectedNodes = this->GetSelectedNodes();

  bool isVisible = false;
  mitk::DataTreeNode* node = 0;
  for (std::vector<mitk::DataTreeNode*>::iterator it = selectedNodes.begin()
    ; it != selectedNodes.end(); it++)
  {
    isVisible = false;
    node = *it;
    node->GetBoolProperty("visible", isVisible);
    node->SetVisibility(!isVisible);
  }
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowInfoDialogForSelectedNodes( bool checked /*= false */ )
{
  std::vector<mitk::DataTreeNode*> selectedNodes = this->GetSelectedNodes();

  QmitkInfoDialog _QmitkInfoDialog(selectedNodes, this->m_Parent);
  _QmitkInfoDialog.exec();
}

void QmitkDataManagerView::Load( bool checked )
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
        mitk::BaseData::Pointer basedata = node->GetData();
        mitk::RenderingManager::GetInstance()->InitializeViews(
          basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
  }

  QApplication::restoreOverrideCursor();
}

void QmitkDataManagerView::GlobalReinit( bool checked /*= false */ )
{
  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNOT::Pointer pred 
    = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
    , mitk::BoolProperty::New(false)));

  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  // calculate bounding geometry of these nodes
  mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs);
  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

std::vector<mitk::DataTreeNode*> QmitkDataManagerView::GetSelectedNodes() const
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

  return selectedNodes;
}