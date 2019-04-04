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

#include "QmitkDataNodeContextMenu.h"

// mitk gui qt application plugin
#include "mitkIContextMenuAction.h"
#include "QmitkFileSaveAction.h"

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt widgets module
#include <QmitkCustomVariants.h>
#include <QmitkNodeDescriptorManager.h>

// berry
#include <berryAbstractUICTKPlugin.h>
#include <berryIContributor.h>
#include <berryIExtensionRegistry.h>
#include <berryISelectionService.h>
#include <berryPlatform.h>

QmitkDataNodeContextMenu::QmitkDataNodeContextMenu(berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent)
  : QMenu(parent)
{
  m_Parent = parent;
  m_WorkbenchPartSite = workbenchPartSite;

  InitNodeDescriptors();
  InitDefaultActions();
  InitExtensionPointActions();
}

QmitkDataNodeContextMenu::~QmitkDataNodeContextMenu()
{
  // remove the registered actions from each descriptor
  for (DescriptorActionListType::const_iterator it = m_DescriptorActionList.begin(); it != m_DescriptorActionList.end(); ++it)
  {
    (it->first)->RemoveAction(it->second);
  }
}

void QmitkDataNodeContextMenu::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage - also for all actions
    m_DataStorage = dataStorage;
    for (DescriptorActionListType::const_iterator it = m_DescriptorActionList.begin(); it != m_DescriptorActionList.end(); ++it)
    {
      QmitkAbstractDataNodeAction* abstractDataNodeAction = dynamic_cast<QmitkAbstractDataNodeAction*>(it->second);
      if(nullptr != abstractDataNodeAction)
      {
        abstractDataNodeAction->SetDataStorage(m_DataStorage.Lock());
      }
    }
  }
}

void QmitkDataNodeContextMenu::SetBaseRenderer(mitk::BaseRenderer* baseRenderer)
{
  if (m_BaseRenderer != baseRenderer)
  {
    // set the new base renderer - also for all actions
    m_BaseRenderer = baseRenderer;
    for (DescriptorActionListType::const_iterator it = m_DescriptorActionList.begin(); it != m_DescriptorActionList.end(); ++it)
    {
      QmitkAbstractDataNodeAction *abstractDataNodeAction = dynamic_cast<QmitkAbstractDataNodeAction *>(it->second);
      if (nullptr != abstractDataNodeAction)
      {
        abstractDataNodeAction->SetBaseRenderer(m_BaseRenderer.Lock());
      }
    }
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
  m_UnknownDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetUnknownDataNodeDescriptor();
  m_ImageDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Image");
  m_MultiComponentImageDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("MultiComponentImage");
  m_DiffusionImageDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("DiffusionImage");
  m_FiberBundleDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("FiberBundle");
  m_PeakImageDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PeakImage");
  m_SegmentDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Segment");
  m_SurfaceDataNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("Surface");
  m_PointSetNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PointSet");
  m_PlanarLineNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarLine");
  m_PlanarCircleNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarCircle");
  m_PlanarEllipseNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarEllipse");
  m_PlanarAngleNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarAngle");
  m_PlanarFourPointAngleNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarFourPointAngle");
  m_PlanarRectangleNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarRectangle");
  m_PlanarPolygonNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarPolygon");
  m_PlanarPathNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarPath");
  m_PlanarDoubleEllipseNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarDoubleEllipse");
  m_PlanarBezierCurveNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarBezierCurve");
  m_PlanarSubdivisionPolygonNodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor("PlanarSubdivisionPolygon");
}

void QmitkDataNodeContextMenu::InitDefaultActions()
{
  m_GlobalReinitAction = new QmitkDataNodeGlobalReinitAction(m_Parent, m_WorkbenchPartSite.Lock());
  m_GlobalReinitAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_GlobalReinitAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_GlobalReinitAction));

  m_ReinitAction = new QmitkDataNodeReinitAction(m_Parent, m_WorkbenchPartSite.Lock());
  m_ReinitAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/Refresh_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ReinitAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ReinitAction));

  QAction* saveAction = new QmitkFileSaveAction(QIcon(":/org.mitk.gui.qt.datamanager/Save_48.png"), m_WorkbenchPartSite.Lock()->GetWorkbenchWindow());
  m_UnknownDataNodeDescriptor->AddAction(saveAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, saveAction));

  m_RemoveAction = new QmitkDataNodeRemoveAction(m_Parent, m_WorkbenchPartSite.Lock());
  m_RemoveAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/Remove_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_RemoveAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_RemoveAction));

  m_ShowSelectedNodesAction = new QmitkDataNodeShowSelectedNodesAction(m_Parent, m_WorkbenchPartSite.Lock());
  m_RemoveAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/ShowSelectedNode_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ShowSelectedNodesAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ShowSelectedNodesAction));

  m_ToggleVisibilityAction = new QmitkDataNodeToggleVisibilityAction(m_Parent, m_WorkbenchPartSite.Lock());
  m_ToggleVisibilityAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/InvertShowSelectedNode_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ToggleVisibilityAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ToggleVisibilityAction));

  m_ShowDetailsAction = new QmitkDataNodeShowDetailsAction(m_Parent, m_WorkbenchPartSite.Lock());
  m_ShowDetailsAction->setIcon(QIcon(":/org.mitk.gui.qt.datamanager/ShowDataInfo_48.png"));
  m_UnknownDataNodeDescriptor->AddAction(m_ShowDetailsAction, true);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_ShowDetailsAction));

  // default widget actions
  m_OpacityAction = new QmitkDataNodeOpacityAction(m_Parent, m_WorkbenchPartSite.Lock());
  // not used for batch actions (no multi-selection action)
  m_UnknownDataNodeDescriptor->AddAction(m_OpacityAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_UnknownDataNodeDescriptor, m_OpacityAction));

  m_ColorAction = new QmitkDataNodeColorAction(m_Parent, m_WorkbenchPartSite.Lock());
  AddColorAction(m_ColorAction);

  m_ColormapAction = new QmitkDataNodeColorMapAction(m_Parent, m_WorkbenchPartSite.Lock());
  // used for batch actions
  m_ImageDataNodeDescriptor->AddAction(m_ColormapAction);
  m_DescriptorActionList.push_back(std::make_pair(m_ImageDataNodeDescriptor, m_ColormapAction));

  if (m_DiffusionImageDataNodeDescriptor != nullptr)
  {
    // not used for batch actions (no multi-selection action)
    m_DiffusionImageDataNodeDescriptor->AddAction(m_ColormapAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, m_ColormapAction));
  }

  m_ComponentAction = new QmitkDataNodeComponentAction(m_Parent, m_WorkbenchPartSite.Lock());
  // not used for batch actions (no multi-selection action)
  m_MultiComponentImageDataNodeDescriptor->AddAction(m_ComponentAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_MultiComponentImageDataNodeDescriptor, m_ComponentAction));

  if (m_DiffusionImageDataNodeDescriptor != nullptr)
  {
    // not used for batch actions (no multi-selection action)
    m_DiffusionImageDataNodeDescriptor->AddAction(m_ComponentAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, m_ComponentAction));
  }

  m_TextureInterpolationAction = new QmitkDataNodeTextureInterpolationAction(m_Parent, m_WorkbenchPartSite.Lock());
  // not used for batch actions (no multi-selection action)
  m_ImageDataNodeDescriptor->AddAction(m_TextureInterpolationAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_ImageDataNodeDescriptor, m_TextureInterpolationAction));

  if (m_DiffusionImageDataNodeDescriptor != nullptr)
  {
    // not used for batch actions (no multi-selection action)
    m_DiffusionImageDataNodeDescriptor->AddAction(m_TextureInterpolationAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_DiffusionImageDataNodeDescriptor, m_TextureInterpolationAction));
  }

  if (m_SegmentDataNodeDescriptor != nullptr)
  {
    // not used for batch actions (no multi-selection action)
    m_SegmentDataNodeDescriptor->AddAction(m_TextureInterpolationAction, false);
    m_DescriptorActionList.push_back(std::make_pair(m_SegmentDataNodeDescriptor, m_TextureInterpolationAction));
  }

  m_SurfaceRepresentationAction = new QmitkDataNodeSurfaceRepresentationAction(m_Parent, m_WorkbenchPartSite.Lock());
  // not used for batch actions (no multi-selection action)
  m_SurfaceDataNodeDescriptor->AddAction(m_SurfaceRepresentationAction, false);
  m_DescriptorActionList.push_back(std::make_pair(m_SurfaceDataNodeDescriptor, m_SurfaceRepresentationAction));
}

void QmitkDataNodeContextMenu::InitExtensionPointActions()
{
  // find contextMenuAction extension points and add them to the node descriptor
  berry::IExtensionRegistry* extensionPointService = berry::Platform::GetExtensionRegistry();
  QList<berry::IConfigurationElement::Pointer> customMenuConfigs = extensionPointService->GetConfigurationElementsFor("org.mitk.gui.qt.datamanager.contextMenuActions");

  // prepare all custom QActions
  m_ConfigElements.clear();
  DescriptorActionListType descriptorActionList;
  for (const auto& customMenuConfig : customMenuConfigs)
  {
    QString actionNodeDescriptorName = customMenuConfig->GetAttribute("nodeDescriptorName");
    QString actionLabel = customMenuConfig->GetAttribute("label");
    QString actionClass = customMenuConfig->GetAttribute("class");

    if (actionNodeDescriptorName.isEmpty() || actionLabel.isEmpty() || actionClass.isEmpty())
    {
      // no properties found for the current extension point
      continue;
    }

    // find matching descriptor
    auto nodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(actionNodeDescriptorName);
    if (nullptr == nodeDescriptor)
    {
      MITK_WARN << "Cannot add action \"" << actionLabel << "\" because descriptor " << actionNodeDescriptorName << " does not exist.";
      continue;
    }

    // create action with or without icon
    QAction* contextMenuAction;
    QString actionIconName = customMenuConfig->GetAttribute("icon");
    if (!actionIconName.isEmpty())
    {
      QIcon actionIcon;
      if (QFile::exists(actionIconName))
      {
        actionIcon = QIcon(actionIconName);
      }
      else
      {
        actionIcon = berry::AbstractUICTKPlugin::ImageDescriptorFromPlugin(customMenuConfig->GetContributor()->GetName(), actionIconName);
      }
      contextMenuAction = new QAction(actionIcon, actionLabel, m_Parent);
    }
    else
    {
      contextMenuAction = new QAction(actionLabel, m_Parent);
    }

    // connect action trigger
    connect(contextMenuAction, static_cast<void(QAction::*)(bool)>(&QAction::triggered), this, &QmitkDataNodeContextMenu::OnExtensionPointActionTriggered);

    // mark configuration element into lookup list for context menu handler
    m_ConfigElements[contextMenuAction] = customMenuConfig;
    // mark new action in sortable list for addition to descriptor
    descriptorActionList.emplace_back(nodeDescriptor, contextMenuAction);
  }

  AddDescriptorActionList(descriptorActionList);
}

void QmitkDataNodeContextMenu::InitServiceActions()
{
}

void QmitkDataNodeContextMenu::OnContextMenuRequested(const QPoint& /*pos*/)
{
  if (m_WorkbenchPartSite.Expired())
  {
    return;
  }

  if (m_SelectedNodes.isEmpty())
  {
    // no selection set - retrieve selection from the workbench selection service
    m_SelectedNodes = AbstractDataNodeAction::GetSelectedNodes(m_WorkbenchPartSite.Lock());
  }

  // if either a selection was set or a selection could be retrieved from the workbench selection service
  berry::ISelection::ConstPointer selection = m_WorkbenchPartSite.Lock()->GetWorkbenchWindow()->GetSelectionService()->GetSelection();
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();

  if (currentSelection.IsNull() || currentSelection->IsEmpty())
  {
    return;
  }

  m_SelectedNodes = QList<mitk::DataNode::Pointer>::fromStdList(currentSelection->GetSelectedDataNodes());
  if (!m_SelectedNodes.isEmpty())
  {
    clear();
    QList<QAction*> actions;
    if (m_SelectedNodes.size() == 1)
    {
      // no batch action; should only contain a single node
      actions = GetActions(m_SelectedNodes.front());
    }
    else
    {
      // batch action
      actions = GetActions(m_SelectedNodes);
    }

    // initialize abstract data node actions
    for (auto& action : actions)
    {
      QmitkAbstractDataNodeAction* abstractDataNodeAction = dynamic_cast<QmitkAbstractDataNodeAction*>(action);
      if (nullptr != abstractDataNodeAction)
      {
        abstractDataNodeAction->SetSelectedNodes(m_SelectedNodes);
      }
    }
    addActions(actions);
    popup(QCursor::pos());
  }
}

void QmitkDataNodeContextMenu::OnExtensionPointActionTriggered(bool)
{
  QAction* action = qobject_cast<QAction*>(sender());
  ConfigurationElementsType::const_iterator it = m_ConfigElements.find(action);
  if (it == m_ConfigElements.end())
  {
    MITK_WARN << "Associated configuration element for action " << action->text().toStdString() << " not found.";
    return;
  }

  berry::IConfigurationElement::Pointer confElem = it->second;
  mitk::IContextMenuAction* contextMenuAction = confElem->CreateExecutableExtension<mitk::IContextMenuAction>("class");

  QString className = confElem->GetAttribute("class");
  QString smoothed = confElem->GetAttribute("smoothed");

  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
    contextMenuAction->SetDataStorage(dataStorage);
  }

  if (className == "QmitkCreatePolygonModelAction")
  {
    if (smoothed == "false")
    {
      contextMenuAction->SetSmoothed(false);
    }
    else
    {
      contextMenuAction->SetSmoothed(true);
    }
    contextMenuAction->SetDecimated(m_SurfaceDecimation);
  }

  contextMenuAction->Run(m_SelectedNodes);
}

void QmitkDataNodeContextMenu::AddColorAction(QWidgetAction* colorAction)
{
  bool colorActionCanBatch = true;
  if (nullptr != m_ImageDataNodeDescriptor)
  {
    m_ImageDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_ImageDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_MultiComponentImageDataNodeDescriptor)
  {
    m_MultiComponentImageDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_MultiComponentImageDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_DiffusionImageDataNodeDescriptor)
  {
    m_DiffusionImageDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
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
    m_SurfaceDataNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_SurfaceDataNodeDescriptor, colorAction));
  }

  if (nullptr != m_PointSetNodeDescriptor)
  {
    m_PointSetNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PointSetNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarLineNodeDescriptor)
  {
    m_PlanarLineNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarLineNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarCircleNodeDescriptor)
  {
    m_PlanarCircleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarCircleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarEllipseNodeDescriptor)
  {
    m_PlanarEllipseNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarEllipseNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarAngleNodeDescriptor)
  {
    m_PlanarAngleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarAngleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarFourPointAngleNodeDescriptor)
  {
    m_PlanarFourPointAngleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarFourPointAngleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarRectangleNodeDescriptor)
  {
    m_PlanarRectangleNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarRectangleNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarPolygonNodeDescriptor)
  {
    m_PlanarPolygonNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarPolygonNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarPathNodeDescriptor)
  {
    m_PlanarPathNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarPathNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarDoubleEllipseNodeDescriptor)
  {
    m_PlanarDoubleEllipseNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarDoubleEllipseNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarBezierCurveNodeDescriptor)
  {
    m_PlanarBezierCurveNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarBezierCurveNodeDescriptor, colorAction));
  }

  if (nullptr != m_PlanarSubdivisionPolygonNodeDescriptor)
  {
    m_PlanarSubdivisionPolygonNodeDescriptor->AddAction(colorAction, colorActionCanBatch);
    m_DescriptorActionList.push_back(std::make_pair(m_PlanarSubdivisionPolygonNodeDescriptor, colorAction));
  }
}

void QmitkDataNodeContextMenu::AddDescriptorActionList(DescriptorActionListType& descriptorActionList)
{
  // sort all custom QActions by their texts
  using ListEntryType = std::pair<QmitkNodeDescriptor*, QAction*>;
  std::sort(descriptorActionList.begin(), descriptorActionList.end(),
    [](const ListEntryType& left, const ListEntryType& right) -> bool
  {
    assert(left.second != nullptr && right.second != nullptr); // unless we messed up above
    return left.second->text() < right.second->text();
  });

  // add custom QActions in sorted order
  for (auto& descriptorActionPair : descriptorActionList)
  {
    auto& nodeDescriptor = descriptorActionPair.first;
    auto& contextMenuAction = descriptorActionPair.second;

    // add action to the descriptor
    nodeDescriptor->AddAction(contextMenuAction);

    // mark new action into list of descriptors to remove in destructor
    m_DescriptorActionList.push_back(descriptorActionPair);
  }
}

QList<QAction*> QmitkDataNodeContextMenu::GetActions(const mitk::DataNode* node)
{
  QList<QAction*> actions;
  for (DescriptorActionListType::const_iterator it = m_DescriptorActionList.begin(); it != m_DescriptorActionList.end(); ++it)
  {
    if ((it->first)->CheckNode(node) ||
         it->first->GetNameOfClass() == "Unknown")
    {
      actions.append(it->second);
    }
  }

  return actions;
}

QList<QAction*> QmitkDataNodeContextMenu::GetActions(const QList<mitk::DataNode::Pointer>& nodes)
{
  QList<QAction*> actions;

  for (DescriptorActionListType::const_iterator it = m_DescriptorActionList.begin(); it != m_DescriptorActionList.end(); ++it)
  {
    for (const auto& node : nodes)
    {
      if ((it->first)->CheckNode(node) ||
           it->first->GetNameOfClass() == "Unknown")
      {
        const auto& batchActions = (it->first)->GetBatchActions();
        if (std::find(batchActions.begin(), batchActions.end(), it->second) != batchActions.end())
        {
          // current descriptor action is a batch action
          actions.append(it->second);
        }
        break; // only add action once; goto next descriptor action
      }
    }
  }

  return actions;
}
