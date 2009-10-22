#include "QmitkDataManagerView.h"

#include <itkOtsuThresholdImageFilter.h>

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
#include <mitkNodePredicateAND.h>
#include <mitkITKImageImport.h>
//## Qmitk
#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageTableModel.h>
#include <QmitkPropertiesTableEditor.h>
#include <QmitkStdMultiWidgetEditor.h>
#include <QmitkCommonFunctionality.h>
#include <QmitkDataStorageTreeModel.h>
#include <QmitkNodeDescriptorManager.h>
#include <QmitkCustomVariants.h>
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
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QApplication>
#include <QCursor>
#include <QHeaderView>
#include <QTreeView>
#include <QWidgetAction>
#include <QSplitter>
#include <QPushButton>
#include <QMotifStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QKeyEvent>
#include "mitkDataTreeNodeObject.h"

QmitkDataManagerView::QmitkDataManagerView()
{
}


QmitkDataManagerView::~QmitkDataManagerView()
{
  if(m_DataManagerPreferencesNode.IsNotNull())
    m_DataManagerPreferencesNode->OnChanged
    .RemoveListener(cherry::MessageDelegate1<QmitkDataManagerView, const cherry::ICherryPreferences*>(this, &QmitkDataManagerView::OnPreferencesChanged));

  cherry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemoveSelectionListener(m_SelectionListener);

  QmitkNodeDescriptor* unknownDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataTreeNodeDescriptor();

  unknownDataTreeNodeDescriptor->RemoveAction(m_GlobalReinitAction);
  unknownDataTreeNodeDescriptor->RemoveAction(m_SaveAction);
  unknownDataTreeNodeDescriptor->RemoveAction(m_RemoveAction);
  unknownDataTreeNodeDescriptor->RemoveAction(m_ReinitAction);
  unknownDataTreeNodeDescriptor->RemoveAction(m_OpacityAction);

  QmitkNodeDescriptor* imageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  imageDataTreeNodeDescriptor->RemoveAction(m_OtsuFilterAction);

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
  m_NodeTreeModel = new QmitkDataStorageTreeModel(this->GetDataStorage());

  //# Tree View (experimental)
  m_NodeTreeView = new QTreeView;
  m_NodeTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  m_NodeTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_NodeTreeView->setAlternatingRowColors(true);
  m_NodeTreeView->setDragEnabled(true);
  m_NodeTreeView->setDropIndicatorShown(true);
  m_NodeTreeView->setAcceptDrops(true);
  m_NodeTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_NodeTreeView->setModel(m_NodeTreeModel);
  QObject::connect( m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );

  //# m_NodeMenu
  m_NodeMenu = new QMenu(m_NodeTreeView);

  //# m_SelectionProvider
  m_SelectionProvider = new QmitkDataTreeNodeSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_NodeTreeView->selectionModel());
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);

  // # Actions
  QmitkNodeDescriptor* unknownDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataTreeNodeDescriptor();

  m_GlobalReinitAction = unknownDataTreeNodeDescriptor->AddAction(QIcon(":/datamanager/refresh.xpm"), "Global Reinit");
  QObject::connect( m_GlobalReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( GlobalReinit(bool) ) );

  m_SaveAction = unknownDataTreeNodeDescriptor->AddAction(QIcon(":/datamanager/save.xpm"), "Save selected nodes");
  QObject::connect( m_SaveAction, SIGNAL( triggered(bool) )
    , this, SLOT( SaveSelectedNodes(bool) ) );

  m_RemoveAction = unknownDataTreeNodeDescriptor->AddAction(QIcon(":/datamanager/remove.xpm"), "Remove selected nodes");
  QObject::connect( m_RemoveAction, SIGNAL( triggered(bool) )
    , this, SLOT( RemoveSelectedNodes(bool) ) );

  m_ReinitAction = unknownDataTreeNodeDescriptor->AddAction(QIcon(":/datamanager/refresh.xpm"), "Reinit selected nodes");
  QObject::connect( m_ReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ReinitSelectedNodes(bool) ) );

  QmitkNodeDescriptor* imageDataTreeNodeDescriptor = 
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  m_OtsuFilterAction = imageDataTreeNodeDescriptor->AddAction("Apply Otsu Filter");
  QObject::connect( m_OtsuFilterAction, SIGNAL( triggered(bool) )
    , this, SLOT( OtsuFilter(bool) ) );

  m_OpacitySlider = new QSlider;
  m_OpacitySlider->setMinimum(0);
  m_OpacitySlider->setMaximum(100);
  m_OpacitySlider->setOrientation(Qt::Horizontal);
  QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
    , this, SLOT( OpactiyChanged(int) ) );

  QLabel* _OpacityLabel = new QLabel("Opacity: ");
  QHBoxLayout* _OpacityWidgetLayout = new QHBoxLayout;
  _OpacityWidgetLayout->addWidget(_OpacityLabel);
  _OpacityWidgetLayout->addWidget(m_OpacitySlider);
  QWidget* _OpacityWidget = new QWidget;
  _OpacityWidget->setLayout(_OpacityWidgetLayout);

  m_OpacityAction = unknownDataTreeNodeDescriptor->AddWidgetAction(false);
  m_OpacityAction->setDefaultWidget(_OpacityWidget);
  QObject::connect( m_OpacityAction, SIGNAL( changed() )
    , this, SLOT( OpactiyActionChanged() ) );
// 
//   QAction* _ShowOnlySelectedNodes 
//     = m_NodeMenu->addAction(QIcon(":/datamanager/data-type-image-mask-visible-24.png")
//     , "Show only selected nodes");
//   QObject::connect( _ShowOnlySelectedNodes, SIGNAL( triggered(bool) )
//     , this, SLOT( ShowOnlySelectedNodes(bool) ) );
// 
//   QAction* _ToggleSelectedVisibility 
//     = m_NodeMenu->addAction(QIcon(":/datamanager/data-type-image-mask-invisible-24.png")
//     , "Toggle visibility of selected nodes");
//   QObject::connect( _ToggleSelectedVisibility, SIGNAL( triggered(bool) )
//     , this, SLOT( ToggleVisibilityOfSelectedNodes(bool) ) );
// 
//   QAction* _ActionShowInfoDialog 
//     = m_NodeMenu->addAction(QIcon(":/datamanager/show-data-info.png")
//     , "Show additional information for selected nodes");
//   QObject::connect( _ActionShowInfoDialog, SIGNAL( triggered(bool) )
//     , this, SLOT( ShowInfoDialogForSelectedNodes(bool) ) );

//   QToolBar* _NodeToolbar = new QToolBar;
//   _NodeToolbar->setIconSize(QSize(16, 16));
//   _NodeToolbar->addAction(_SaveAction);
//   _NodeToolbar->addAction(_RemoveAction);
//   _NodeToolbar->addAction(_ReinitAction);
//   _NodeToolbar->addAction(_ShowOnlySelectedNodes);
//   _NodeToolbar->addAction(_ToggleSelectedVisibility);
//   _NodeToolbar->addAction(_ActionShowInfoDialog);

  QGridLayout* _ParentLayout = new QGridLayout;
  _ParentLayout->addWidget(m_NodeTreeView, 0, 0);

  m_Parent->setLayout(_ParentLayout);

  // call preferences changed to enable initial single click editing or not
  this->OnPreferencesChanged(m_DataManagerPreferencesNode.GetPointer());

  m_SelectionListener = new cherry::SelectionChangedAdapter<QmitkDataManagerView>
    (this, &QmitkDataManagerView::SelectionChanged);
}


void QmitkDataManagerView::OnPreferencesChanged(const cherry::ICherryPreferences* prefs )
{
  if(prefs->GetBool("Single click property editing", true))
  {
    m_NodeTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | 
      QAbstractItemView::EditKeyPressed);
  }
  else
  {
    m_NodeTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  }
}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  QModelIndex selected = m_NodeTreeView->indexAt ( pos );
  mitk::DataTreeNode::Pointer node = m_NodeTreeModel->GetNode(selected);
  std::vector<mitk::DataTreeNode*> selectedNodes = this->GetSelectedNodes();

  if(!selectedNodes.empty())
  {
    m_NodeMenu->clear();
    QList<QAction*> actions;
    if(selectedNodes.size() == 1)
    {
      actions = QmitkNodeDescriptorManager::GetInstance()->GetActions(node);

      for(QList<QAction*>::iterator it = actions.begin(); it != actions.end(); ++it)
      {
        (*it)->setData(QVariant::fromValue(node.GetPointer()));
      }
    }
    else
      actions = QmitkNodeDescriptorManager::GetInstance()->GetActions(selectedNodes);

    m_NodeMenu->addActions(actions);
    m_NodeMenu->popup(QCursor::pos());
  }
}

void QmitkDataManagerView::OpactiyChanged(int value)
{
  mitk::DataTreeNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    float opacity = static_cast<float>(value)/100.0f;
    node->SetFloatProperty("opacity", opacity);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  LOG_INFO << "slider changed";
}

void QmitkDataManagerView::OpactiyActionChanged()
{
  mitk::DataTreeNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    float opacity = 0.0;
    if(node->GetFloatProperty("opacity", opacity))
    {
      m_OpacitySlider->setValue(static_cast<int>(opacity*100));
    }
  }
  LOG_INFO << "changed";
}

void QmitkDataManagerView::SaveSelectedNodes( bool checked /*= false */ )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTreeView->selectionModel()->selectedRows();

  mitk::DataTreeNode* node = 0;
  unsigned int indexesOfSelectedRowsSize = indexesOfSelectedRows.size();
  for (unsigned int i = 0; i<indexesOfSelectedRowsSize; ++i)
  {
    node = m_NodeTreeModel->GetNode(indexesOfSelectedRows.at(i));
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
  QModelIndexList indexesOfSelectedRows = m_NodeTreeView->selectionModel()->selectedRows();
  std::vector<mitk::DataTreeNode*> selectedNodes;

  mitk::DataTreeNode* node = 0;
  QString question = tr("Do you really want to delete ");

  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = m_NodeTreeModel->GetNode(*it);
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
  std::vector<mitk::DataTreeNode*> nodes = m_NodeTreeModel->GetNodeSet();

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
  std::vector<mitk::DataTreeNode*> allNodes = m_NodeTreeModel->GetNodeSet();
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
  QModelIndexList indexesOfSelectedRows = m_NodeTreeView->selectionModel()->selectedRows();
  std::vector<mitk::DataTreeNode*> selectedNodes;

  mitk::DataTreeNode* node = 0;
  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = 0;
    node = m_NodeTreeModel->GetNode(*it);
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node != 0 )
      selectedNodes.push_back(node);
  }

  return selectedNodes;
}

void QmitkDataManagerView::SelectionChanged( cherry::IWorkbenchPart::Pointer part , cherry::ISelection::ConstPointer selection )
{
  mitk::DataTreeNodeSelection::ConstPointer _DataTreeNodeSelection 
    = selection.Cast<const mitk::DataTreeNodeSelection>();

  if(_DataTreeNodeSelection.IsNotNull())
  {
    std::vector<mitk::DataTreeNode*> selectedNodes;
    mitk::DataTreeNodeObject* _DataTreeNodeObject = 0;
    mitk::DataTreeNode* _DataTreeNode = 0;

    m_NodeTreeView->selectionModel()->clear();
    for(mitk::DataTreeNodeSelection::iterator it = _DataTreeNodeSelection->Begin();
      it != _DataTreeNodeSelection->End(); ++it)
    {
      _DataTreeNodeObject = dynamic_cast<mitk::DataTreeNodeObject*>((*it).GetPointer());
      if(_DataTreeNodeObject)
        _DataTreeNode = _DataTreeNodeObject->GetDataTreeNode();
      QModelIndex treeIndex = m_NodeTreeModel->GetIndex(_DataTreeNode);
      if(treeIndex.isValid())
        m_NodeTreeView->selectionModel()->select(treeIndex, QItemSelectionModel::Select | QItemSelectionModel::Current);
    }
  }
}

void QmitkDataManagerView::OtsuFilter( bool checked /*= false */ )
{
  std::vector<mitk::DataTreeNode*> selectedNodes = this->GetSelectedNodes();

  mitk::DataTreeNode* _DataTreeNode = 0;
  mitk::Image::Pointer mitkImage = 0;
  for (std::vector<mitk::DataTreeNode*>::iterator it = selectedNodes.begin()
    ; it != selectedNodes.end(); it++)
  {
    _DataTreeNode = *it;
    mitkImage = dynamic_cast<mitk::Image*>( _DataTreeNode->GetData() );

    if(mitkImage.IsNull())
      continue;

    try
    {
      // get selected mitk image
      const unsigned short dim = 3;
      typedef int InputPixelType;
      typedef unsigned char OutputPixelType;

      typedef itk::Image< InputPixelType, dim > InputImageType;
      typedef itk::Image< OutputPixelType, dim > OutputImageType;

      typedef itk::OtsuThresholdImageFilter< InputImageType, OutputImageType > FilterType;
      FilterType::Pointer filter = FilterType::New();

      filter->SetOutsideValue( 1 );
      filter->SetInsideValue( 0 );

      InputImageType::Pointer itkImage;
      mitk::CastToItkImage(mitkImage, itkImage);

      filter->SetInput( itkImage );

      filter->Update();

      mitk::DataTreeNode::Pointer resultNode = mitk::DataTreeNode::New();
      std::string nameOfResultImage = _DataTreeNode->GetName();
      nameOfResultImage.append("Otsu");
      resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
      resultNode->SetProperty("binary", mitk::BoolProperty::New(true) );
      resultNode->SetData( mitk::ImportItkImage ( filter->GetOutput() ) );

      this->GetDataStorage()->Add(resultNode, _DataTreeNode);

    }
    catch( std::exception& err )
    {
      LOG_ERROR(this->GetClassName()) << err.what();
    }

  }
}