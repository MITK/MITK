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
    StatisticNameVector statisticNames, QVariant label, QmitkImageStatisticsTreeItem *parentItem = nullptr);
  explicit QmitkImageStatisticsTreeItem(StatisticNameVector statisticNames,
    QVariant label, QmitkImageStatisticsTreeItem *parentItem = nullptr);
  ~QmitkImageStatisticsTreeItem();

  void appendChild(QmitkImageStatisticsTreeItem *child);

  QmitkImageStatisticsTreeItem *child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  QmitkImageStatisticsTreeItem *parentItem();

private:
  ImageStatisticsObject m_statistics;
  StatisticNameVector m_statisticNames;
  QVariant m_label;
  QmitkImageStatisticsTreeItem *m_parentItem = nullptr;
  QList<QmitkImageStatisticsTreeItem *> m_childItems;
};

#endif // QmitkImageStatisticsTreeItem_h
