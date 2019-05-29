/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// data storage viewer test plugin
#include "QmitkDataStorageViewerTestView.h"

#include "mitkNodePredicateDataType.h"

// berry
#include <berryIWorkbenchWindow.h>

// qt
#include <QHBoxLayout>

const std::string QmitkDataStorageViewerTestView::VIEW_ID = "org.mitk.views.datastorageviewertest";

void QmitkDataStorageViewerTestView::SetFocus()
{
  // nothing here
}

void QmitkDataStorageViewerTestView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_DataStorageDefaultListModel = new QmitkDataStorageDefaultListModel(this);
  m_DataStorageDefaultListModel->SetDataStorage(GetDataStorage());
  m_Controls.selectionListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.selectionListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.selectionListView->setAlternatingRowColors(true);
  m_Controls.selectionListView->setModel(m_DataStorageDefaultListModel);

  m_DataStorageDefaultListModel2 = new QmitkDataStorageDefaultListModel(this);
  m_DataStorageDefaultListModel2->SetDataStorage(GetDataStorage());
  m_Controls.selectionListView2->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.selectionListView2->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.selectionListView2->setAlternatingRowColors(true);
  m_Controls.selectionListView2->setModel(m_DataStorageDefaultListModel2);

  m_Controls.singleSlot->SetDataStorage(GetDataStorage());
  m_Controls.singleSlot->SetEmptyInfo(QString("EmptyInfo: Set this to display info in empty state"));
  m_Controls.singleSlot->SetInvalidInfo(QString("InvalidInfo: is displayed for invalid states"));
  m_Controls.singleSlot->SetPopUpTitel(QString("This is the definable caption. Choose your data now!"));
  m_Controls.singleSlot->SetPopUpHint(QString("I am an optional hint, that can be set by the developer<p/>If not set the widget is invisible."));

  m_Controls.multiSlot->SetDataStorage(GetDataStorage());
  m_Controls.multiSlot->SetEmptyInfo(QString("EmptyInfo: Set this to display info in empty state"));
  m_Controls.multiSlot->SetInvalidInfo(QString("InvalidInfo: is displayed for invalid states"));
  m_Controls.multiSlot->SetPopUpTitel(QString("This is the definable caption. Choose your data now!"));
  m_Controls.multiSlot->SetPopUpHint(QString("I am an optional hint, that can be set by the developer<p/>If not set the widget is invisible."));

  m_ModelViewSelectionConnector = std::make_unique<QmitkModelViewSelectionConnector>();
  try
  {
    m_ModelViewSelectionConnector->SetView(m_Controls.selectionListView);
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Cannot connect the model-view pair signals and slots.";
  }
  m_SelectionServiceConnector = std::make_unique<QmitkSelectionServiceConnector>();

  m_ModelViewSelectionConnector2 = std::make_unique<QmitkModelViewSelectionConnector>();
  try
  {
    m_ModelViewSelectionConnector2->SetView(m_Controls.selectionListView2);
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Cannot connect the model-view pair signals and slots.";
  }
  m_SelectionServiceConnector2 = std::make_unique<QmitkSelectionServiceConnector>();

  m_SelectionServiceConnector3 = std::make_unique<QmitkSelectionServiceConnector>();
  m_SelectionServiceConnector4 = std::make_unique<QmitkSelectionServiceConnector>();

  connect(m_Controls.selectionProviderCheckBox, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider1(bool)));
  connect(m_Controls.selectionProviderCheckBox2, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider2(bool)));

  connect(m_Controls.selectionListenerCheckBox, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionListener1(bool)));
  connect(m_Controls.selectionListenerCheckBox2, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionListener2(bool)));

  connect(m_Controls.selectionProviderCheckBox3, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider3(bool)));
  connect(m_Controls.selectionListenerCheckBox3, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionListener3(bool)));

  connect(m_Controls.checkOnlyVisible, SIGNAL(toggled(bool)), m_Controls.singleSlot, SLOT(SetSelectOnlyVisibleNodes(bool)));
  connect(m_Controls.checkOptional, SIGNAL(toggled(bool)), m_Controls.singleSlot, SLOT(SetSelectionIsOptional(bool)));
  connect(m_Controls.checkOnlyImages, SIGNAL(toggled(bool)), this, SLOT(OnOnlyImages(bool)));

  connect(m_Controls.selectionProviderCheckBox4, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionProvider4(bool)));
  connect(m_Controls.selectionListenerCheckBox4, SIGNAL(toggled(bool)), this, SLOT(SetAsSelectionListener4(bool)));

  connect(m_Controls.checkOnlyVisible_2, SIGNAL(toggled(bool)), m_Controls.multiSlot, SLOT(SetSelectOnlyVisibleNodes(bool)));
  connect(m_Controls.checkOptional_2, SIGNAL(toggled(bool)), m_Controls.multiSlot, SLOT(SetSelectionIsOptional(bool)));
  connect(m_Controls.checkOnlyImages_2, SIGNAL(toggled(bool)), this, SLOT(OnOnlyImages2(bool)));
  connect(m_Controls.checkOnlyUneven, SIGNAL(toggled(bool)), this, SLOT(OnOnlyUneven(bool)));
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider1(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_ModelViewSelectionConnector.get(), SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
  else
  {
    m_SelectionServiceConnector->RemoveAsSelectionProvider();
    disconnect(m_ModelViewSelectionConnector.get(), SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener1(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector.get(), SIGNAL(ServiceSelectionChanged(QList<mitk::DataNode::Pointer>)), m_ModelViewSelectionConnector.get(), SLOT(SetCurrentSelection(QList<mitk::DataNode::Pointer>)));
  }
  else
  {
    m_SelectionServiceConnector->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector.get(), SIGNAL(ServiceSelectionChanged(QList<mitk::DataNode::Pointer>)), m_ModelViewSelectionConnector.get(), SLOT(SetCurrentSelection(QList<mitk::DataNode::Pointer>)));

  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider2(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector2->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_ModelViewSelectionConnector2.get(), SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector2.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
  else
  {
    m_SelectionServiceConnector2->RemoveAsSelectionProvider();
    disconnect(m_ModelViewSelectionConnector2.get(), SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector2.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener2(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector2->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector2.get(), SIGNAL(ServiceSelectionChanged(QList<mitk::DataNode::Pointer>)), m_ModelViewSelectionConnector2.get(), SLOT(SetCurrentSelection(QList<mitk::DataNode::Pointer>)));
  }
  else
  {
    m_SelectionServiceConnector2->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector2.get(), SIGNAL(ServiceSelectionChanged(QList<mitk::DataNode::Pointer>)), m_ModelViewSelectionConnector2.get(), SLOT(SetCurrentSelection(QList<mitk::DataNode::Pointer>)));
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider3(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector3->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_Controls.singleSlot, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector3.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
  else
  {
    m_SelectionServiceConnector3->RemoveAsSelectionProvider();
    disconnect(m_Controls.singleSlot, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector3.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener3(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector3->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());  
    connect(m_SelectionServiceConnector3.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector3->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector3.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider4(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector4->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_Controls.multiSlot, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector4.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
  else
  {
    m_SelectionServiceConnector4->RemoveAsSelectionProvider();
    disconnect(m_Controls.multiSlot, SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector4.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener4(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector4->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector4.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.multiSlot, &QmitkMultiNodeSelectionWidget::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector4->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector4.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged, m_Controls.multiSlot, &QmitkMultiNodeSelectionWidget::SetCurrentSelection);
  }
}

void QmitkDataStorageViewerTestView::OnOnlyImages(bool checked)
{
  if (checked)
  {
    m_Controls.singleSlot->SetNodePredicate(mitk::NodePredicateDataType::New("Image"));
  }
  else
  {
    m_Controls.singleSlot->SetNodePredicate(nullptr);
  }
};

void QmitkDataStorageViewerTestView::OnOnlyImages2(bool checked)
{
  if (checked)
  {
    m_Controls.multiSlot->SetNodePredicate(mitk::NodePredicateDataType::New("Image"));
    m_Controls.multiSlot->SetInvalidInfo(QString("InvalidInfo: is displayed for invalid states. Only images allowed!"));
  }
  else
  {
    m_Controls.multiSlot->SetNodePredicate(nullptr);
    m_Controls.multiSlot->SetInvalidInfo(QString("InvalidInfo: is displayed for invalid states"));
  }
};

void QmitkDataStorageViewerTestView::OnOnlyUneven(bool checked)
{
  if (checked)
  {
    auto checkFunction = [](const QmitkMultiNodeSelectionWidget::NodeList & nodes)
    {
      if (!(nodes.size() % 2))
      {
        std::stringstream ss;
        ss << "<font class=\"warning\"><p>Invalid selection.<p/><p>The number of selected nodes must be uneven! the current number is " << nodes.size() << ".</p>";
        return ss.str();
      }
      return std::string();
    };

    m_Controls.multiSlot->SetSelectionCheckFunction(checkFunction);
  }
  else
  {
    auto checkFunction = [](const QmitkMultiNodeSelectionWidget::NodeList & /*nodes*/)
    {
      return std::string();
    };

    m_Controls.multiSlot->SetSelectionCheckFunction(checkFunction);
  }
};
