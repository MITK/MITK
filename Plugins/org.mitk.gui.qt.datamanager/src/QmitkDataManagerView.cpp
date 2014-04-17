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

#include <itkOtsuThresholdImageFilter.h>

//# Own Includes
//## mitk
#include "mitkDataStorageEditorInput.h"
#include "mitkIDataStorageReference.h"
#include "mitkNodePredicateDataType.h"
#include "mitkCoreObjectFactory.h"
#include "mitkDataNodeFactory.h"
#include "mitkColorProperty.h"
#include "mitkCommon.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNot.h"
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
#include <QmitkDataStorageFilterProxyModel.h>
#include <QmitkCustomVariants.h>
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
#include <QMotifStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QKeyEvent>
#include <QColor>
#include <QColorDialog>
#include <QSizePolicy>
#include <QSignalMapper>
#include <QSortFilterProxyModel>

#include "mitkDataNodeObject.h"
#include "mitkIContextMenuAction.h"
#include "berryIExtensionPointService.h"
#include "mitkRenderingModeProperty.h"

const std::string QmitkDataManagerView::VIEW_ID = "org.mitk.views.datamanager";

QmitkDataManagerView::QmitkDataManagerView()
    : m_GlobalReinitOnNodeDelete(true),
      m_ItemDelegate(NULL)
{
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
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

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
  m_SurfaceDecimation = prefs->GetBool("Use surface decimation", false);

  m_FilterModel = new QmitkDataStorageFilterProxyModel();
  m_FilterModel->setSourceModel(m_NodeTreeModel);
  m_FilterModel->SetShowHelperObjects(
      prefs->GetBool("Show helper objects", false));
  m_FilterModel->SetShowNodesContainingNoData(
      prefs->GetBool("Show nodes containing no data", false));

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
  std::list<berry::IEditorDescriptor::Pointer> editors = editorRegistry->GetEditors("*.mitk");
  if (editors.size() > 1)
  {
    m_ShowInMapper = new QSignalMapper(this);
    foreach(berry::IEditorDescriptor::Pointer descriptor, editors)
    {
      QAction* action = new QAction(QString::fromStdString(descriptor->GetLabel()), this);
      m_ShowInActions << action;
      m_ShowInMapper->connect(action, SIGNAL(triggered()), m_ShowInMapper, SLOT(map()));
      m_ShowInMapper->setMapping(action, QString::fromStdString(descriptor->GetId()));
    }
    connect(m_ShowInMapper, SIGNAL(mapped(QString)), this, SLOT(ShowIn(QString)));
  }

  QmitkNodeDescriptor* unknownDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataNodeDescriptor();

  QmitkNodeDescriptor* imageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  QmitkNodeDescriptor* surfaceDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Surface");

  QAction* globalReinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), "Global Reinit", this);
  QObject::connect( globalReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( GlobalReinit(bool) ) );
  unknownDataNodeDescriptor->AddAction(globalReinitAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, globalReinitAction));

  QAction* saveAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Save_48.png"), "Save...", this);
  QObject::connect( saveAction, SIGNAL( triggered(bool) )
    , this, SLOT( SaveSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(saveAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,saveAction));

  QAction* removeAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Remove_48.png"), "Remove", this);
  QObject::connect( removeAction, SIGNAL( triggered(bool) )
    , this, SLOT( RemoveSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(removeAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,removeAction));

  QAction* reinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), "Reinit", this);
  QObject::connect( reinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ReinitSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(reinitAction);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,reinitAction));

  // find contextMenuAction extension points and add them to the node descriptor
  berry::IExtensionPointService::Pointer extensionPointService = berry::Platform::GetExtensionPointService();
  berry::IConfigurationElement::vector cmActions(
    extensionPointService->GetConfigurationElementsFor("org.mitk.gui.qt.datamanager.contextMenuActions") );
  berry::IConfigurationElement::vector::iterator cmActionsIt;

  std::string cmNodeDescriptorName;
  std::string cmLabel;
  std::string cmIcon;
  std::string cmClass;

  QmitkNodeDescriptor* tmpDescriptor;
  QAction* contextMenuAction;
  QVariant cmActionDataIt;
  m_ConfElements.clear();

  int i=1;
  for (cmActionsIt = cmActions.begin()
    ; cmActionsIt != cmActions.end()
    ; ++cmActionsIt)
  {
    cmIcon.erase();
    if((*cmActionsIt)->GetAttribute("nodeDescriptorName", cmNodeDescriptorName)
      && (*cmActionsIt)->GetAttribute("label", cmLabel)
      && (*cmActionsIt)->GetAttribute("class", cmClass))
    {
      (*cmActionsIt)->GetAttribute("icon", cmIcon);
      // create context menu entry here
      tmpDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(QString::fromStdString(cmNodeDescriptorName));
      if(!tmpDescriptor)
      {
        MITK_WARN << "cannot add action \"" << cmLabel << "\" because descriptor " << cmNodeDescriptorName << " does not exist";
        continue;
      }
      contextMenuAction = new QAction( QString::fromStdString(cmLabel), parent);
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

  QLabel* _OpacityLabel = new QLabel("Opacity: ");
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

  QLabel* _ColorLabel = new QLabel("Color: ");
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

  m_TextureInterpolation = new QAction("Texture Interpolation", this);
  m_TextureInterpolation->setCheckable ( true );
  QObject::connect( m_TextureInterpolation, SIGNAL( changed() )
    , this, SLOT( TextureInterpolationChanged() ) );
  QObject::connect( m_TextureInterpolation, SIGNAL( toggled(bool) )
    , this, SLOT( TextureInterpolationToggled(bool) ) );
  imageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor,m_TextureInterpolation));

  m_ColormapAction = new QAction("Colormap", this);
  m_ColormapAction->setMenu(new QMenu);
  QObject::connect( m_ColormapAction->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( ColormapMenuAboutToShow() ) );
  imageDataNodeDescriptor->AddAction(m_ColormapAction, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor, m_ColormapAction));

  m_SurfaceRepresentation = new QAction("Surface Representation", this);
  m_SurfaceRepresentation->setMenu(new QMenu);
  QObject::connect( m_SurfaceRepresentation->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( SurfaceRepresentationMenuAboutToShow() ) );
  surfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentation, false);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(surfaceDataNodeDescriptor, m_SurfaceRepresentation));

  QAction* showOnlySelectedNodes
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowSelectedNode_48.png")
    , "Show only selected nodes", this);
  QObject::connect( showOnlySelectedNodes, SIGNAL( triggered(bool) )
    , this, SLOT( ShowOnlySelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(showOnlySelectedNodes);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor, showOnlySelectedNodes));

  QAction* toggleSelectedVisibility
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/InvertShowSelectedNode_48.png")
    , "Toggle visibility", this);
  QObject::connect( toggleSelectedVisibility, SIGNAL( triggered(bool) )
    , this, SLOT( ToggleVisibilityOfSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(toggleSelectedVisibility);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,toggleSelectedVisibility));

  QAction* actionShowInfoDialog
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowDataInfo_48.png")
    , "Details...", this);
  QObject::connect( actionShowInfoDialog, SIGNAL( triggered(bool) )
    , this, SLOT( ShowInfoDialogForSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(actionShowInfoDialog);
  m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(unknownDataNodeDescriptor,actionShowInfoDialog));

  //obsolete...
  //QAction* otsuFilterAction = new QAction("Apply Otsu Filter", this);
  //QObject::connect( otsuFilterAction, SIGNAL( triggered(bool) )
  //  , this, SLOT( OtsuFilter(bool) ) );
  // //Otsu filter does not work properly, remove it temporarily
  // imageDataNodeDescriptor->AddAction(otsuFilterAction);
  // m_DescriptorActionList.push_back(std::pair<QmitkNodeDescriptor*, QAction*>(imageDataNodeDescriptor,otsuFilterAction));

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

  std::string className;
  std::string smoothed;
  confElem->GetAttribute("class", className);
  confElem->GetAttribute("smoothed", smoothed);

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

  if( m_FilterModel->GetShowHelperObjectsFlag()!= prefs->GetBool("Show helper objects", false) )
      m_FilterModel->SetShowHelperObjects(!m_FilterModel->GetShowHelperObjectsFlag());

  if (m_FilterModel->GetShowNodesContainingNoDataFlag() != prefs->GetBool("Show nodes containing no data", false))
      m_FilterModel->SetShowNodesContainingNoData(!m_FilterModel->GetShowNodesContainingNoDataFlag());

  m_GlobalReinitOnNodeDelete = prefs->GetBool("Call global reinit if node is deleted", true);

  m_NodeTreeView->expandAll();

  m_SurfaceDecimation = prefs->GetBool("Use surface decimation", false);

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
    if (node)
  {
    float opacity = static_cast<float>(value)/100.0f;
    node->SetFloatProperty("opacity", opacity);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkDataManagerView::OpacityActionChanged()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
  {
    float opacity = 0.0;
    if(node->GetFloatProperty("opacity", opacity))
    {
      m_OpacitySlider->setValue(static_cast<int>(opacity*100));
    }
  }
}

void QmitkDataManagerView::ColorChanged()
 {
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
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
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
   }
 }

void QmitkDataManagerView::ColorActionChanged()
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
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
    if (node)
  {
    bool textureInterpolation = false;
    node->GetBoolProperty("texture interpolation", textureInterpolation);
    m_TextureInterpolation->setChecked(textureInterpolation);
  }
}

void QmitkDataManagerView::TextureInterpolationToggled( bool checked )
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (node)
  {
    node->SetBoolProperty("texture interpolation", checked);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

}

void QmitkDataManagerView::ColormapActionToggled( bool /*checked*/ )
{
    mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(m_NodeTreeView->selectionModel()->currentIndex()));
    if (!node)
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
    if (!node)
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
    if (!node)
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

void QmitkDataManagerView::SaveSelectedNodes( bool )
{
  QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
  QModelIndexList indexesOfSelectedRows;
  for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i) {
      indexesOfSelectedRows.push_back(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i]));
  }

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
      {
        QString error;
        try
        {
           mitk::QmitkIOUtil::SaveBaseDataWithDialog( data.GetPointer(), node->GetName().c_str(), m_Parent );
        }
        catch(std::exception& e)
        {
          error = e.what();
        }
        catch(...)
        {
          error = "Unknown error occured";
        }
        if( !error.isEmpty() )
          QMessageBox::critical( m_Parent, "Error saving...", error );
      }
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
      renderWindow->GetRenderingManager()->RequestUpdateAll();
    }
  }
}

void QmitkDataManagerView::RemoveSelectedNodes( bool )
{
    QModelIndexList indexesOfSelectedRowsFiltered = m_NodeTreeView->selectionModel()->selectedRows();
    QModelIndexList indexesOfSelectedRows;
    for (int i = 0; i < indexesOfSelectedRowsFiltered.size(); ++i) {
        indexesOfSelectedRows.push_back(m_FilterModel->mapToSource(indexesOfSelectedRowsFiltered[i]));
    }
  if(indexesOfSelectedRows.size() < 1)
  {
    return;
  }
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
          basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
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

void QmitkDataManagerView::OtsuFilter( bool )
{
  QList<mitk::DataNode::Pointer> selectedNodes = this->GetCurrentSelection();

  mitk::Image::Pointer mitkImage = 0;
  foreach(mitk::DataNode::Pointer node, selectedNodes)
  {
    mitkImage = dynamic_cast<mitk::Image*>( node->GetData() );

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
      std::string nameOfResultImage = node->GetName();
      nameOfResultImage.append("Otsu");
      resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
      resultNode->SetProperty("binary", mitk::BoolProperty::New(true) );
      resultNode->SetData( mitk::ImportItkImage(filter->GetOutput())->Clone());

      this->GetDataStorage()->Add(resultNode, node);

    }
    catch( std::exception& err )
    {
      MITK_ERROR(this->GetClassName()) << err.what();
    }

  }
}
void QmitkDataManagerView::NodeTreeViewRowsRemoved (
  const QModelIndex & /*parent*/, int /*start*/, int /*end*/ )
{
  m_CurrentRowCount = m_FilterModel->rowCount();
}
void QmitkDataManagerView::NodeTreeViewRowsInserted( const QModelIndex & parent, int, int )
{
  m_NodeTreeView->setExpanded(parent, true);

  // a new row was inserted
  if( m_CurrentRowCount == 0 && m_FilterModel->rowCount() == 1 )
  {
    this->OpenRenderWindowPart();
    m_CurrentRowCount = m_FilterModel->rowCount();
    /*
    std::vector<mitk::DataNode*> nodes = m_NodeTreeModel->GetNodeSet();
    if(nodes.size() == 1)
    {
      QModelIndex treeIndex = m_NodeTreeModel->GetIndex(nodes.front());
      m_NodeTreeView->selectionModel()->setCurrentIndex( treeIndex, QItemSelectionModel::ClearAndSelect );
    }
    */
  }
}

void QmitkDataManagerView::NodeSelectionChanged( const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ )
{
  QList<mitk::DataNode::Pointer> nodes = m_NodeTreeModel->GetNodeSet();

  foreach(mitk::DataNode::Pointer node, nodes)
  {
    if ( node.IsNotNull() )
      node->SetBoolProperty("selected", false);
  }

  nodes.clear();
  nodes = this->GetCurrentSelection();

  foreach(mitk::DataNode::Pointer node, nodes)
  {
    if ( node.IsNotNull() )
      node->SetBoolProperty("selected", true);
  }
  //changing the selection does NOT require any rendering processes!
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ShowIn(const QString &editorId)
{
  berry::IWorkbenchPage::Pointer page = this->GetSite()->GetPage();
  berry::IEditorInput::Pointer input(new mitk::DataStorageEditorInput(this->GetDataStorageReference()));
  page->OpenEditor(input, editorId.toStdString(), false, berry::IWorkbenchPage::MATCH_ID);
}

mitk::IRenderWindowPart* QmitkDataManagerView::OpenRenderWindowPart(bool activatedEditor)
{
  if (activatedEditor)
  {
    return this->GetRenderWindowPart(QmitkAbstractView::ACTIVATE | QmitkAbstractView::OPEN);
  }
  else
  {
    return this->GetRenderWindowPart(QmitkAbstractView::BRING_TO_FRONT | QmitkAbstractView::OPEN);
  }
}
