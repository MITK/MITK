/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataManagerView.h"

// mitk gui qt datamanager
#include "internal/QmitkDataManagerItemDelegate.h"
#include "internal/QmitkNodeTableViewKeyFilter.h"


// mitk core
#include <mitkCommon.h>
#include <mitkCoreObjectFactory.h>
#include <mitkEnumerationProperty.h>

#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingModeProperty.h>
#include <mitkIPreferences.h>

// qt widgets module
#include <QmitkCustomVariants.h>
#include <QmitkDataStorageFilterProxyModel.h>
#include <QmitkDataStorageTreeModel.h>
#include <QmitkIOUtil.h>
#include <QmitkNodeDescriptorManager.h>

// mitk core services plugin
#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>

// mitk gui common plugin
#include <mitkDataNodeObject.h>

// mitk gui qt application plugin
#include <QmitkDataNodeGlobalReinitAction.h>
#include <QmitkDataNodeToggleVisibilityAction.h>

// mitk gui qt common plugin
#include <QmitkDnDFrameWidget.h>

// qt
#include <QGridLayout>
#include <QVBoxLayout>
#include <QTreeView>

const QString QmitkDataManagerView::VIEW_ID = "org.mitk.views.datamanager";

QmitkDataManagerView::QmitkDataManagerView()
  : m_ItemDelegate(nullptr)
{
}

QmitkDataManagerView::~QmitkDataManagerView()
{
  // nothing here
}

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  m_CurrentRowCount = 0;
  m_Parent = parent;

  auto* prefs = this->GetPreferences();
  assert(prefs != nullptr);

  m_NodeTreeModel = new QmitkDataStorageTreeModel(GetDataStorage(), prefs->GetBool("Place new nodes on top", true));
  m_NodeTreeModel->setParent(parent);
  m_NodeTreeModel->SetAllowHierarchyChange(prefs->GetBool("Allow changing of parent node", false));

  // Prepare filters
  m_HelperObjectFilterPredicate = mitk::NodePredicateOr::New(
    mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)),
    mitk::NodePredicateProperty::New("hidden object", mitk::BoolProperty::New(true)));
  m_NodeWithNoDataFilterPredicate = mitk::NodePredicateData::New(nullptr);

  m_FilterModel = new QmitkDataStorageFilterProxyModel();
  m_FilterModel->setSourceModel(m_NodeTreeModel);
  m_FilterModel->AddFilterPredicate(m_HelperObjectFilterPredicate);
  m_FilterModel->AddFilterPredicate(m_NodeWithNoDataFilterPredicate);

  m_NodeTreeView = new QTreeView;
  m_NodeTreeView->setHeaderHidden(true);
  m_NodeTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_NodeTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_NodeTreeView->setAlternatingRowColors(true);
  m_NodeTreeView->setDragEnabled(true);
  m_NodeTreeView->setDropIndicatorShown(true);
  m_NodeTreeView->setAcceptDrops(true);
  m_NodeTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  m_NodeTreeView->setModel(m_FilterModel);
  m_NodeTreeView->setTextElideMode(Qt::ElideMiddle);
  m_NodeTreeView->installEventFilter(new QmitkNodeTableViewKeyFilter(this, GetDataStorage()));

  m_ItemDelegate = new QmitkDataManagerItemDelegate(m_NodeTreeView);
  m_NodeTreeView->setItemDelegate(m_ItemDelegate);

  connect(m_NodeTreeModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(NodeTreeViewRowsInserted(const QModelIndex&, int, int)));
  connect(m_NodeTreeModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(NodeTreeViewRowsRemoved(const QModelIndex&, int, int)));
  connect(m_NodeTreeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(NodeSelectionChanged(const QItemSelection &, const QItemSelection &)));
  connect(m_NodeTreeModel, &QmitkDataStorageTreeModel::nodeVisibilityChanged, this, &QmitkDataManagerView::OnNodeVisibilityChanged);

  // data node context menu and menu actions
  m_DataNodeContextMenu = new QmitkDataNodeContextMenu(GetSite(), m_NodeTreeView);
  m_DataNodeContextMenu->SetDataStorage(GetDataStorage());
  connect(m_NodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), m_DataNodeContextMenu, SLOT(OnContextMenuRequested(const QPoint&)));

  QGridLayout* dndFrameWidgetLayout = new QGridLayout;
  dndFrameWidgetLayout->addWidget(m_NodeTreeView, 0, 0);
  dndFrameWidgetLayout->setContentsMargins(0, 0, 0, 0);

  m_DnDFrameWidget = new QmitkDnDFrameWidget(m_Parent);
  m_DnDFrameWidget->setLayout(dndFrameWidgetLayout);

  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->addWidget(m_DnDFrameWidget);
  layout->setContentsMargins(0, 0, 0, 0);

  m_Parent->setLayout(layout);
}

void QmitkDataManagerView::SetFocus()
{
}

//////////////////////////////////////////////////////////////////////////
// Node tree modification
//////////////////////////////////////////////////////////////////////////
void QmitkDataManagerView::NodeTreeViewRowsInserted(const QModelIndex& parent, int /*start*/, int /*end*/)
{
  QModelIndex viewIndex = m_FilterModel->mapFromSource(parent);
  m_NodeTreeView->setExpanded(viewIndex, true);

  // a new row was inserted
  if (m_CurrentRowCount == 0 && m_NodeTreeModel->rowCount() == 1)
  {
    mitk::WorkbenchUtil::OpenRenderWindowPart(GetSite()->GetPage());
    m_CurrentRowCount = m_NodeTreeModel->rowCount();
  }
}

void QmitkDataManagerView::NodeTreeViewRowsRemoved(const QModelIndex& /*parent*/, int /*start*/, int /*end*/)
{
  m_CurrentRowCount = m_NodeTreeModel->rowCount();
}

void QmitkDataManagerView::NodeSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
  auto selectedNodes = GetCurrentSelection();
  auto nodeSet = m_NodeTreeModel->GetNodeSet();

  for (auto node : std::as_const(nodeSet))
  {
    if (node.IsNotNull())
    {
      node->SetSelected(selectedNodes.contains(node));
    }
  }
  m_DataNodeContextMenu->SetSelectedNodes(selectedNodes);
}

void QmitkDataManagerView::OnNodeVisibilityChanged()
{
  ToggleVisibilityAction::Run(GetSite(), GetDataStorage(), QList<mitk::DataNode::Pointer>());
}

void QmitkDataManagerView::NodeChanged(const mitk::DataNode* /*node*/)
{
  // m_FilterModel->invalidate();
  // fix as proposed by R. Khlebnikov in the mitk-users mail from 02.09.2014
  QMetaObject::invokeMethod(m_FilterModel, "invalidate", Qt::QueuedConnection);
}

void QmitkDataManagerView::OnPreferencesChanged(const mitk::IPreferences* prefs)
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
  m_NodeTreeView->expandAll();

  m_NodeTreeModel->SetAllowHierarchyChange(prefs->GetBool("Allow changing of parent node", false));

  GlobalReinitAction::Run(GetSite(), GetDataStorage());
}

QItemSelectionModel* QmitkDataManagerView::GetDataNodeSelectionModel() const
{
  return m_NodeTreeView->selectionModel();
}
