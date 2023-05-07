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
  explicit QmitkImageStatisticsTreeItem(ImageStatisticsObject statisticsData,
    StatisticNameVector statisticNames, QVariant label, bool isWIP, QmitkImageStatisticsTreeItem *parentItem = nullptr);
  explicit QmitkImageStatisticsTreeItem(StatisticNameVector statisticNames,
    QVariant label, bool isWIP, QmitkImageStatisticsTreeItem *parentItem = nullptr);
  ~QmitkImageStatisticsTreeItem();

  void appendChild(QmitkImageStatisticsTreeItem *child);

  QmitkImageStatisticsTreeItem *child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  QmitkImageStatisticsTreeItem *parentItem();

  /**indicates that the statistic container owned by this instance is only a dummy
  WIP containter and the calculation of the up-to-date statistic is not yet finished.**/
  bool isWIP() const;

private:
  ImageStatisticsObject m_statistics;
  StatisticNameVector m_statisticNames;
  QVariant m_label;
  QmitkImageStatisticsTreeItem *m_parentItem = nullptr;
  QList<QmitkImageStatisticsTreeItem *> m_childItems;
  bool m_IsWIP;
};

#endif
