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

#include "QmitkImageStatisticsWidget.h"

#include "QmitkStatisticsModelToStringConverter.h"
#include "QmitkImageStatisticsTreeModel.h"

#include <QSortFilterProxyModel>
#include <QClipboard>

QmitkImageStatisticsWidget::QmitkImageStatisticsWidget(QWidget* parent) : QWidget(parent)
{
  m_Controls.setupUi(this);
  m_imageStatisticsModel = new QmitkImageStatisticsTreeModel(parent);
  CreateConnections();
  m_ProxyModel = new QSortFilterProxyModel(this);
  m_Controls.treeViewStatistics->setEnabled(false);
  m_Controls.treeViewStatistics->setModel(m_ProxyModel);
  m_ProxyModel->setSourceModel(m_imageStatisticsModel);
  connect(m_imageStatisticsModel, &QmitkImageStatisticsTreeModel::dataAvailable, this, &QmitkImageStatisticsWidget::OnDataAvailable);
  connect(m_imageStatisticsModel,
          &QmitkImageStatisticsTreeModel::modelChanged,
          m_Controls.treeViewStatistics,
          &QTreeView::expandAll);
}

void QmitkImageStatisticsWidget::SetDataStorage(mitk::DataStorage* newDataStorage)
{
  m_imageStatisticsModel->SetDataStorage(newDataStorage);
}

void QmitkImageStatisticsWidget::SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes)
{
  m_imageStatisticsModel->SetImageNodes(nodes);
}

void QmitkImageStatisticsWidget::SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes)
{
  m_imageStatisticsModel->SetMaskNodes(nodes);
}

void QmitkImageStatisticsWidget::Reset()
{
  m_imageStatisticsModel->Clear();
  m_Controls.treeViewStatistics->setEnabled(false);
  m_Controls.buttonCopyImageStatisticsToClipboard->setEnabled(false);
}

void QmitkImageStatisticsWidget::CreateConnections()
{
	connect(m_Controls.buttonCopyImageStatisticsToClipboard, &QPushButton::clicked, this, &QmitkImageStatisticsWidget::OnClipboardButtonClicked);
}

void QmitkImageStatisticsWidget::OnDataAvailable()
{
  m_Controls.buttonCopyImageStatisticsToClipboard->setEnabled(true);
  m_Controls.treeViewStatistics->setEnabled(true);
}

void QmitkImageStatisticsWidget::OnClipboardButtonClicked()
{
  QmitkStatisticsModelToStringConverter converter;
  converter.SetTableModel(m_imageStatisticsModel);
  converter.SetRootIndex(m_Controls.treeViewStatistics->rootIndex());
  converter.SetIncludeHeaderData(true);

  QString clipboardAsString = converter.GetString();
  QApplication::clipboard()->setText(clipboardAsString, QClipboard::Clipboard);
}
