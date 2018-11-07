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
#include <berryAbstractUICTKPlugin.h>
#include <berryIContributor.h>
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

QmitkDataManagerView::QmitkDataManagerView()
  : m_GlobalReinitOnNodeDelete(true)
  , m_GlobalReinitOnNodeVisibilityChanged(false)
  , m_ItemDelegate(nullptr)
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
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(VIEW_ID))
        .Cast<berry::IBerryPreferences>();
  assert( prefs );
  prefs->OnChanged.AddListener( berry::MessageDelegate1<QmitkDataManagerView
    , const berry::IBerryPreferences*>( this
      , &QmitkDataManagerView::OnPreferencesChanged ) );

  //# GUI
  m_NodeTreeModel = new QmitkDataStorageTreeModel(this->GetDataStorage(), prefs->GetBool("Place new nodes on top", true));
  m_NodeTreeModel->setParent( parent );
  m_NodeTreeModel->SetAllowHierarchyChange(
    prefs->GetBool("Allow changing of parent node", false));
  m_SurfaceDecimation = prefs->GetBool("Use surface decimation", false);
  // Prepare filters
  m_HelperObjectFilterPredicate = mitk::NodePredicateOr::New(
   mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)),
   mitk::NodePredicateProperty::New("hidden object", mitk::BoolProperty::New(true)));
  m_NodeWithNoDataFilterPredicate = mitk::NodePredicateData::New(nullptr);

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

  connect( m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );
  connect( m_NodeTreeModel, SIGNAL(rowsInserted (const QModelIndex&, int, int))
    , this, SLOT(NodeTreeViewRowsInserted ( const QModelIndex&, int, int )) );
  connect( m_NodeTreeModel, SIGNAL(rowsRemoved (const QModelIndex&, int, int))
    , this, SLOT(NodeTreeViewRowsRemoved( const QModelIndex&, int, int )) );
  connect( m_NodeTreeView->selectionModel()
    , SIGNAL( selectionChanged ( const QItemSelection &, const QItemSelection & ) )
    , this
    , SLOT( NodeSelectionChanged ( const QItemSelection &, const QItemSelection & ) ) );
  connect(m_NodeTreeModel, &QmitkDataStorageTreeModel::nodeVisibilityChanged, this, &QmitkDataManagerView::OnNodeVisibilityChanged);

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

  auto unknownDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataNodeDescriptor();

  auto imageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");

  auto multiComponentImageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("MultiComponentImage");

  auto diffusionImageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("DiffusionImage");

  auto fiberBundleDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("FiberBundle");

  auto peakImageDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PeakImage");

  auto segmentDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Segment");

  auto surfaceDataNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Surface");

  auto pointSetNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PointSet");

  auto planarLineNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarLine");
  auto planarCircleNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarCircle");
  auto planarEllipseNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarEllipse");
  auto planarAngleNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarAngle");
  auto planarFourPointAngleNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarFourPointAngle");
  auto planarRectangleNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarRectangle");
  auto planarPolygonNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarPolygon");
  auto planarPathNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarPath");
  auto planarDoubleEllipseNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarDoubleEllipse");
  auto planarBezierCurveNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarBezierCurve");
  auto planarSubdivisionPolygonNodeDescriptor =
    QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarSubdivisionPolygon");

  QAction* globalReinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), tr("Global Reinit"), this);
  QObject::connect( globalReinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( GlobalReinit(bool) ) );
  unknownDataNodeDescriptor->AddAction(globalReinitAction);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor, globalReinitAction));

  QAction* saveAction = new QmitkFileSaveAction(QIcon(":/org.mitk.gui.qt.datamanager/Save_48.png"),
                                                this->GetSite()->GetWorkbenchWindow());
  unknownDataNodeDescriptor->AddAction(saveAction);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor,saveAction));

  QAction* removeAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Remove_48.png"), tr("Remove"), this);
  QObject::connect( removeAction, SIGNAL( triggered(bool) )
    , this, SLOT( RemoveSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(removeAction);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor,removeAction));

  QAction* reinitAction = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"), tr("Reinit"), this);
  QObject::connect( reinitAction, SIGNAL( triggered(bool) )
    , this, SLOT( ReinitSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(reinitAction);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor,reinitAction));

  // find contextMenuAction extension points and add them to the node descriptor
  berry::IExtensionRegistry* extensionPointService = berry::Platform::GetExtensionRegistry();
  QList<berry::IConfigurationElement::Pointer> customMenuConfigs =
    extensionPointService->GetConfigurationElementsFor("org.mitk.gui.qt.datamanager.contextMenuActions");

  // Prepare all custom QActions
  m_ConfElements.clear();
  DescriptorActionListType customMenuEntries;
  for (auto& customMenuConfig : customMenuConfigs)
  {
    QString actionNodeDescriptorName = customMenuConfig->GetAttribute("nodeDescriptorName");
    QString actionLabel = customMenuConfig->GetAttribute("label");
    QString actionClass = customMenuConfig->GetAttribute("class");

    if (actionNodeDescriptorName.isEmpty() || actionLabel.isEmpty() || actionClass.isEmpty())
    {
        continue;
    }

    QString actionIconName = customMenuConfig->GetAttribute("icon");

    // Find matching descriptor
    auto nodeDescriptor  = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(actionNodeDescriptorName);
    if ( nodeDescriptor == nullptr)
    {
        MITK_WARN << "Cannot add action \"" << actionLabel << "\" because descriptor " << actionNodeDescriptorName << " does not exist.";
        continue;
    }

    // Create action with or without icon
    QAction* contextMenuAction;
    if ( !actionIconName.isEmpty() )
    {
        QIcon actionIcon;
        if ( QFile::exists(actionIconName) )
        {
          actionIcon = QIcon(actionIconName);
        } else
        {
          actionIcon = berry::AbstractUICTKPlugin::ImageDescriptorFromPlugin(
            customMenuConfig->GetContributor()->GetName(), actionIconName);
        }
        contextMenuAction = new QAction(actionIcon, actionLabel, parent);
    } else
    {
        contextMenuAction = new QAction(actionLabel, parent);
    }

    // Define menu handler to trigger on click
    connect(contextMenuAction, static_cast<void(QAction::*)(bool)>(&QAction::triggered),
            this, &QmitkDataManagerView::ContextMenuActionTriggered);

    // Mark configuration element into lookup list for context menu handler
    m_ConfElements[contextMenuAction] = customMenuConfig;
    // Mark new action in sortable list for addition to descriptor
    customMenuEntries.emplace_back(nodeDescriptor, contextMenuAction);
  }

  // Sort all custom QActions by their texts
  {
    using ListEntryType = std::pair<QmitkNodeDescriptor*,QAction*>;
    std::sort(customMenuEntries.begin(), customMenuEntries.end(),
              [](const ListEntryType& left, const ListEntryType& right) -> bool
              {
                  assert (left.second != nullptr && right.second != nullptr); // unless we messed up above
                  return left.second->text() < right.second->text();
              });
  }

  // Add custom QActions in sorted order
  int globalAddedMenuIndex=1;
  for (auto& menuEntryToAdd : customMenuEntries)
  {
    auto& nodeDescriptor = menuEntryToAdd.first;
    auto& contextMenuAction = menuEntryToAdd.second;

    // TODO is the action "data" used by anything? Otherwise remove!
    contextMenuAction->setData(static_cast<int>(globalAddedMenuIndex));
    ++globalAddedMenuIndex;

    // Really add this action to that descriptor (in pre-defined order)
    nodeDescriptor->AddAction(contextMenuAction);

    // Mark new action into list of descriptors to remove in d'tor
    m_DescriptorActionList.push_back(menuEntryToAdd);
  }

  m_OpacitySlider = new QSlider;
  m_OpacitySlider->setMinimum(0);
  m_OpacitySlider->setMaximum(100);
  m_OpacitySlider->setOrientation(Qt::Horizontal);
  QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
    , this, SLOT( OpacityChanged(int) ) );

  QLabel* opacityLabel = new QLabel(tr("Opacity: "));
  QHBoxLayout* opacityWidgetLayout = new QHBoxLayout;
  opacityWidgetLayout->setContentsMargins(4,4,4,4);
  opacityWidgetLayout->addWidget(opacityLabel);
  opacityWidgetLayout->addWidget(m_OpacitySlider);
  QWidget* opacityWidget = new QWidget;
  opacityWidget->setLayout(opacityWidgetLayout);

  QWidgetAction* opacityAction = new QWidgetAction(this);
  opacityAction->setDefaultWidget(opacityWidget);
  QObject::connect( opacityAction , SIGNAL( changed() )
    , this, SLOT( OpacityActionChanged() ) );
  unknownDataNodeDescriptor->AddAction(opacityAction , false);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor,opacityAction));

  m_ColorButton = new QPushButton;
  m_ColorButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  //m_ColorButton->setText("Change color");
  QObject::connect( m_ColorButton, SIGNAL( clicked() )
    , this, SLOT( ColorChanged() ) );

  QLabel* colorLabel = new QLabel(tr("Color: "));
  colorLabel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  QHBoxLayout* colorWidgetLayout = new QHBoxLayout;
  colorWidgetLayout->setContentsMargins(4,4,4,4);
  colorWidgetLayout->addWidget(colorLabel);
  colorWidgetLayout->addWidget(m_ColorButton);
  QWidget* colorWidget = new QWidget;
  colorWidget->setLayout(colorWidgetLayout);

  QWidgetAction* colorAction = new QWidgetAction(this);
  colorAction->setDefaultWidget(colorWidget);
  QObject::connect( colorAction, SIGNAL( changed() )
    , this, SLOT( ColorActionChanged() ) );

  { // only give the color context menu option where appropriate
    bool colorActionCanBatch = true;
    if (imageDataNodeDescriptor != nullptr)
    {
      imageDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(imageDataNodeDescriptor, colorAction));
    }
    if (multiComponentImageDataNodeDescriptor != nullptr)
    {
      multiComponentImageDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(multiComponentImageDataNodeDescriptor, colorAction));
    }
    if (diffusionImageDataNodeDescriptor != nullptr)
    {
      diffusionImageDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(diffusionImageDataNodeDescriptor, colorAction));
    }
    if (fiberBundleDataNodeDescriptor != nullptr)
    {
      fiberBundleDataNodeDescriptor->AddAction(colorAction, false);
      m_DescriptorActionList.push_back(std::make_pair(fiberBundleDataNodeDescriptor, colorAction));
    }
    if (peakImageDataNodeDescriptor != nullptr)
    {
      peakImageDataNodeDescriptor->AddAction(colorAction, false);
      m_DescriptorActionList.push_back(std::make_pair(peakImageDataNodeDescriptor, colorAction));
    }
    if (segmentDataNodeDescriptor != nullptr)
    {
      segmentDataNodeDescriptor->AddAction(colorAction, false);
      m_DescriptorActionList.push_back(std::make_pair(segmentDataNodeDescriptor, colorAction));
    }
    if (surfaceDataNodeDescriptor != nullptr)
    {
      surfaceDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(surfaceDataNodeDescriptor, colorAction));
    }
    if (pointSetNodeDescriptor != nullptr)
    {
      pointSetNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(pointSetNodeDescriptor, colorAction));
    }

    if (planarLineNodeDescriptor != nullptr)
    {
      planarLineNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarLineNodeDescriptor, colorAction));
    }

    if (planarCircleNodeDescriptor != nullptr)
    {
      planarCircleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarCircleNodeDescriptor, colorAction));
    }

    if (planarEllipseNodeDescriptor != nullptr)
    {
      planarEllipseNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarEllipseNodeDescriptor, colorAction));
    }

    if (planarAngleNodeDescriptor != nullptr)
    {
      planarAngleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarAngleNodeDescriptor, colorAction));
    }

    if (planarFourPointAngleNodeDescriptor != nullptr)
    {
      planarFourPointAngleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarFourPointAngleNodeDescriptor, colorAction));
    }

    if (planarRectangleNodeDescriptor != nullptr)
    {
      planarRectangleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarRectangleNodeDescriptor, colorAction));
    }

    if (planarPolygonNodeDescriptor != nullptr)
    {
      planarPolygonNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarPolygonNodeDescriptor, colorAction));
    }

    if (planarPathNodeDescriptor != nullptr)
    {
      planarPathNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarPathNodeDescriptor, colorAction));
    }

    if (planarDoubleEllipseNodeDescriptor != nullptr)
    {
      planarDoubleEllipseNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarDoubleEllipseNodeDescriptor, colorAction));
    }

    if (planarBezierCurveNodeDescriptor != nullptr)
    {
      planarBezierCurveNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarBezierCurveNodeDescriptor, colorAction));
    }

    if (planarSubdivisionPolygonNodeDescriptor != nullptr)
    {
      planarSubdivisionPolygonNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
      m_DescriptorActionList.push_back(std::make_pair(planarSubdivisionPolygonNodeDescriptor, colorAction));
    }
  }

  m_ComponentSlider = new QmitkNumberPropertySlider;
  m_ComponentSlider->setOrientation(Qt::Horizontal);
  //QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
  //  , this, SLOT( OpacityChanged(int) ) );

  QLabel* componentLabel = new QLabel(tr("Component: "));
  QHBoxLayout* componentWidgetLayout = new QHBoxLayout;
  componentWidgetLayout->setContentsMargins(4,4,4,4);
  componentWidgetLayout->addWidget(componentLabel);
  componentWidgetLayout->addWidget(m_ComponentSlider);
  QLabel* componentValueLabel = new QLabel();
  componentWidgetLayout->addWidget(componentValueLabel);
  connect(m_ComponentSlider, SIGNAL(valueChanged(int)), componentValueLabel, SLOT(setNum(int)));
  QWidget* componentWidget = new QWidget;
  componentWidget->setLayout(componentWidgetLayout);

  QWidgetAction* componentAction = new QWidgetAction(this);
  componentAction->setDefaultWidget(componentWidget);
  QObject::connect( componentAction , SIGNAL( changed() )
    , this, SLOT( ComponentActionChanged() ) );
  multiComponentImageDataNodeDescriptor->AddAction(componentAction, false);
  m_DescriptorActionList.push_back(std::make_pair(multiComponentImageDataNodeDescriptor,componentAction));
  if (diffusionImageDataNodeDescriptor!=nullptr)
  {
      diffusionImageDataNodeDescriptor->AddAction(componentAction, false);
      m_DescriptorActionList.push_back(std::make_pair(diffusionImageDataNodeDescriptor,componentAction));
  }

  m_TextureInterpolation = new QAction(tr("Texture Interpolation"), this);
  m_TextureInterpolation->setCheckable ( true );
  QObject::connect( m_TextureInterpolation, SIGNAL( changed() )
    , this, SLOT( TextureInterpolationChanged() ) );
  QObject::connect( m_TextureInterpolation, SIGNAL( toggled(bool) )
    , this, SLOT( TextureInterpolationToggled(bool) ) );
  imageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
  m_DescriptorActionList.push_back(std::make_pair(imageDataNodeDescriptor,m_TextureInterpolation));
  if (diffusionImageDataNodeDescriptor!=nullptr)
  {
      diffusionImageDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
      m_DescriptorActionList.push_back(std::make_pair(diffusionImageDataNodeDescriptor,m_TextureInterpolation));
  }
  if (segmentDataNodeDescriptor != nullptr)
  {
    segmentDataNodeDescriptor->AddAction(m_TextureInterpolation, false);
    m_DescriptorActionList.push_back(std::make_pair(segmentDataNodeDescriptor, m_TextureInterpolation));
  }

  m_ColormapAction = new QAction(tr("Colormap"), this);
  m_ColormapAction->setMenu(new QMenu);
  QObject::connect( m_ColormapAction->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( ColormapMenuAboutToShow() ) );
  imageDataNodeDescriptor->AddAction(m_ColormapAction);
  m_DescriptorActionList.push_back(std::make_pair(imageDataNodeDescriptor, m_ColormapAction));
  if (diffusionImageDataNodeDescriptor!=nullptr)
  {
      diffusionImageDataNodeDescriptor->AddAction(m_ColormapAction, false);
      m_DescriptorActionList.push_back(std::make_pair(diffusionImageDataNodeDescriptor, m_ColormapAction));
  }

  m_SurfaceRepresentation = new QAction(tr("Surface Representation"), this);
  m_SurfaceRepresentation->setMenu(new QMenu(m_NodeTreeView));
  QObject::connect( m_SurfaceRepresentation->menu(), SIGNAL( aboutToShow() )
    , this, SLOT( SurfaceRepresentationMenuAboutToShow() ) );
  surfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentation, false);
  m_DescriptorActionList.push_back(std::make_pair(surfaceDataNodeDescriptor, m_SurfaceRepresentation));

  QAction* showOnlySelectedNodes
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowSelectedNode_48.png")
    , tr("Show only selected nodes"), this);
  QObject::connect( showOnlySelectedNodes, SIGNAL( triggered(bool) )
    , this, SLOT( ShowOnlySelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(showOnlySelectedNodes);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor, showOnlySelectedNodes));

  QAction* toggleSelectedVisibility
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/InvertShowSelectedNode_48.png")
    , tr("Toggle visibility"), this);
  QObject::connect( toggleSelectedVisibility, SIGNAL( triggered(bool) )
    , this, SLOT( ToggleVisibilityOfSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(toggleSelectedVisibility);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor,toggleSelectedVisibility));

  QAction* actionShowInfoDialog
    = new QAction(QIcon(":/org.mitk.gui.qt.datamanager/ShowDataInfo_48.png")
    , tr("Details..."), this);
  QObject::connect( actionShowInfoDialog, SIGNAL( triggered(bool) )
    , this, SLOT( ShowInfoDialogForSelectedNodes(bool) ) );
  unknownDataNodeDescriptor->AddAction(actionShowInfoDialog);
  m_DescriptorActionList.push_back(std::make_pair(unknownDataNodeDescriptor,actionShowInfoDialog));

  QGridLayout* dndFrameWidgetLayout = new QGridLayout;
  dndFrameWidgetLayout->addWidget(m_NodeTreeView, 0, 0);
  dndFrameWidgetLayout->setContentsMargins(0,0,0,0);

  m_DndFrameWidget = new QmitkDnDFrameWidget(m_Parent);
  m_DndFrameWidget->setLayout(dndFrameWidgetLayout);

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

  contextMenuAction->SetDataStorage(this->GetDataStorage());

  if(className == "QmitkCreatePolygonModelAction")
  {

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

  contextMenuAction->Run( this->GetCurrentSelection() ); // run the action
}

void QmitkDataManagerView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  if (m_NodeTreeModel->GetPlaceNewNodesOnTopFlag() != prefs->GetBool("Place new nodes on top", true))
  {
    m_NodeTreeModel->SetPlaceNewNodesOnTop(!m_NodeTreeModel->GetPlaceNewNodesOnTopFlag());
  }

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
  m_GlobalReinitOnNodeVisibilityChanged = prefs->GetBool("Call global reinit if node visibility is changed", false);

  m_NodeTreeView->expandAll();

  m_SurfaceDecimation = prefs->GetBool("Use surface decimation", false);

  m_NodeTreeModel->SetAllowHierarchyChange(prefs->GetBool("Allow changing of parent node", false));

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
    ColorActionChanged(); // update color button

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
      QMenu* showInMenu = m_NodeMenu->addMenu(tr("Show In"));
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
  mitk::IntProperty* componentProperty = nullptr;
  int numComponents = 0;
  if(node)
  {
    componentProperty =
        dynamic_cast<mitk::IntProperty*>(node->GetProperty("Image.Displayed Component"));
    mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
    if (img != nullptr)
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
    m_ComponentSlider->SetProperty(static_cast<mitk::IntProperty*>(nullptr));
  }
}

void QmitkDataManagerView::ColorChanged()
{
  bool colorSelected = false;
  QColor newColor;

  auto selectedIndices = m_NodeTreeView->selectionModel()->selectedIndexes();
  for (auto& selectedIndex : selectedIndices)
  {
    auto node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(selectedIndex));
    if(node)
    {
      float rgb[3];
      if (node->GetColor(rgb))
      {
        if (!colorSelected)
        {
          QColor initial(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
          newColor = QColorDialog::getColor(initial, nullptr, QString(tr("Change color")));

          if ( newColor.isValid() )
          {
            colorSelected = true;
          }
          else
          {
            return;
          }
        }

        node->SetProperty("color", mitk::ColorProperty::New(newColor.redF(), newColor.greenF(), newColor.blueF()));
        if ( node->GetProperty("binaryimage.selectedcolor") )
        {
          node->SetProperty("binaryimage.selectedcolor", mitk::ColorProperty::New(newColor.redF(), newColor.greenF(), newColor.blueF()));
        }
      }
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataManagerView::ColorActionChanged()
{
  // Adapts color displayed in context menu item
  auto selectedIndices = m_NodeTreeView->selectionModel()->selectedIndexes();
  if (selectedIndices.isEmpty())
    return;

  mitk::DataNode* node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(selectedIndices.front()));
  if(node)
  {
    float rgb[3];
    if (node->GetColor(rgb))
    {
      QColor color(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
      QString styleSheet = QString("background-color: ") + color.name(QColor::HexRgb);
      m_ColorButton->setAutoFillBackground(true);
      m_ColorButton->setStyleSheet(styleSheet);
    }
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
  auto selectedIndices = m_NodeTreeView->selectionModel()->selectedIndexes();
  for (auto& selectedIndex : selectedIndices)
  {
    auto node = m_NodeTreeModel->GetNode(m_FilterModel->mapToSource(selectedIndex));
    if (!node)
      return;

    mitk::LookupTableProperty::Pointer lookupTableProperty =
      dynamic_cast<mitk::LookupTableProperty*>(node->GetProperty("LookupTable"));
    if (!lookupTableProperty)
      return;

    QAction* senderAction = qobject_cast<QAction*>(QObject::sender());
    if (!senderAction)
      return;

    mitk::LookupTable::Pointer lookupTable = lookupTableProperty->GetValue();
    if (!lookupTable)
      return;

    std::string activatedItem = senderAction->text().toStdString();
    lookupTable->SetType(activatedItem);
    lookupTableProperty->SetValue(lookupTable);

    if (mitk::LookupTable::LookupTableType::MULTILABEL == lookupTable->GetActiveType())
    {
      // special case: multilabel => set the level window to include the whole pixel range
      UseWholePixelRange(node);
    }

    mitk::RenderingModeProperty::Pointer renderingMode =
      dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty("Image Rendering.Mode"));
    renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
  }

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
  auto renderWindow = this->GetRenderWindowPart();

  if (nullptr == renderWindow)
    renderWindow = this->OpenRenderWindowPart(false);

  if (nullptr == renderWindow)
    return;

  auto dataStorage = this->GetDataStorage();

  auto selectedNodesIncludedInBoundingBox = mitk::NodePredicateAnd::New(
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false))),
    mitk::NodePredicateProperty::New("selected", mitk::BoolProperty::New(true)));

  auto nodes = dataStorage->GetSubset(selectedNodesIncludedInBoundingBox);

  if (nodes->empty())
    return;

  if (1 == nodes->Size()) // Special case: If exactly one ...
  {
    auto image = dynamic_cast<mitk::Image*>(nodes->ElementAt(0)->GetData());

    if (nullptr != image) // ... image is selected, reinit is expected to rectify askew images.
    {
      mitk::RenderingManager::GetInstance()->InitializeViews(image->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
      return;
    }
  }

  auto boundingGeometry = dataStorage->ComputeBoundingGeometry3D(nodes, "visible");

  mitk::RenderingManager::GetInstance()->InitializeViews(boundingGeometry);
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

  mitk::DataNode::Pointer node = nullptr;
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
  question.append(tr(" from data storage?"));

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

  QmitkInfoDialog qmitkInfoDialog(selectedNodes, this->m_Parent);
  qmitkInfoDialog.exec();
}

void QmitkDataManagerView::NodeChanged(const mitk::DataNode* /*node*/)
{
  // m_FilterModel->invalidate();
  // fix as proposed by R. Khlebnikov in the mitk-users mail from 02.09.2014
  QMetaObject::invokeMethod( m_FilterModel, "invalidate", Qt::QueuedConnection );
}

void QmitkDataManagerView::UseWholePixelRange(mitk::DataNode* node)
{
  auto image = dynamic_cast<mitk::Image*>(node->GetData());
  if (nullptr != image)
  {
    mitk::LevelWindow levelWindow;
    levelWindow.SetToImageRange(image);
    node->SetLevelWindow(levelWindow);
  }
}

QItemSelectionModel *QmitkDataManagerView::GetDataNodeSelectionModel() const
{
  return m_NodeTreeView->selectionModel();
}

void QmitkDataManagerView::GlobalReinit( bool )
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  if (renderWindow == nullptr)
    renderWindow = this->OpenRenderWindowPart(false);

  // no render window available
  if (renderWindow == nullptr) return;

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
  auto selectedNodes = this->GetCurrentSelection();

  for (auto node : m_NodeTreeModel->GetNodeSet())
  {
    if (node.IsNotNull())
      node->SetSelected(selectedNodes.contains(node));
  }
}

void QmitkDataManagerView::OnNodeVisibilityChanged()
{
  if (m_GlobalReinitOnNodeVisibilityChanged)
  {
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(GetDataStorage());
  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkDataManagerView::ShowIn(const QString &editorId)
{
  berry::IWorkbenchPage::Pointer page = this->GetSite()->GetPage();
  berry::IEditorInput::Pointer input(new mitk::DataStorageEditorInput(this->GetDataStorageReference()));
  page->OpenEditor(input, editorId, false, berry::IWorkbenchPage::MATCH_ID);
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
