/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataNodeContextMenu.h>

#include <QmitkCustomVariants.h>
#include <QmitkFileSaveAction.h>
#include <QmitkNodeDescriptorManager.h>

#include <mitkDataNodeSelection.h>
#include <mitkIContextMenuAction.h>

#include <berryAbstractUICTKPlugin.h>
#include <berryIContributor.h>
#include <berryIExtensionRegistry.h>
#include <berryISelectionService.h>
#include <berryPlatform.h>

QmitkDataNodeContextMenu::QmitkDataNodeContextMenu(berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent)
  : QMenu(parent),
    m_Parent(parent),
    m_WorkbenchPartSite(workbenchPartSite)
{
  this->InitNodeDescriptors();
  this->InitDefaultActions();
  this->InitExtensionPointActions();
}

QmitkDataNodeContextMenu::~QmitkDataNodeContextMenu()
{
  for (auto& descriptorActionPair : m_DescriptorActionList)
    descriptorActionPair.first->RemoveAction(descriptorActionPair.second);
}

void QmitkDataNodeContextMenu::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;

  for (auto& descriptorActionPair : m_DescriptorActionList)
  {
    auto dataNodeAction = dynamic_cast<QmitkAbstractDataNodeAction*>(descriptorActionPair.second);

    if (nullptr != dataNodeAction)
      dataNodeAction->SetDataStorage(dataStorage);
  }  
}

void QmitkDataNodeContextMenu::SetBaseRenderer(mitk::BaseRenderer* baseRenderer)
{
  m_BaseRenderer = baseRenderer;

  for (auto& descriptorActionPair : m_DescriptorActionList)
  {
    auto dataNodeAction = dynamic_cast<QmitkAbstractDataNodeAction*>(descriptorActionPair.second);

    if (nullptr != dataNodeAction)
      dataNodeAction->SetBaseRenderer(baseRenderer);
  }
}

void QmitkDataNodeContextMenu::SetSurfaceDecimation(bool surfaceDecimation)
{
  m_SurfaceDecimation = surfaceDecimation;
}

void QmitkDataNodeContextMenu::SetSelectedNodes(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  m_SelectedNodes = selectedNodes;
}

void QmitkDataNodeContextMenu::InitNodeDescriptors()
{
  auto nodeDescriptorManager = QmitkNodeDescriptorManager::GetInstance();

  m_UnknownDataNodeDescriptor = nodeDescriptorManager->GetUnknownDataNodeDescriptor();
  m_ImageDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("Image");
  m_MultiComponentImageDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("MultiComponentImage");
  m_DiffusionImageDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("DiffusionImage");
  m_FiberBundleDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("FiberBundle");
  m_PeakImageDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("PeakImage");
  m_SegmentDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("Segment");
  m_SurfaceDataNodeDescriptor = nodeDescriptorManager->GetDescriptor("Surface");
  m_PointSetNodeDescriptor = nodeDescriptorManager->GetDescriptor("PointSet");
  m_PlanarLineNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarLine");
  m_PlanarCircleNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarCircle");
  m_PlanarEllipseNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarEllipse");
  m_PlanarAngleNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarAngle");
  m_PlanarFourPointAngleNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarFourPointAngle");
  m_PlanarRectangleNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarRectangle");
  m_PlanarPolygonNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarPolygon");
  m_PlanarPathNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarPath");
  m_PlanarDoubleEllipseNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarDoubleEllipse");
  m_PlanarBezierCurveNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarBezierCurve");
  m_PlanarSubdivisionPolygonNodeDescriptor = nodeDescriptorManager->GetDescriptor("PlanarSubdivisionPolygon");
}

void QmitkDataNodeContextMenu::InitDefaultActions()
{
  auto workbenchPartSite = m_WorkbenchPartSite.Lock();

  m_GlobalReinitAction = new QmitkDataNodeGlobalReinitAction(m_Parent, workbenchPartSite);
  m_GlobalReinitAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_GlobalReinitAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_GlobalReinitAction));

  m_ReinitAction = new QmitkDataNodeReinitAction(m_Parent, workbenchPartSite);
  m_ReinitAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ReinitAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ReinitAction));

  QAction* saveAction = new QmitkFileSaveAction(QIcon(":/org.mitk.gui.qt.datamanager/Save_48.png"), workbenchPartSite->GetWorkbenchWindow());
  m_UnknownDataNodeDescriptor->AddAction(saveAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, saveAction));

  m_RemoveAction = new QmitkDataNodeRemoveAction(m_Parent, workbenchPartSite);
  m_RemoveAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/Remove_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_RemoveAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_RemoveAction));

  m_ShowSelectedNodesAction = new QmitkDataNodeShowSelectedNodesAction(m_Parent, workbenchPartSite);
  m_RemoveAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/ShowSelectedNode_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ShowSelectedNodesAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ShowSelectedNodesAction));

  m_ToggleVisibilityAction = new QmitkDataNodeToggleVisibilityAction(m_Parent, workbenchPartSite);
  m_ToggleVisibilityAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/InvertShowSelectedNode_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ToggleVisibilityAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ToggleVisibilityAction));

  m_ShowDetailsAction = new QmitkDataNodeShowDetailsAction(m_Parent, workbenchPartSite);
  m_ShowDetailsAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/ShowDataInfo_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ShowDetailsAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ShowDetailsAction));

  m_OpacityAction = new QmitkDataNodeOpacityAction(m_Parent, workbenchPartSite);
  m_UnknownDataNodeDescriptor->AddAction(m_OpacityAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_OpacityAction));

  m_ColorAction = new QmitkDataNodeColorAction(m_Parent, workbenchPartSite);
  this->AddColorAction(m_ColorAction);

  m_ColormapAction = new QmitkDataNodeColorMapAction(m_Parent, workbenchPartSite);
  m_ImageDataNodeDescriptor->AddAction(m_ColormapAction);
  m_DescriptorActionList.push_back(std::make_pair(m_ImageDataNodeDescriptor, m_ColormapAction));

  if (nullptr != m_DiffusionImageDataNodeDescriptor)
  {
    m_DiffusionImageDataNodeDescriptor->AddAction(m_ColormapAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, m_ColormapAction));
  }

  m_ComponentAction = new QmitkDataNodeComponentAction(m_Parent, workbenchPartSite);
  m_MultiComponentImageDataNodeDescriptor->AddAction(m_ComponentAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_MultiComponentImageDataNodeDescriptor, m_ComponentAction));

  if (nullptr != m_DiffusionImageDataNodeDescriptor)
  {
    m_DiffusionImageDataNodeDescriptor->AddAction(m_ComponentAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, m_ComponentAction));
  }

  m_TextureInterpolationAction = new QmitkDataNodeTextureInterpolationAction(m_Parent, workbenchPartSite);
  m_ImageDataNodeDescriptor->AddAction(m_TextureInterpolationAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_ImageDataNodeDescriptor, m_TextureInterpolationAction));

  if (nullptr != m_DiffusionImageDataNodeDescriptor)
  {
    m_DiffusionImageDataNodeDescriptor->AddAction(m_TextureInterpolationAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, m_TextureInterpolationAction));
  }

  if (nullptr != m_SegmentDataNodeDescriptor)
  {
    m_SegmentDataNodeDescriptor->AddAction(m_TextureInterpolationAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_SegmentDataNodeDescriptor, m_TextureInterpolationAction));
  }

  m_SurfaceRepresentationAction = new QmitkDataNodeSurfaceRepresentationAction(m_Parent, workbenchPartSite);
  m_SurfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentationAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_SurfaceDataNodeDescriptor, m_SurfaceRepresentationAction));
}

void QmitkDataNodeContextMenu::InitExtensionPointActions()
{
  auto extensionPointService = berry::Platform::GetExtensionRegistry();
  auto customMenuConfigs = extensionPointService->GetConfigurationElementsFor("org.mitk.gui.qt.datamanager.contextMenuActions");

  DescriptorActionListType descriptorActionList;
  m_ConfigElements.clear();

  for (const auto& customMenuConfig : qAsConst(customMenuConfigs))
  {
    auto descriptorName = customMenuConfig->GetAttribute("nodeDescriptorName");
    auto actionLabel = customMenuConfig->GetAttribute("label");
    auto actionClass = customMenuConfig->GetAttribute("class");

    if (descriptorName.isEmpty() || actionLabel.isEmpty() || actionClass.isEmpty())
      continue;

    auto descriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(descriptorName);

    if (nullptr == descriptor)
    {
      MITK_WARN << "Cannot add action \"" << actionLabel << "\" to non-existent descriptor \"" << descriptorName << "\".";
      continue;
    }

    QAction* action = nullptr;
    auto actionIcon = customMenuConfig->GetAttribute("icon");

    if (!actionIcon.isEmpty())
    {
      QIcon icon = !QFile::exists(actionIcon)
        ? berry::AbstractUICTKPlugin::ImageDescriptorFromPlugin(customMenuConfig->GetContributor()->GetName(), actionIcon)
        : QIcon(actionIcon);

      action = new QAction(icon, actionLabel, m_Parent);
    }
    else
    {
      action = new QAction(actionLabel, m_Parent);
    }

    if (nullptr != action)
    {
      // See T26938. We do not know why but without the lambda function indirection, the
      // connection is lost after the content menu was shown for the first time.
      connect(action, &QAction::triggered, [action, this]()
      {
        this->OnExtensionPointActionTriggered(action);
      });

      m_ConfigElements[action] = customMenuConfig;
      descriptorActionList.push_back(std::make_pair(descriptor, action));
    }
  }

  this->AddDescriptorActionList(descriptorActionList);
}

void QmitkDataNodeContextMenu::InitServiceActions()
{
}

void QmitkDataNodeContextMenu::OnContextMenuRequested(const QPoint& /*pos*/)
{
  if (m_WorkbenchPartSite.Expired())
    return;

  auto selection = m_WorkbenchPartSite.Lock()->GetWorkbenchWindow()->GetSelectionService()->GetSelection()
    .Cast<const mitk::DataNodeSelection>();

  if (selection.IsNull() || selection->IsEmpty())
    return;

  m_SelectedNodes = QList<mitk::DataNode::Pointer>::fromStdList(selection->GetSelectedDataNodes());

  if (!m_SelectedNodes.isEmpty())
  {
    this->clear();

    auto actions = m_SelectedNodes.size() == 1
      ? this->GetActions(m_SelectedNodes.front())
      : this->GetActions(m_SelectedNodes);

    for (auto& action : actions)
    {
      auto dataNodeAction = dynamic_cast<QmitkAbstractDataNodeAction*>(action);

      if (nullptr != dataNodeAction)
        dataNodeAction->SetSelectedNodes(m_SelectedNodes);
    }

    this->addActions(actions);
    this->popup(QCursor::pos());
  }
}

void QmitkDataNodeContextMenu::OnExtensionPointActionTriggered(QAction* action)
{
  auto configElementIter = m_ConfigElements.find(action);

  if (m_ConfigElements.end() == configElementIter)
  {
    MITK_WARN << "Associated configuration element for action \"" << action->text() << "\" not found.";
    return;
  }

  auto configElement = configElementIter->second;
  auto contextMenuAction = configElement->CreateExecutableExtension<mitk::IContextMenuAction>("class");

  if (!m_DataStorage.IsExpired())
    contextMenuAction->SetDataStorage(m_DataStorage.Lock());

  if ("QmitkCreatePolygonModelAction" == configElement->GetAttribute("class"))
  {
    contextMenuAction->SetSmoothed("true" == configElement->GetAttribute("smoothed"));
    contextMenuAction->SetDecimated(m_SurfaceDecimation);
  }

  contextMenuAction->Run(m_SelectedNodes);
}

void QmitkDataNodeContextMenu::AddColorAction(QWidgetAction* colorAction)
{
  if (nullptr != m_ImageDataNodeDescriptor)
  {
    m_ImageDataNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_ImageDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_MultiComponentImageDataNodeDescriptor)
  {
    m_MultiComponentImageDataNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_MultiComponentImageDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_DiffusionImageDataNodeDescriptor)
  {
    m_DiffusionImageDataNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_FiberBundleDataNodeDescriptor)
  {
    m_FiberBundleDataNodeDescriptor->AddAction(colorAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_FiberBundleDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_PeakImageDataNodeDescriptor)
  {
    m_PeakImageDataNodeDescriptor->AddAction(colorAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_PeakImageDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_SegmentDataNodeDescriptor)
  {
    m_SegmentDataNodeDescriptor->AddAction(colorAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_SegmentDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_SurfaceDataNodeDescriptor)
  {
    m_SurfaceDataNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_SurfaceDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_PointSetNodeDescriptor)
  {
    m_PointSetNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PointSetNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarLineNodeDescriptor)
  {
    m_PlanarLineNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarLineNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarCircleNodeDescriptor)
  {
    m_PlanarCircleNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarCircleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarEllipseNodeDescriptor)
  {
    m_PlanarEllipseNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarEllipseNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarAngleNodeDescriptor)
  {
    m_PlanarAngleNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarAngleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarFourPointAngleNodeDescriptor)
  {
    m_PlanarFourPointAngleNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarFourPointAngleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarRectangleNodeDescriptor)
  {
    m_PlanarRectangleNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarRectangleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarPolygonNodeDescriptor)
  {
    m_PlanarPolygonNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarPolygonNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarPathNodeDescriptor)
  {
    m_PlanarPathNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarPathNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarDoubleEllipseNodeDescriptor)
  {
    m_PlanarDoubleEllipseNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarDoubleEllipseNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarBezierCurveNodeDescriptor)
  {
    m_PlanarBezierCurveNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarBezierCurveNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarSubdivisionPolygonNodeDescriptor)
  {
    m_PlanarSubdivisionPolygonNodeDescriptor->AddAction(colorAction, true);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarSubdivisionPolygonNodeDescriptor, colorAction));
  }
}

void QmitkDataNodeContextMenu::AddDescriptorActionList(DescriptorActionListType& descriptorActionList)
{
  using ListItem = std::pair<QmitkNodeDescriptor*, QAction*>;

  std::sort(descriptorActionList.begin(), descriptorActionList.end(), [](const ListItem& left, const ListItem& right) -> bool
  {
    return left.second->text() < right.second->text();
  });

  for (auto& descriptorActionPair : descriptorActionList)
  {
    descriptorActionPair.first->AddAction(descriptorActionPair.second);
    m_DescriptorActionList.push_back(descriptorActionPair);
  }
}

QList<QAction*> QmitkDataNodeContextMenu::GetActions(const mitk::DataNode* node)
{
  QList<QAction*> actions;

  for(const auto& descriptorActionPair : m_DescriptorActionList)
  {
    if (descriptorActionPair.first->CheckNode(node) || "Unknown" == descriptorActionPair.first->GetNameOfClass())
      actions.append(descriptorActionPair.second);
  }

  return actions;
}

QList<QAction*> QmitkDataNodeContextMenu::GetActions(const QList<mitk::DataNode::Pointer>& nodes)
{
  QList<QAction*> actions;

  for (const auto& descriptorActionPair : m_DescriptorActionList)
  {
    for (const auto& node : nodes)
    {
      if (descriptorActionPair.first->CheckNode(node) || "Unknown" == descriptorActionPair.first->GetNameOfClass())
      {
        auto batchActions = descriptorActionPair.first->GetBatchActions();

        if (std::find(batchActions.begin(), batchActions.end(), descriptorActionPair.second) != batchActions.end())
          actions.append(descriptorActionPair.second);

        break;
      }
    }
  }

  return actions;
}
