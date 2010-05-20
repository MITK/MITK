/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkDataManagerView.h"

#include <itkOtsuThresholdImageFilter.h>

//# Own Includes
//## mitk
#include "mitkDataStorageEditorInput.h"
#include "mitkIDataStorageReference.h"
#include "mitkNodePredicateDataType.h"
#include "mitkCoreObjectFactory.h"
#include "mitkPACSPlugin.h"
#include "mitkDataNodeFactory.h"
#include "mitkColorProperty.h"
#include "mitkCommon.h"
#include "mitkDelegateManager.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateProperty.h"
#include "mitkEnumerationProperty.h"
#include "mitkProperties.h"
#include <mitkNodePredicateAND.h>
#include <mitkITKImageImport.h>
#include <mitkIDataStorageService.h>
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
//## Berry
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

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
#include <QColor>
#include <QColorDialog>
#include <QSizePolicy>
#include "mitkDataNodeObject.h"

const std::string QmitkDataManagerView::VIEW_ID = "org.mitk.views.datamanager";

QmitkDataManagerView::QmitkDataManagerView()
{
}


QmitkDataManagerView::~QmitkDataManagerView()
{
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemoveSelectionListener(m_SelectionListener);
}

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  m_Parent = parent;
  //# Preferences
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  //# GUI
  m_NodeTreeModel = new QmitkDataStorageTreeModel(this->GetDataStorage());
  m_NodeTreeModel->setParent( parent );

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
  m_NodeTreeView->installEventFilter(new QmitkNodeTableViewKeyFilter(this));
  QObject::connect( m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );
  QObject::connect( m_NodeTreeModel, SIGNAL(rowsInserted (const QModelIndex&, int, int))
    , this, SLOT(NodeTreeViewRowsInserted ( const QModelIndex&, int, int )) );
  QObject::connect( m_NodeTreeView->selectionModel()
    , SIGNAL( selectionChanged ( const QItemSelection &, const QItemSelection & ) )
    , this
    , SLOT( NodeSelectionChanged ( const QItemSelection &, const QItemSelection & ) ) );

  //# m_NodeMenu
  m_NodeMenu = new QMenu(m_NodeTreeView);

  //# m_SelectionProvider
  m_SelectionProvider = new QmitkDataNodeSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_NodeTreeView->selectionModel());
  this->GetSite()->SetSelectionProvider(m_SelectionProvider);

  // # Actions
  QmitkNodeDescriptor* unknownDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataNodeDescriptor();

  QmitkNodeDescriptor* imageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  QmitkNodeDescriptor* surfaceDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Surface");

  m_GlobalReinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), "Global Reinit", this);
  QObject::connect( m_GlobalReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( GlobalReinit(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_GlobalReinitAction);

  m_SaveAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Save_48.png"), "Save...", this);
  QObject::connect( m_SaveAction, SIGNAL( triggered(bool) )
    , this, SLOT( SaveSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_SaveAction);

  m_RemoveAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Remove_48.png"), "Remove", this);
  QObject::connect( m_RemoveAction, SIGNAL( triggered(bool) )
    , this, SLOT( RemoveSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_RemoveAction);

  m_ReinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), "Reinit", this);
  QObject::connect( m_ReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ReinitSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_ReinitAction);

  m_OpacitySlider = new QSlider;
  m_OpacitySlider->setMinimum(0);
  m_OpacitySlider->setMaximum(100);
  m_OpacitySlider->setOrientation(Qt::Horizontal);
  QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
    , this, SLOT( OpacityChanged(int) ) );

  QLabel* _OpacityLabel = new QLabel("Opacity: ");
  QHBoxLayout* _OpacityWidgetLayout = new QHBoxLayout;
  _OpacityWidgetLayout->setContentsMargins(4,4,4,4);
  _OpacityWidgetLayout->addWidget(_OpacityLabel);
  _OpacityWidgetLayout->addWidget(m_OpacitySlider);
  QWidget* _OpacityWidget = new QWidget;
  _OpacityWidget->setLayout(_OpacityWidgetLayout);

  m_OpacityAction = new QWidgetAction(this);
  m_OpacityAction->setDefaultWidget(_OpacityWidget);
  QObject::connect( m_OpacityAction, SIGNAL( changed() )
    , this, SLOT( OpacityActionChanged() ) );
  unknownDataNodeDescriptor->AddAction(m_OpacityAction, false);

  m_ColorButton = new QPushButton;
  m_ColorButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  //m_ColorButton->setText("Change color");
  QObject::connect( m_ColorButton, SIGNAL( clicked() )
    , this, SLOT( ColorChanged() ) );

  QLabel* _ColorLabel = new QLabel("Color: ");
  _ColorLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  QHBoxLayout* _ColorWidgetLayout = new QHBoxLayout;
  _ColorWidgetLayout->setContentsMargins(4,4,4,4);
  _ColorWidgetLayout->addWidget(_ColorLabel);
  _ColorWidgetLayout->addWidget(m_ColorButton);
  QWidget* _ColorWidget = new QWidget;
  _ColorWidget->setLayout(_ColorWidgetLayout);

  m_ColorAction = new QWidgetAction(this);
  m_ColorAction->setDefaultWidget(_ColorWidget);
  QObject::connect( m_ColorAction, SIGNAL( changed() )
    , this, SLOT( ColorActionChanged() ) );
  unknownDataNodeDescriptor->AddAction(m_ColorAction, false);

  m_TextureInterpolation = new QAction("Texture Interpolation", this);
  m_TextureInterpolation->setCheckable ( true );
  QObject::connect( m_TextureInterpolation, SIGNAL( changed() )
    , this, SLOT( TextureInterpolationChanged() ) );
  QObject::connect( m_TextureInterpolation, SIGNAL( toggled(bool) )
    , this, SLOT( TextureInterpolationToggled(bool) ) );
  imageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);

  m_SurfaceRepresentation = new QAction("Surface Representation", this);
  m_SurfaceRepresentation->setMenu(new QMenu);

  QObject::connect( m_SurfaceRepresentation->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( SurfaceRepresentationMenuAboutToShow() ) );
  surfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentation, false);

  m_ShowOnlySelectedNodes
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowSelectedNode_48.png")
    , "Show only selected nodes", this);
  QObject::connect( m_ShowOnlySelectedNodes, SIGNAL( triggered(bool) )
    , this, SLOT( ShowOnlySelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_ShowOnlySelectedNodes);

  m_ToggleSelectedVisibility
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/InvertShowSelectedNode_48.png")
    , "Toggle visibility", this);
  QObject::connect( m_ToggleSelectedVisibility, SIGNAL( triggered(bool) )
    , this, SLOT( ToggleVisibilityOfSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_ToggleSelectedVisibility);

  m_ActionShowInfoDialog
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowDataInfo_48.png")
    , "Details...", this);
  QObject::connect( m_ActionShowInfoDialog, SIGNAL( triggered(bool) )
    , this, SLOT( ShowInfoDialogForSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(m_ActionShowInfoDialog);

  m_OtsuFilterAction = new QAction("Apply Otsu Filter", this);
  QObject::connect( m_OtsuFilterAction, SIGNAL( triggered(bool) )
    , this, SLOT( OtsuFilter(bool) ) );
  // Otsu filter does not work properly, remove it temporarily
  // imageDataNodeDescriptor->AddAction(m_OtsuFilterAction);

  QGridLayout* _DndFrameWidgetLayout = new QGridLayout;
  _DndFrameWidgetLayout->addWidget(m_NodeTreeView, 0, 0);
  _DndFrameWidgetLayout->setContentsMargins(0,0,0,0);

  m_DndFrameWidget = new QmitkDnDFrameWidget(m_Parent);
  m_DndFrameWidget->setLayout(_DndFrameWidgetLayout);

  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->addWidget(m_DndFrameWidget);
  layout->setContentsMargins(0,0,0,0);

  m_Parent->setLayout(layout);

  m_SelectionListener = new berry::SelectionChangedAdapter<QmitkDataManagerView>
    (this, &QmitkDataManagerView::SelectionChanged);

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  s->AddSelectionListener(m_SelectionListener);

}

void QmitkDataManagerView::SetFocus()
{
}

mitk::DataStorage::Pointer QmitkDataManagerView::GetDataStorage() const
{
  mitk::IDataStorageService::Pointer service =
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

  if (service.IsNotNull())
  {
    return service->GetDefaultDataStorage()->GetDataStorage();
  }

  return 0;
}

void QmitkDataManagerView::OnPreferencesChanged(const berry::IBerryPreferences* )
{
}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  QModelIndex selected = m_NodeTreeView->indexAt ( pos );
  mitk::DataNode::Pointer node = m_NodeTreeModel->GetNode(selected);
  std::vector<mitk::DataNode*> selectedNodes = this->GetSelectedNodes();

  if(!selectedNodes.empty())
  {
    m_NodeMenu->clear();
    QList<QAction*> actions;
    if(selectedNodes.size() == 1 )
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

void QmitkDataManagerView::OpacityChanged(int value)
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    float opacity = static_cast<float>(value)/100.0f;
    node->SetFloatProperty("opacity", opacity);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  MITK_INFO << "slider changed";
}

void QmitkDataManagerView::OpacityActionChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    float opacity = 0.0;
    if(node->GetFloatProperty("opacity", opacity))
    {
      m_OpacitySlider->setValue(static_cast<int>(opacity*100));
    }
  }
  MITK_INFO << "changed";
}

void QmitkDataManagerView::ColorChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    QColor color = QColorDialog::getColor();
    m_ColorButton->setAutoFillBackground(true);
    node->SetProperty("color",mitk::ColorProperty::New(color.red()/255.0,color.green()/255.0,color.blue()/255.0));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  MITK_INFO << "slider changed";
}

void QmitkDataManagerView::ColorActionChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    mitk::Color color;
    mitk::ColorProperty::Pointer colorProp;
    node->GetProperty(colorProp,"color");
    if(colorProp.IsNull())
      return;
    color = colorProp->GetValue();

    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(color[0]*255));
    styleSheet.append(",");
    styleSheet.append(QString::number(color[1]*255));
    styleSheet.append(",");
    styleSheet.append(QString::number(color[2]*255));
    styleSheet.append(")");
    m_ColorButton->setStyleSheet(styleSheet);
  }
  MITK_INFO << "changed";
}

void QmitkDataManagerView::TextureInterpolationChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    bool textureInterpolation = false;
    node->GetBoolProperty("texture interpolation", textureInterpolation);
    m_TextureInterpolation->setChecked(textureInterpolation);
  }
}

void QmitkDataManagerView::TextureInterpolationToggled( bool checked )
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(node)
  {
    node->SetBoolProperty("texture interpolation", checked);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

}

void QmitkDataManagerView::SurfaceRepresentationMenuAboutToShow()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(!node)
    return;

  mitk::EnumerationProperty* representationProp =
      dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("material.representation"));
  if(!representationProp)
    return;

  // clear menu
  m_SurfaceRepresentation->menu()->clear();
  QAction* tmp;

  // create menu entries
  for(mitk::EnumerationProperty::EnumConstIterator it=representationProp->Begin(); it!=representationProp->End()
    ; it++)
  {
    tmp = m_SurfaceRepresentation->menu()->addAction(QString::fromStdString(it->second));
    tmp->setCheckable(true);

    if(it->second == representationProp->GetValueAsString())
    {
      tmp->setChecked(true);
    }

    QObject::connect( tmp, SIGNAL( triggered(bool) )
      , this, SLOT( SurfaceRepresentationActionToggled(bool) ) );
  }
}

void QmitkDataManagerView::SurfaceRepresentationActionToggled( bool checked )
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_NodeTreeView->selectionModel()->currentIndex());
  if(!node)
    return;

  mitk::EnumerationProperty* representationProp =
      dynamic_cast<mitk::EnumerationProperty*> (node->GetProperty("material.representation"));
  if(!representationProp)
    return;

  QAction* senderAction = qobject_cast<QAction*> ( QObject::sender() );

  if(!senderAction)
    return;

  std::string activatedItem = senderAction->text().toStdString();

  if ( activatedItem != representationProp->GetValueAsString() )
  {
    if ( representationProp->IsValidEnumerationValue( activatedItem ) )
    {
      representationProp->SetValue( activatedItem );
      representationProp->InvokeEvent( itk::ModifiedEvent() );
      representationProp->Modified();

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

}

void QmitkDataManagerView::SaveSelectedNodes( bool )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTreeView->selectionModel()->selectedRows();

  mitk::DataNode* node = 0;
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

void QmitkDataManagerView::ReinitSelectedNodes( bool )
{
  this->ReinitMultiWidgetEditor();
  std::vector<mitk::DataNode*> selectedNodes = this->GetSelectedNodes();

  mitk::DataNode* node = 0;
  for (std::vector<mitk::DataNode*>::iterator it = selectedNodes.begin()
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

void QmitkDataManagerView::RemoveSelectedNodes( bool )
{
  QModelIndexList indexesOfSelectedRows = m_NodeTreeView->selectionModel()->selectedRows();
  std::vector<mitk::DataNode*> selectedNodes;

  mitk::DataNode* node = 0;
  QString question = tr("Do you really want to remove ");

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
  question.append(" from data storage?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( m_Parent
    , tr("DataManager")
    , question
    , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if(answerButton == QMessageBox::Yes)
  {
    for (std::vector<mitk::DataNode*>::iterator it = selectedNodes.begin()
      ; it != selectedNodes.end(); it++)
    {
      node = *it;
      this->GetDataStorage()->Remove(node);
      this->GlobalReinit(false);

    }
  }
}

void QmitkDataManagerView::MakeAllNodesInvisible( bool )
{
  std::vector<mitk::DataNode*> nodes = m_NodeTreeModel->GetNodeSet();

  for (std::vector<mitk::DataNode*>::iterator it = nodes.begin()
    ; it != nodes.end(); it++)
  {
    (*it)->SetVisibility(false);
  }
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowOnlySelectedNodes( bool )
{
  std::vector<mitk::DataNode*> selectedNodes = this->GetSelectedNodes();
  std::vector<mitk::DataNode*> allNodes = m_NodeTreeModel->GetNodeSet();
  mitk::DataNode* node = 0;

  for (std::vector<mitk::DataNode*>::iterator it = allNodes.begin()
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

void QmitkDataManagerView::ToggleVisibilityOfSelectedNodes( bool )
{
  std::vector<mitk::DataNode*> selectedNodes = this->GetSelectedNodes();

  bool isVisible = false;
  mitk::DataNode* node = 0;
  for (std::vector<mitk::DataNode*>::iterator it = selectedNodes.begin()
    ; it != selectedNodes.end(); it++)
  {
    isVisible = false;
    node = *it;
    node->GetBoolProperty("visible", isVisible);
    node->SetVisibility(!isVisible);
  }
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowInfoDialogForSelectedNodes( bool )
{
  std::vector<mitk::DataNode*> selectedNodes = this->GetSelectedNodes();

  QmitkInfoDialog _QmitkInfoDialog(selectedNodes, this->m_Parent);
  _QmitkInfoDialog.exec();
}

void QmitkDataManagerView::Load( bool )
{
  QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Load data", "", mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());
  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    FileOpen((*it).toAscii(), 0);
  }
}

void QmitkDataManagerView::FileOpen( const char * fileName, mitk::DataNode* parentNode )
{
  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();

  try
  {
    factory->SetFileName( fileName );

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    factory->Update();

    for ( unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i )
    {
      mitk::DataNode::Pointer node = factory->GetOutput( i );
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

void QmitkDataManagerView::GlobalReinit( bool )
{
  this->ReinitMultiWidgetEditor();
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

std::vector<mitk::DataNode*> QmitkDataManagerView::GetSelectedNodes() const
{
  QModelIndexList indexesOfSelectedRows = m_NodeTreeView->selectionModel()->selectedRows();
  std::vector<mitk::DataNode*> selectedNodes;

  mitk::DataNode* node = 0;
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

void QmitkDataManagerView::SelectionChanged( berry::IWorkbenchPart::Pointer part , berry::ISelection::ConstPointer selection )
{
  if(part.GetPointer() == this)
    return;
  mitk::DataNodeSelection::ConstPointer _DataNodeSelection
    = selection.Cast<const mitk::DataNodeSelection>();

  if(_DataNodeSelection.IsNull())
    return;

  std::vector<mitk::DataNode*> selectedNodes;
  mitk::DataNodeObject* _DataNodeObject = 0;
  mitk::DataNode* _DataNode = 0;
  QItemSelection newSelection;

  m_NodeTreeView->selectionModel()->reset();

  for(mitk::DataNodeSelection::iterator it = _DataNodeSelection->Begin();
    it != _DataNodeSelection->End(); ++it)
  {
    _DataNodeObject = dynamic_cast<mitk::DataNodeObject*>((*it).GetPointer());
    if(_DataNodeObject)
      _DataNode = _DataNodeObject->GetDataNode();
    QModelIndex treeIndex = m_NodeTreeModel->GetIndex(_DataNode);
    if(treeIndex.isValid())
      newSelection.select(treeIndex, treeIndex);
  }
  m_NodeTreeView->selectionModel()->select(newSelection, QItemSelectionModel::SelectCurrent);
}

void QmitkDataManagerView::OtsuFilter( bool )
{
  std::vector<mitk::DataNode*> selectedNodes = this->GetSelectedNodes();

  mitk::DataNode* _DataNode = 0;
  mitk::Image::Pointer mitkImage = 0;
  for (std::vector<mitk::DataNode*>::iterator it = selectedNodes.begin()
    ; it != selectedNodes.end(); it++)
  {
    _DataNode = *it;
    mitkImage = dynamic_cast<mitk::Image*>( _DataNode->GetData() );

    if(mitkImage.IsNull())
      continue;

    try
    {
      // get selected mitk image
      const unsigned short dim = 3;
      typedef short InputPixelType;
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

      mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
      std::string nameOfResultImage = _DataNode->GetName();
      nameOfResultImage.append("Otsu");
      resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
      resultNode->SetProperty("binary", mitk::BoolProperty::New(true) );
      resultNode->SetData( mitk::ImportItkImage ( filter->GetOutput() ) );

      this->GetDataStorage()->Add(resultNode, _DataNode);

    }
    catch( std::exception& err )
    {
      MITK_ERROR(this->GetClassName()) << err.what();
    }

  }
}

void QmitkDataManagerView::NodeTreeViewRowsInserted( const QModelIndex & parent, int, int )
{
  m_NodeTreeView->setExpanded(parent, true);
  /*std::vector<mitk::DataNode*> nodes = m_NodeTreeModel->GetNodeSet();
  if(nodes.size() == 1)
  {
    QModelIndex treeIndex = m_NodeTreeModel->GetIndex(nodes.front());
    m_NodeTreeView->selectionModel()->select(treeIndex, QItemSelectionModel::SelectCurrent);
  }*/
}

void QmitkDataManagerView::NodeSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
  std::vector<mitk::DataNode*> nodes = m_NodeTreeModel->GetNodeSet();
  mitk::DataNode* node = 0;

  for (std::vector<mitk::DataNode*>::iterator it = nodes.begin()
    ; it != nodes.end(); it++)
  {
    node = *it;
    if ( node )
      node->SetBoolProperty("selected", false);
  }

  nodes.clear();
  nodes = this->GetSelectedNodes();

  for (std::vector<mitk::DataNode*>::iterator it = nodes.begin()
    ; it != nodes.end(); it++)
  {
    node = *it;
    if ( node )
      node->SetBoolProperty("selected", true);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ReinitMultiWidgetEditor()
{
  berry::IEditorPart::Pointer editor =
    this->GetSite()->GetPage()->GetActiveEditor();

  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNull())
  {
    mitk::IDataStorageService::Pointer service =
      berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

    mitk::IDataStorageReference::Pointer DataStorageReference;
    if (service.IsNotNull())
    {
      DataStorageReference = service->GetDefaultDataStorage();
    }
    mitk::DataStorageEditorInput::Pointer editorInput;
    editorInput = new mitk::DataStorageEditorInput(DataStorageReference);
    // open a new multi-widget editor, but do not give it the focus
    berry::IEditorPart::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
  }
}