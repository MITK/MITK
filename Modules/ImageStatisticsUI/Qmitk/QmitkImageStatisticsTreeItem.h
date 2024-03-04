/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageStatisticsTreeItem_h
#define QmitkImageStatisticsTreeItem_h

#include <QList>
#include <QVariant>

#include <mitkWeakPointer.h>
#include <mitkDataNode.h>

#include "mitkImageStatisticsContainer.h"

/*!
\class QmitkImageStatisticsTreeItem
An item that represents an entry (usually ImageStatisticsObject) for the QmitkImageStatisticsTreeModel
*/
class QmitkImageStatisticsTreeItem
{
public:
  using ImageStatisticsObject = mitk::ImageStatisticsContainer::ImageStatisticsObject;
  using StatisticNameVector = mitk::ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector;
  QmitkImageStatisticsTreeItem();
  explicit QmitkImageStatisticsTreeItem(const ImageStatisticsObject& statisticsData,
    const StatisticNameVector& statisticNames, QVariant itemText, bool isWIP,
    QmitkImageStatisticsTreeItem* parentItem = nullptr, const mitk::DataNode* imageNode = nullptr,
    const mitk::DataNode* maskNode = nullptr, const mitk::Label* label = nullptr);
  explicit QmitkImageStatisticsTreeItem(const StatisticNameVector& statisticNames,
    QVariant itemText, bool isWIP, QmitkImageStatisticsTreeItem *parentItem = nullptr, const mitk::DataNode* imageNode = nullptr,
    const mitk::DataNode* maskNode = nullptr, const mitk::Label* label = nullptr);
  ~QmitkImageStatisticsTreeItem();

  void appendChild(QmitkImageStatisticsTreeItem *child);

  QmitkImageStatisticsTreeItem *child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  QmitkImageStatisticsTreeItem *parentItem();

  /**indicates that the statistic container owned by this instance is only a dummy
  WIP container and the calculation of the up-to-date statistic is not yet finished.**/
  bool isWIP() const;

  mitk::Label::ConstPointer GetLabelInstance() const;

private:
  ImageStatisticsObject m_statistics;
  StatisticNameVector m_statisticNames;
  QVariant m_ItemText;
  QmitkImageStatisticsTreeItem *m_parentItem = nullptr;
  QList<QmitkImageStatisticsTreeItem *> m_childItems;
  mitk::WeakPointer<const mitk::DataNode> m_ImageNode;
  mitk::WeakPointer<const mitk::DataNode> m_MaskNode;
  mitk::WeakPointer<const mitk::Label> m_Label;

  bool m_IsWIP;
};

#endif
