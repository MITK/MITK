/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkDataManagerView.h"

//# Own Includes
//## mitk
#include "mitkDataStorageEditorInput.h"
#include "mitkIDataStorageReference.h"
#include "mitkNodePredicateDataType.h"
#include "mitkCoreObjectFactory.h"
#include "mitkColorProperty.h"
#include "mitkCommon.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"
#include "mitkEnumerationProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkProperties.h"
#include <mitkNodePredicateAnd.h>
#include <mitkITKImageImport.h>
#include <mitkIDataStorageService.h>
#include <mitkIRenderingManager.h>
#include <mitkImageCast.h>
//## Qmitk
#include <QmitkDnDFrameWidget.h>
#include <QmitkIOUtil.h>
#include <QmitkDataStorageTreeModel.h>
#include <QmitkCustomVariants.h>
#include <QmitkFileSaveAction.h>
#include <QmitkDataStorageFilterProxyModel.h>
#include <QmitkNumberPropertySlider.h>
#include "src/internal/QmitkNodeTableViewKeyFilter.h"
#include "src/internal/QmitkInfoDialog.h"
#include "src/internal/QmitkDataManagerItemDelegate.h"
//## Berry
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIEditorRegistry.h>

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
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QKeyEvent>
#include <QColor>
#include <QColorDialog>
#include <QSizePolicy>
#include <QSortFilterProxyModel>
#include <QSignalMapper>

#include "mitkDataNodeObject.h"
#include "mitkIContextMenuAction.h"
#include "berryIExtensionRegistry.h"
#include "mitkRenderingModeProperty.h"

const QString QmitkDataManagerView::VIEW_ID = "org.mitk.views.datamanager";

QString QmitkDataManagerView::TR_GLOBAL_REINIT = QAction::tr("Global Reinit");
QString QmitkDataManagerView::TR_REMOVE = QAction::tr("Remove");
QString QmitkDataManagerView::TR_REINIT = QAction::tr("Reinit");
QString QmitkDataManagerView::TR_OPACITY = QLabel::tr("Opacity: ");
QString QmitkDataManagerView::TR_COLOR = QLabel::tr("Color: ");
QString QmitkDataManagerView::TR_TEXTURE_INTERPOLATION = QAction::tr("Texture Interpolation");
QString QmitkDataManagerView::TR_SURFACE_REPRESENTATION = QAction::tr("Surface Representation");
QString QmitkDataManagerView::TR_SHOW_ONLY_SELECTED_NOTES = QAction::tr("Show only selected nodes");
QString QmitkDataManagerView::TR_TOGGLE_VISIBILITY = QAction::tr("Toggle visibility");
QString QmitkDataManagerView::TR_DETAILS = QAction::tr("Details...");
QString QmitkDataManagerView::TR_COLORMAP = QAction::tr("Colormap");

const QString QmitkDataManagerView::contextMenuExtTranslateNames[] = { 
  QAction::tr("Create polygon model"),   
  QAction::tr("Create smoothed polygon model"),
  QAction::tr("Autocrop")
};

QmitkDataManagerView::QmitkDataManagerView()
    : m_GlobalReinitOnNodeDelete(true),
      m_ItemDelegate(NULL)
{

  contextMenuExtSrcNames << "Create polygon model" << "Create smoothed polygon model" << "Autocrop";
}

QmitkDataManagerView::~QmitkDataManagerView()
{
  //Remove all registered actions from each descriptor
  for (std::vector< std::pair< QmitkNodeDescriptor*, QAction* > >::iterator it = m_DescriptorActionList.begin();it != m_DescriptorActionList.end(); it++)
  {
    // first== the NodeDescriptor; second== the registered QAction
    (it->first)->RemoveAction(it->second);
  }
}

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  m_CurrentRowCount = 0;
  m_Parent = parent;
  //# Preferences
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(VIEW_ID))
        .Cast<berry::IBerryPreferences>();
  assert( prefs );
  prefs->OnChanged.AddListener( berry::MessageDelegate1<QmitkDataManagerView
    , const berry::IBerryPreferences*>( this
      , &QmitkDataManagerView::OnPreferencesChanged ) );

  //# GUI
  m_NodeTreeModel = new QmitkDataStorageTreeModel(this->GetDataStorage());
  m_NodeTreeModel->setParent( parent );
  m_NodeTreeModel->SetPlaceNewNodesOnTop(
      prefs->GetBool("Place new nodes on top", true) );
  m_NodeTreeModel->SetAllowHierarchyChange(
    prefs->GetBool("Allow changing of parent node", false));
  m_SurfaceDecimation = prefs->GetBool("Use surface decimation", false);
  // Prepare filters
  m_HelperObjectFilterPredicate = mitk::NodePredicateOr::New(
   mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)),
   mitk::NodePredicateProperty::New("hidden object", mitk::BoolProperty::New(true)));
  m_NodeWithNoDataFilterPredicate = mitk::NodePredicateData::New(0);

  m_FilterModel = new QmitkDataStorageFilterProxyModel();
  m_FilterModel->setSourceModel(m_NodeTreeModel);
  m_FilterModel->AddFilterPredicate(m_HelperObjectFilterPredicate);
  m_FilterModel->AddFilterPredicate(m_NodeWithNoDataFilterPredicate);

  //# Tree View (experimental)
  m_NodeTreeView = new QTreeView;
  m_NodeTreeView->setHeaderHidden(true);
  m_NodeTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  m_NodeTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_NodeTreeView->setAlternatingRowColors(true);
  m_NodeTreeView->setDragEnabled(true);
  m_NodeTreeView->setDropIndicatorShown(true);
  m_NodeTreeView->setAcceptDrops(true);
  m_NodeTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_NodeTreeView->setModel(m_FilterModel);
  m_NodeTreeView->setTextElideMode(Qt::ElideMiddle);
  m_NodeTreeView->installEventFilter(new QmitkNodeTableViewKeyFilter(this));

  m_ItemDelegate = new QmitkDataManagerItemDelegate(m_NodeTreeView);
  m_NodeTreeView->setItemDelegate(m_ItemDelegate);

  QObject::connect( m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );
  QObject::connect( m_NodeTreeModel, SIGNAL(rowsInserted (const QModelIndex&, int, int))
    , this, SLOT(NodeTreeViewRowsInserted ( const QModelIndex&, int, int )) );
  QObject::connect( m_NodeTreeModel, SIGNAL(rowsRemoved (const QModelIndex&, int, int))
    , this, SLOT(NodeTreeViewRowsRemoved( const QModelIndex&, int, int )) );
  QObject::connect( m_NodeTreeView->selectionModel()
    , SIGNAL( selectionChanged ( const QItemSelection &, const QItemSelection & ) )
    , this
    , SLOT( NodeSelectionChanged ( const QItemSelection &, const QItemSelection & ) ) );

  //# m_NodeMenu
  m_NodeMenu = new QMenu(m_NodeTreeView);

  // # Actions
  berry::IEditorRegistry* editorRegistry = berry::PlatformUI::GetWorkbench()->GetEditorRegistry();
  QList<berry::IEditorDescriptor::Pointer> editors = editorRegistry->GetEditors("*.mitk");
  if (editors.size() > 1)
  {
    m_ShowInMapper = new QSignalMapper(this);
    foreach(berry::IEditorDescriptor::Pointer descriptor, editors)
    {
      QAction* action = new QAction(descriptor->GetLabel(), this);
      m_ShowInActions << action;
      m_ShowInMapper->connect(action, SIGNAL(triggered()), m_ShowInMapper, SLOT(map()));
      m_ShowInMapper->setMapping(action, descriptor->GetId());
    }
    connect(m_ShowInMapper, SIGNAL(mapped(QString)), this, SLOT(ShowIn(QString)));
  }

  QmitkNodeDescriptor* unknownDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataNodeDescriptor();

  QmitkNodeDescriptor* imageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  auto multiComponentImageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("MultiComponentImage");

  QmitkNodeDescriptor* diffusionImageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("DiffusionImage");

  QmitkNodeDescriptor* surfaceDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Surface");

  QAction* globalReinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), TR_GLOBAL_REINIT, this);
  QObject::connect( globalReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( GlobalReinit(bool) ) );
  unknownDataNodeDescriptor->AddAction(globalReinitAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, globalReinitAction));

  QAction* saveAction = new QmitkFileSaveAction(QIcon(":/org.mitk.gui.qt.datamanager/Save_48.png"),
                                                this->GetSite()->GetWorkbenchWindow());

  unknownDataNodeDescriptor->AddAction(saveAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,saveAction));

  QAction* removeAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Remove_48.png"), TR_REMOVE, this);
  QObject::connect( removeAction, SIGNAL( triggered(bool) )
    , this, SLOT( RemoveSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(removeAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,removeAction));

  QAction* reinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), TR_REINIT, this);
  QObject::connect( reinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ReinitSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(reinitAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,reinitAction));

  // find contextMenuAction extension points and add them to the node descriptor
  berry::IExtensionRegistry* extensionPointService = berry::Platform::GetExtensionRegistry();
  QList<berry::IConfigurationElement::Pointer> cmActions(
    extensionPointService->GetConfigurationElementsFor("org.mitk.gui.qt.datamanager.contextMenuActions") );
  QList<berry::IConfigurationElement::Pointer>::iterator cmActionsIt;

  QmitkNodeDescriptor* tmpDescriptor;
  QAction* contextMenuAction;
  QVariant cmActionDataIt;
  m_ConfElements.clear();

  int i=1;
  for (cmActionsIt = cmActions.begin()
    ; cmActionsIt != cmActions.end()
    ; ++cmActionsIt)
  {
    QString cmNodeDescriptorName = (*cmActionsIt)->GetAttribute("nodeDescriptorName");
    QString cmLabel = (*cmActionsIt)->GetAttribute("label");
    QString cmClass = (*cmActionsIt)->GetAttribute("class");
    if(!cmNodeDescriptorName.isEmpty() &&
       !cmLabel.isEmpty() &&
       !cmClass.isEmpty())
    {
      QString cmIcon = (*cmActionsIt)->GetAttribute("icon");
      // create context menu entry here
      tmpDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(cmNodeDescriptorName);
      if(!tmpDescriptor)
      {
        MITK_WARN << "cannot add action \"" << cmLabel << "\" because descriptor " << cmNodeDescriptorName << " does not exist";
        continue;
      }

      QString translatedLabel = QString(cmLabel.toUtf8().constData());
      for (int i=0; i<contextMenuExtSrcNames.size(); i++) {
        if (contextMenuExtSrcNames[i].compare(QString(cmLabel.toUtf8().constData())) == 0) {
          translatedLabel  = contextMenuExtTranslateNames[i];
          break;
        }
      }

      // check if the user specified an icon attribute
      if (!cmIcon.isEmpty())
      {
        contextMenuAction = new QAction(QIcon(cmIcon), translatedLabel, parent);
      }
      else
      {
        contextMenuAction = new QAction(translatedLabel, parent);
      }

      tmpDescriptor->AddAction(contextMenuAction);
      m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(tmpDescriptor,contextMenuAction));
      m_ConfElements[contextMenuAction] = *cmActionsIt;

      cmActionDataIt.setValue<int>(i);
      contextMenuAction->setData( cmActionDataIt );
      connect( contextMenuAction, SIGNAL( triggered(bool) ) , this, SLOT( ContextMenuActionTriggered(bool) ) );
      ++i;
    }
  }

  m_OpacitySlider = new QSlider;
  m_OpacitySlider->setMinimum(0);
  m_OpacitySlider->setMaximum(100);
  m_OpacitySlider->setOrientation(Qt::Horizontal);
  QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
    , this, SLOT( OpacityChanged(int) ) );

  QLabel* _OpacityLabel = new QLabel(TR_OPACITY);
  QHBoxLayout* _OpacityWidgetLayout = new QHBoxLayout;
  _OpacityWidgetLayout->setContentsMargins(4,4,4,4);
  _OpacityWidgetLayout->addWidget(_OpacityLabel);
  _OpacityWidgetLayout->addWidget(m_OpacitySlider);
  QWidget* _OpacityWidget = new QWidget;
  _OpacityWidget->setLayout(_OpacityWidgetLayout);

  QWidgetAction* opacityAction = new QWidgetAction(this);
  opacityAction ->setDefaultWidget(_OpacityWidget);
  QObject::connect( opacityAction , SIGNAL( changed() )
    , this, SLOT( OpacityActionChanged() ) );
  unknownDataNodeDescriptor->AddAction(opacityAction , false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,opacityAction));

  m_ColorButton = new QPushButton;
  m_ColorButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  //m_ColorButton->setText("Change color");
  QObject::connect( m_ColorButton, SIGNAL( clicked() )
    , this, SLOT( ColorChanged() ) );

  QLabel* _ColorLabel = new QLabel(TR_COLOR);
  _ColorLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  QHBoxLayout* _ColorWidgetLayout = new QHBoxLayout;
  _ColorWidgetLayout->setContentsMargins(4,4,4,4);
  _ColorWidgetLayout->addWidget(_ColorLabel);
  _ColorWidgetLayout->addWidget(m_ColorButton);
  QWidget* _ColorWidget = new QWidget;
  _ColorWidget->setLayout(_ColorWidgetLayout);

  QWidgetAction* colorAction = new QWidgetAction(this);
  colorAction->setDefaultWidget(_ColorWidget);
  QObject::connect( colorAction, SIGNAL( changed() )
    , this, SLOT( ColorActionChanged() ) );
  unknownDataNodeDescriptor->AddAction(colorAction, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,colorAction));

  m_ComponentSlider = new QmitkNumberPropertySlider;
  m_ComponentSlider->setOrientation(Qt::Horizontal);
  //QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
  //  , this, SLOT( OpacityChanged(int) ) );

  QLabel* _ComponentLabel = new QLabel("Component: ");
  QHBoxLayout* _ComponentWidgetLayout = new QHBoxLayout;
  _ComponentWidgetLayout->setContentsMargins(4,4,4,4);
  _ComponentWidgetLayout->addWidget(_ComponentLabel);
  _ComponentWidgetLayout->addWidget(m_ComponentSlider);
  QLabel* _ComponentValueLabel = new QLabel();
  _ComponentWidgetLayout->addWidget(_ComponentValueLabel);
  connect(m_ComponentSlider, SIGNAL(valueChanged(int)), _ComponentValueLabel, SLOT(setNum(int)));
  QWidget* _ComponentWidget = new QWidget;
  _ComponentWidget->setLayout(_ComponentWidgetLayout);

  QWidgetAction* componentAction = new QWidgetAction(this);
  componentAction->setDefaultWidget(_ComponentWidget);
  QObject::connect( componentAction , SIGNAL( changed() )
    , this, SLOT( ComponentActionChanged() ) );
  multiComponentImageDataNodeDescriptor->AddAction(componentAction, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(multiComponentImageDataNodeDescriptor,componentAction));
  if (diffusionImageDataNodeDescriptor!=NULL)
  {
      diffusionImageDataNodeDescriptor->AddAction(componentAction, false);
      m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(diffusionImageDataNodeDescriptor,componentAction));
  }

  m_TextureInterpolation = new QAction(TR_TEXTURE_INTERPOLATION, this);
  m_TextureInterpolation->setCheckable ( true );
  QObject::connect( m_TextureInterpolation, SIGNAL( changed() )
    , this, SLOT( TextureInterpolationChanged() ) );
  QObject::connect( m_TextureInterpolation, SIGNAL( toggled(bool) )
    , this, SLOT( TextureInterpolationToggled(bool) ) );
  imageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor,m_TextureInterpolation));
  if (diffusionImageDataNodeDescriptor!=NULL)
  {
      diffusionImageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
      m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(diffusionImageDataNodeDescriptor,m_TextureInterpolation));
  }

  m_ColormapAction = new QAction(TR_COLORMAP, this);
  m_ColormapAction->setMenu(new QMenu);
  QObject::connect( m_ColormapAction->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( ColormapMenuAboutToShow() ) );
  imageDataNodeDescriptor->AddAction(m_ColormapAction, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor, m_ColormapAction));
  if (diffusionImageDataNodeDescriptor!=NULL)
  {
      diffusionImageDataNodeDescriptor->AddAction(m_ColormapAction, false);
      m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(diffusionImageDataNodeDescriptor, m_ColormapAction));
  }

  m_SurfaceRepresentation = new QAction(TR_SURFACE_REPRESENTATION, this);

  m_SurfaceRepresentation->setMenu(new QMenu(m_NodeTreeView));

  QObject::connect( m_SurfaceRepresentation->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( SurfaceRepresentationMenuAboutToShow() ) );
  surfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentation, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(surfaceDataNodeDescriptor, m_SurfaceRepresentation));

  QAction* showOnlySelectedNodes
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowSelectedNode_48.png")
	, TR_SHOW_ONLY_SELECTED_NOTES, this);
  QObject::connect( showOnlySelectedNodes, SIGNAL( triggered(bool) )
    , this, SLOT( ShowOnlySelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(showOnlySelectedNodes);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, showOnlySelectedNodes));

  QAction* toggleSelectedVisibility
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/InvertShowSelectedNode_48.png")
	, TR_TOGGLE_VISIBILITY, this);
  QObject::connect( toggleSelectedVisibility, SIGNAL( triggered(bool) )
    , this, SLOT( ToggleVisibilityOfSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(toggleSelectedVisibility);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,toggleSelectedVisibility));

  QAction* actionShowInfoDialog
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowDataInfo_48.png")
	, TR_DETAILS, this);
  QObject::connect( actionShowInfoDialog, SIGNAL( triggered(bool) )
    , this, SLOT( ShowInfoDialogForSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(actionShowInfoDialog);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,actionShowInfoDialog));

  QGridLayout* _DndFrameWidgetLayout = new QGridLayout;
  _DndFrameWidgetLayout->addWidget(m_NodeTreeView, 0, 0);
  _DndFrameWidgetLayout->setContentsMargins(0,0,0,0);

  m_DndFrameWidget = new QmitkDnDFrameWidget(m_Parent);
  m_DndFrameWidget->setLayout(_DndFrameWidgetLayout);

  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->addWidget(m_DndFrameWidget);
  layout->setContentsMargins(0,0,0,0);

  m_Parent->setLayout(layout);
}

void QmitkDataManagerView::SetFocus()
{
}

void QmitkDataManagerView::ContextMenuActionTriggered( bool )
{
  QAction* action = qobject_cast<QAction*> ( sender() );

  std::map<QAction*, berry::IConfigurationElement::Pointer>::iterator it
    = m_ConfElements.find( action );
  if( it == m_ConfElements.end() )
  {
    MITK_WARN << "associated conf element for action " << action->text().toStdString() << " not found";
    return;
  }
  berry::IConfigurationElement::Pointer confElem = it->second;
  mitk::IContextMenuAction* contextMenuAction = confElem->CreateExecutableExtension<mitk::IContextMenuAction>("class");

  QString className = confElem->GetAttribute("class");
  QString smoothed = confElem->GetAttribute("smoothed");

  if(className == "QmitkCreatePolygonModelAction")
  {
    contextMenuAction->SetDataStorage(this->GetDataStorage());
    if(smoothed == "false")
    {
      contextMenuAction->SetSmoothed(false);
    }
    else
    {
      contextMenuAction->SetSmoothed(true);
    }
    contextMenuAction->SetDecimated(m_SurfaceDecimation);
  }
  else if(className == "QmitkStatisticsAction")
  {
    contextMenuAction->SetFunctionality(this);
  }
  else if(className == "QmitkCreateSimulationAction")
  {
    contextMenuAction->SetDataStorage(this->GetDataStorage());
  }
  contextMenuAction->Run( this->GetCurrentSelection() ); // run the action
}

void QmitkDataManagerView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  if( m_NodeTreeModel->GetPlaceNewNodesOnTopFlag() !=  prefs->GetBool("Place new nodes on top", true) )
    m_NodeTreeModel->SetPlaceNewNodesOnTop( !m_NodeTreeModel->GetPlaceNewNodesOnTopFlag() );

  bool hideHelperObjects = !prefs->GetBool("Show helper objects", false);
  if (m_FilterModel->HasFilterPredicate(m_HelperObjectFilterPredicate) != hideHelperObjects)
  {
    if (hideHelperObjects)
    {
        m_FilterModel->AddFilterPredicate(m_HelperObjectFilterPredicate);
    }
    else
    {
        m_FilterModel->RemoveFilterPredicate(m_HelperObjectFilterPredicate);
    }
  }
  bool hideNodesWithNoData = !prefs->GetBool("Show nodes containing no data", false);

  if (m_FilterModel->HasFilterPredicate(m_NodeWithNoDataFilterPredicate) != hideNodesWithNoData)
  {
    if (hideNodesWithNoData)
    {
        m_FilterModel->AddFilterPredicate(m_NodeWithNoDataFilterPredicate);
    }
    else
    {
        m_FilterModel->RemoveFilterPredicate(m_NodeWithNoDataFilterPredicate);
    }
  }

  m_GlobalReinitOnNodeDelete = prefs->GetBool("Call global reinit if node is deleted", true);

  m_NodeTreeView->expandAll();

  m_SurfaceDecimation = prefs->GetBool("Use surface decimation", false);

  m_NodeTreeModel->SetAllowHierarchyChange(
    prefs->GetBool("Allow changing of parent node", false));

  this->GlobalReinit();


}

void QmitkDataManagerView::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  QModelIndex selectedProxy = m_NodeTreeView->indexAt ( pos );
  QModelIndex selected = m_FilterModel->mapToSource(selectedProxy);
  mitk::DataNode::Pointer node = m_NodeTreeModel->GetNode(selected);
  QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelection();

  if(!selectedNodes.isEmpty())
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

    if (!m_ShowInActions.isEmpty())
    {
      QMenu* showInMenu = m_NodeMenu->addMenu("Show In");
      showInMenu->addActions(m_ShowInActions);
    }
    m_NodeMenu->addActions(actions);
    m_NodeMenu->popup(QCursor::pos());
  }
}

void QmitkDataManagerView::OpacityChanged(int value)
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  if(node)
  {
    float opacity = static_cast<float>(value)/100.0f;
    node->SetFloatProperty("opacity", opacity);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkDataManagerView::OpacityActionChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  if(node)
  {
    float opacity = 0.0;
    if(node->GetFloatProperty("opacity", opacity))
    {
      m_OpacitySlider->setValue(static_cast<int>(opacity*100));
    }
  }
}

void QmitkDataManagerView::ComponentActionChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  mitk::IntProperty* componentProperty = NULL;
  int numComponents = 0;
  if(node)
  {
    componentProperty =
        dynamic_cast<mitk::IntProperty*>(node->GetProperty("Image.Displayed Component"));
    mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
    if (img != NULL)
    {
      numComponents = img->GetPixelType().GetNumberOfComponents();
    }
  }
  if (componentProperty && numComponents > 1)
  {
    m_ComponentSlider->SetProperty(componentProperty);
    m_ComponentSlider->setMinValue(0);
    m_ComponentSlider->setMaxValue(numComponents-1);
  }
  else
  {
    m_ComponentSlider->SetProperty(static_cast<mitk::IntProperty*>(NULL));
  }
}

void QmitkDataManagerView::ColorChanged()
 {
   mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
   if(node)
   {
    mitk::Color color;
    mitk::ColorProperty::Pointer colorProp;
    node->GetProperty(colorProp,"color");
    if(colorProp.IsNull())
      return;
    color = colorProp->GetValue();
    QColor initial(color.GetRed()*255,color.GetGreen()*255,color.GetBlue()*255);
    QColor qcolor = QColorDialog::getColor(initial,0,QString("Change color"));
    if (!qcolor.isValid())
      return;
    m_ColorButton->setAutoFillBackground(true);
    node->SetProperty("color",mitk::ColorProperty::New(qcolor.red()/255.0,qcolor.green()/255.0,qcolor.blue()/255.0));
    if (node->GetProperty("binaryimage.selectedcolor"))
    {
      node->SetProperty("binaryimage.selectedcolor",mitk::ColorProperty::New(qcolor.red()/255.0,qcolor.green()/255.0,qcolor.blue()/255.0));
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
   }
 }

void QmitkDataManagerView::ColorActionChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
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
}

void QmitkDataManagerView::TextureInterpolationChanged()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  if(node)
  {
    bool textureInterpolation = false;
    node->GetBoolProperty("texture interpolation", textureInterpolation);
    m_TextureInterpolation->setChecked(textureInterpolation);
  }
}

void QmitkDataManagerView::TextureInterpolationToggled( bool checked )
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  if(node)
  {
    node->SetBoolProperty("texture interpolation", checked);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

}

void QmitkDataManagerView::ColormapActionToggled( bool /*checked*/ )
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  if(!node)
    return;

  mitk::LookupTableProperty::Pointer lookupTableProperty =
    dynamic_cast<mitk::LookupTableProperty*>(node->GetProperty("LookupTable"));
  if (!lookupTableProperty)
    return;

  QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
  if(!senderAction)
    return;

  std::string activatedItem = senderAction->text().toStdString();

  mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
  if (!lookupTable)
    return;

  lookupTable->SetType(activatedItem);
  lookupTableProperty->SetValue(lookupTable);
  mitk::RenderingModeProperty::Pointer renderingMode =
    dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty("Image Rendering.Mode"));
  renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ColormapMenuAboutToShow()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
  if(!node)
    return;

  mitk::LookupTableProperty::Pointer lookupTableProperty =
    dynamic_cast<mitk::LookupTableProperty*>(node->GetProperty("LookupTable"));
  if (!lookupTableProperty)
  {
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    lookupTableProperty = mitk::LookupTableProperty::New();
    lookupTableProperty->SetLookupTable(mitkLut);
    node->SetProperty("LookupTable", lookupTableProperty);
  }

  mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
  if (!lookupTable)
    return;

  m_ColormapAction->menu()->clear();
  QAction* tmp;

  int i = 0;
  std::string lutType = lookupTable->typenameList[i];

  while (lutType != "END_OF_ARRAY")
  {
    tmp = m_ColormapAction->menu()->addAction(QString::fromStdString(lutType));
    tmp->setCheckable(true);

    if (lutType == lookupTable->GetActiveTypeAsString())
    {
      tmp->setChecked(true);
    }

    QObject::connect(tmp, SIGNAL(triggered(bool)), this, SLOT(ColormapActionToggled(bool)));

    lutType = lookupTable->typenameList[++i];
  }
}

void QmitkDataManagerView::SurfaceRepresentationMenuAboutToShow()
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
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

void QmitkDataManagerView::SurfaceRepresentationActionToggled( bool /*checked*/ )
{
  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
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

void QmitkDataManagerView::ReinitSelectedNodes( bool )
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  if (renderWindow == NULL)
    renderWindow = this->OpenRenderWindowPart(false);

  QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelection();

  foreach(mitk::DataNode::Pointer node, selectedNodes)
  {
    mitk::BaseData::Pointer basedata = node->GetData();
    if ( basedata.IsNotNull() &&
      basedata->GetTimeGeometry()->IsValid() )
    {
      renderWindow->GetRenderingManager()->InitializeViews(
          basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    }
  }
}

void QmitkDataManagerView::RemoveSelectedNodes( bool )
{
  QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
  QModelIndexList indexesOfSelectedRows;
  for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i)
  {
    indexesOfSelectedRows.push_back(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i]));
  }
  if(indexesOfSelectedRows.size() < 1)
  {
    return;
  }
  std::vector<mitk::DataNode::Pointer> selectedNodes;

  mitk::DataNode::Pointer node = 0;
  QString question = tr("Do you really want to remove ");

  for (QModelIndexList::iterator it = indexesOfSelectedRows.begin()
    ; it != indexesOfSelectedRows.end(); it++)
  {
    node = m_NodeTreeModel->GetNode(*it);
    // if node is not defined or if the node contains geometry data do not remove it
    if ( node.IsNotNull() /*& strcmp(node->GetData()->GetNameOfClass(), "PlaneGeometryData") != 0*/ )
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
    for (std::vector<mitk::DataNode::Pointer>::iterator it = selectedNodes.begin()
      ; it != selectedNodes.end(); it++)
    {
      node = *it;
      this->GetDataStorage()->Remove(node);
      if (m_GlobalReinitOnNodeDelete)
          this->GlobalReinit(false);
    }
  }
}

void QmitkDataManagerView::MakeAllNodesInvisible( bool )
{
  QList<mitk::DataNode::Pointer> nodes = m_NodeTreeModel->GetNodeSet();

  foreach(mitk::DataNode::Pointer node, nodes)
  {
    node->SetVisibility(false);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowOnlySelectedNodes( bool )
{
  QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelection();
  QList<mitk::DataNode::Pointer> allNodes = m_NodeTreeModel->GetNodeSet();

  foreach(mitk::DataNode::Pointer node, allNodes)
  {
    node->SetVisibility(selectedNodes.contains(node));
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ToggleVisibilityOfSelectedNodes( bool )
{
  QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelection();

  bool isVisible = false;
  foreach(mitk::DataNode::Pointer node, selectedNodes)
  {
    isVisible = false;
    node->GetBoolProperty("visible", isVisible);
    node->SetVisibility(!isVisible);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowInfoDialogForSelectedNodes( bool )
{
  QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelection();

  QmitkInfoDialog _QmitkInfoDialog(selectedNodes, this->m_Parent);
  _QmitkInfoDialog.exec();
}

void QmitkDataManagerView::NodeChanged(const mitk::DataNode* /*node*/)
{
  // m_FilterModel->invalidate();
  // fix as proposed by R. Khlebnikov in the mitk-users mail from 02.09.2014
  QMetaObject::invokeMethod( m_FilterModel, "invalidate", Qt::QueuedConnection );
}

QItemSelectionModel *QmitkDataManagerView::GetDataNodeSelectionModel() const
{
  return m_NodeTreeView->selectionModel();
}

void QmitkDataManagerView::GlobalReinit( bool )
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  if (renderWindow == NULL)
    renderWindow = this->OpenRenderWindowPart(false);

  // no render window available
  if (renderWindow == NULL) return;

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
}

void QmitkDataManagerView::NodeTreeViewRowsRemoved (
  const QModelIndex & /*parent*/, int /*start*/, int /*end*/ )
{
  m_CurrentRowCount = m_NodeTreeModel->rowCount();
}
void QmitkDataManagerView::NodeTreeViewRowsInserted( const QModelIndex & parent, int, int )
{
  QModelIndex viewIndex = m_FilterModel->mapFromSource(parent);
  m_NodeTreeView->setExpanded(viewIndex, true);

  // a new row was inserted
  if( m_CurrentRowCount == 0 && m_NodeTreeModel->rowCount() == 1 )
  {
    this->OpenRenderWindowPart();
    m_CurrentRowCount = m_NodeTreeModel->rowCount();
  }
}

void QmitkDataManagerView::NodeSelectionChanged( const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ )
{
  QList<mitk::DataNode::Pointer> nodes = m_NodeTreeModel->GetNodeSet();

  foreach(mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull()) {
      node->SetBoolProperty("selected", false);
    }
  }

  nodes.clear();
  nodes = this->GetCurrentSelection();

  foreach(mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull()) {
      node->SetBoolProperty("selected", true);
    }
  }
  //changing the selection does NOT require any rendering processes!
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowIn(const QString &editorId)
{
  berry::IWorkbenchPage::Pointer page = this->GetSite()->GetPage();
  berry::IEditorInput::Pointer input(new mitk::DataStorageEditorInput(this->GetDataStorageReference()));
  page->OpenEditor(input, editorId, false, berry::IWorkbenchPage::MATCH_ID);
}

void QmitkDataManagerView::OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes)
{
  m_NodeTreeView->clearSelection();

  for (int i = 0; i < nodes.size(); ++i) {
    QModelIndex itemIndex = m_NodeTreeModel->GetIndex(nodes[i]);
    m_NodeTreeView->selectionModel()->select(m_FilterModel->mapFromSource(itemIndex), QItemSelectionModel::Select);
    m_NodeTreeView->scrollTo(m_FilterModel->mapFromSource(itemIndex));
  }
}

mitk::IRenderWindowPart* QmitkDataManagerView::OpenRenderWindowPart(bool activatedEditor)
{
  if (activatedEditor)
  {
    return this->GetRenderWindowPart(QmitkAbstractView::ACTIVATE);
  }
  else
  {
    return this->GetRenderWindowPart(QmitkAbstractView::BRING_TO_FRONT);
  }
}
