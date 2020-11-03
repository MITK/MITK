/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// data storage viewer test plugin
#include "QmitkDataStorageViewerTestView.h"

#include "mitkNodePredicateDataType.h"

// berry
#include <berryIWorkbenchWindow.h>

const std::string QmitkDataStorageViewerTestView::VIEW_ID = "org.mitk.views.datastorageviewertest";

void QmitkDataStorageViewerTestView::SetFocus()
{
  // nothing here
}

void QmitkDataStorageViewerTestView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_Controls.listInspector->SetDataStorage(GetDataStorage());
  m_Controls.treeInspector->SetDataStorage(GetDataStorage());

  m_Controls.singleSlot->SetDataStorage(GetDataStorage());
  m_Controls.singleSlot->SetEmptyInfo(QStringLiteral("EmptyInfo: Set this to display info in empty state"));
  m_Controls.singleSlot->SetInvalidInfo(QStringLiteral("InvalidInfo: is displayed for invalid states"));
  m_Controls.singleSlot->SetPopUpTitel(QStringLiteral("This is the definable caption. Choose your data now!"));
  m_Controls.singleSlot->SetPopUpHint(QStringLiteral("I am an optional hint, that can be set by the developer<p/>If not set the widget is invisible."));

  m_Controls.multiSlot->SetDataStorage(GetDataStorage());
  m_Controls.multiSlot->SetEmptyInfo(QStringLiteral("EmptyInfo: Set this to display info in empty state"));
  m_Controls.multiSlot->SetInvalidInfo(QStringLiteral("InvalidInfo: is displayed for invalid states"));
  m_Controls.multiSlot->SetPopUpTitel(QStringLiteral("This is the definable caption. Choose your data now!"));
  m_Controls.multiSlot->SetPopUpHint(QStringLiteral("I am an optional hint, that can be set by the developer<p/>If not set the widget is invisible."));

  m_ModelViewSelectionConnector1 = std::make_unique<QmitkModelViewSelectionConnector>();
  try
  {
    m_ModelViewSelectionConnector1->SetView(m_Controls.listInspector->GetView());
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Cannot connect the model-view pair signals and slots.";
  }
  m_SelectionServiceConnector1 = std::make_unique<QmitkSelectionServiceConnector>();

  m_ModelViewSelectionConnector2 = std::make_unique<QmitkModelViewSelectionConnector>();
  try
  {
    m_ModelViewSelectionConnector2->SetView(m_Controls.treeInspector->GetView());
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Cannot connect the model-view pair signals and slots.";
  }
  m_SelectionServiceConnector2 = std::make_unique<QmitkSelectionServiceConnector>();

  m_SelectionServiceConnector3 = std::make_unique<QmitkSelectionServiceConnector>();
  m_SelectionServiceConnector4 = std::make_unique<QmitkSelectionServiceConnector>();

  connect(m_Controls.selectionProviderCheckBox1, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionProvider1);
  connect(m_Controls.selectionProviderCheckBox2, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionProvider2);

  connect(m_Controls.selectionListenerCheckBox1, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionListener1);
  connect(m_Controls.selectionListenerCheckBox2, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionListener2);

  connect(m_Controls.selectionProviderCheckBox3, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionProvider3);
  connect(m_Controls.selectionListenerCheckBox3, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionListener3);

  connect(m_Controls.checkOnlyVisible1, &QCheckBox::toggled,
    m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetSelectOnlyVisibleNodes);
  connect(m_Controls.checkOnlyImages1, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::OnOnlyImages1);
  connect(m_Controls.checkOptional1, &QCheckBox::toggled,
    m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetSelectionIsOptional);
  connect(m_Controls.checkEnabled1, &QCheckBox::toggled,
    m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::setEnabled);
  connect(m_Controls.checkAuto, &QCheckBox::toggled,
    m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetAutoSelectNewNodes);

  connect(m_Controls.selectionProviderCheckBox4, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionProvider4);
  connect(m_Controls.selectionListenerCheckBox4, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::SetAsSelectionListener4);

  connect(m_Controls.checkOnlyVisible2, &QCheckBox::toggled,
    m_Controls.multiSlot, &QmitkMultiNodeSelectionWidget::SetSelectOnlyVisibleNodes);
  connect(m_Controls.checkOnlyImages2, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::OnOnlyImages2);
  connect(m_Controls.checkOnlyUneven, &QCheckBox::toggled,
    this, &QmitkDataStorageViewerTestView::OnOnlyUneven);
  connect(m_Controls.checkOptional2, &QCheckBox::toggled,
    m_Controls.multiSlot, &QmitkMultiNodeSelectionWidget::SetSelectionIsOptional);
  connect(m_Controls.checkEnabled2, &QCheckBox::toggled,
    m_Controls.multiSlot, &QmitkSingleNodeSelectionWidget::setEnabled);
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider1(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector1->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_ModelViewSelectionConnector1.get(), &QmitkModelViewSelectionConnector::CurrentSelectionChanged,
      m_SelectionServiceConnector1.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
  else
  {
    m_SelectionServiceConnector1->RemoveAsSelectionProvider();
    disconnect(m_ModelViewSelectionConnector1.get(), &QmitkModelViewSelectionConnector::CurrentSelectionChanged,
      m_SelectionServiceConnector1.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener1(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector1->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector1.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_ModelViewSelectionConnector1.get(), &QmitkModelViewSelectionConnector::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector1->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector1.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_ModelViewSelectionConnector1.get(), &QmitkModelViewSelectionConnector::SetCurrentSelection);

  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider2(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector2->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_ModelViewSelectionConnector2.get(), &QmitkModelViewSelectionConnector::CurrentSelectionChanged,
      m_SelectionServiceConnector2.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
  else
  {
    m_SelectionServiceConnector2->RemoveAsSelectionProvider();
    disconnect(m_ModelViewSelectionConnector2.get(), &QmitkModelViewSelectionConnector::CurrentSelectionChanged,
      m_SelectionServiceConnector2.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener2(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector2->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector2.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_ModelViewSelectionConnector2.get(), &QmitkModelViewSelectionConnector::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector2->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector2.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_ModelViewSelectionConnector2.get(), &QmitkModelViewSelectionConnector::SetCurrentSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider3(bool checked)
{
  if (checked)
  {
    connect(m_Controls.singleSlot, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
      m_SelectionServiceConnector3.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
  else
  {
    m_SelectionServiceConnector3->RemoveAsSelectionProvider();
    disconnect(m_Controls.singleSlot, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
      m_SelectionServiceConnector3.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener3(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector3->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector3.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector3->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector3.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionProvider4(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector4->SetAsSelectionProvider(GetSite()->GetSelectionProvider().Cast<QmitkDataNodeSelectionProvider>().GetPointer());
    connect(m_Controls.multiSlot, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
      m_SelectionServiceConnector4.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
  else
  {
    m_SelectionServiceConnector4->RemoveAsSelectionProvider();
    disconnect(m_Controls.multiSlot, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
      m_SelectionServiceConnector4.get(), &QmitkSelectionServiceConnector::ChangeServiceSelection);
  }
}

void QmitkDataStorageViewerTestView::SetAsSelectionListener4(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector4->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector4.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_Controls.multiSlot, &QmitkMultiNodeSelectionWidget::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector4->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector4.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_Controls.multiSlot, &QmitkMultiNodeSelectionWidget::SetCurrentSelection);
  }
}

void QmitkDataStorageViewerTestView::OnOnlyImages1(bool checked)
{
  if (checked)
  {
    m_Controls.singleSlot->SetNodePredicate(mitk::NodePredicateDataType::New("Image"));
  }
  else
  {
    m_Controls.singleSlot->SetNodePredicate(nullptr);
  }
}

void QmitkDataStorageViewerTestView::OnOnlyImages2(bool checked)
{
  if (checked)
  {
    m_Controls.multiSlot->SetNodePredicate(mitk::NodePredicateDataType::New("Image"));
    m_Controls.multiSlot->SetInvalidInfo(QStringLiteral("InvalidInfo: is displayed for invalid states. Only images allowed!"));
  }
  else
  {
    m_Controls.multiSlot->SetNodePredicate(nullptr);
    m_Controls.multiSlot->SetInvalidInfo(QStringLiteral("InvalidInfo: is displayed for invalid states"));
  }
}

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
}
