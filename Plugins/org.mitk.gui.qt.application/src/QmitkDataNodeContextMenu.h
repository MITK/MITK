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

#ifndef QMITKDATANODECONTEXTMENU_H
#define QMITKDATANODECONTEXTMENU_H

#include <org_mitk_gui_qt_application_Export.h>

// qt widgets module
#include "QmitkDataNodeGlobalReinitAction.h"
#include "QmitkDataNodeReinitAction.h"
#include "QmitkDataNodeRemoveAction.h"
#include "QmitkDataNodeShowSelectedNodesAction.h"
#include "QmitkDataNodeToggleVisibilityAction.h"
#include "QmitkDataNodeShowDetailsAction.h"
#include "QmitkDataNodeOpacityAction.h"
#include "QmitkDataNodeColorAction.h"
#include "QmitkDataNodeColorMapAction.h"
#include "QmitkDataNodeComponentAction.h"
#include "QmitkDataNodeTextureInterpolationAction.h"
#include "QmitkDataNodeSurfaceRepresentationAction.h"
#include "QmitkNodeDescriptor.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// blueberry ui qt plugin
#include <berryIConfigurationElement.h>
#include <berryIWorkbenchPartSite.h>

// qt
#include <QMenu>

class MITK_QT_APP QmitkDataNodeContextMenu : public QMenu
{
  Q_OBJECT

public:

  QmitkDataNodeContextMenu(berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent = nullptr);

  virtual ~QmitkDataNodeContextMenu() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetBaseRenderer(mitk::BaseRenderer* baseRenderer);

  void SetSurfaceDecimation(bool surfaceDecimation);

  void SetSelectedNodes(const QList<mitk::DataNode::Pointer>& selectedNodes);

public Q_SLOTS:

  void OnContextMenuRequested(const QPoint& pos);
  void OnExtensionPointActionTriggered(bool);

private:

  using DescriptorActionListType = std::vector<std::pair<QmitkNodeDescriptor*, QAction*>>;
  using ConfigurationElementsType = std::map<QAction*, berry::IConfigurationElement::Pointer>;

  void InitNodeDescriptors();
  void InitDefaultActions();
  void InitExtensionPointActions();
  void InitServiceActions();

  void AddColorAction(QWidgetAction* colorAction);
  void AddDescriptorActionList(DescriptorActionListType& descriptorActionList);

  QList<QAction*> GetActions(const mitk::DataNode* node);
  QList<QAction*> GetActions(const QList<mitk::DataNode::Pointer>& nodes);

  QWidget* m_Parent;
  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;
  QList<mitk::DataNode::Pointer> m_SelectedNodes;

  // store a list of all actions to remove them on menu destruction
  DescriptorActionListType m_DescriptorActionList;

  // stores the configuration elements for the context menu actions from extension points
  ConfigurationElementsType m_ConfigElements;

  QmitkNodeDescriptor* m_UnknownDataNodeDescriptor;
  QmitkNodeDescriptor* m_ImageDataNodeDescriptor;
  QmitkNodeDescriptor* m_MultiComponentImageDataNodeDescriptor;
  QmitkNodeDescriptor* m_DiffusionImageDataNodeDescriptor;
  QmitkNodeDescriptor* m_FiberBundleDataNodeDescriptor;
  QmitkNodeDescriptor* m_PeakImageDataNodeDescriptor;
  QmitkNodeDescriptor* m_SegmentDataNodeDescriptor;
  QmitkNodeDescriptor* m_SurfaceDataNodeDescriptor;
  QmitkNodeDescriptor* m_PointSetNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarLineNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarCircleNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarEllipseNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarAngleNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarFourPointAngleNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarRectangleNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarPolygonNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarPathNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarDoubleEllipseNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarBezierCurveNodeDescriptor;
  QmitkNodeDescriptor* m_PlanarSubdivisionPolygonNodeDescriptor;

  //////////////////////////////////////////////////////////////////////////
  // default actions
  //////////////////////////////////////////////////////////////////////////
  QmitkDataNodeGlobalReinitAction* m_GlobalReinitAction;
  QmitkDataNodeReinitAction* m_ReinitAction;
  QmitkDataNodeRemoveAction* m_RemoveAction;
  QmitkDataNodeShowSelectedNodesAction* m_ShowSelectedNodesAction;
  QmitkDataNodeToggleVisibilityAction* m_ToggleVisibilityAction;
  QmitkDataNodeShowDetailsAction* m_ShowDetailsAction;
  QmitkDataNodeOpacityAction* m_OpacityAction;
  QmitkDataNodeColorAction* m_ColorAction;
  QmitkDataNodeColorMapAction* m_ColormapAction;
  QmitkDataNodeComponentAction* m_ComponentAction;
  QmitkDataNodeTextureInterpolationAction* m_TextureInterpolationAction;
  QmitkDataNodeSurfaceRepresentationAction* m_SurfaceRepresentationAction;

  bool m_SurfaceDecimation;
};

#endif // QMITKDATANODECONTEXTMENU_H
