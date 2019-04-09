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
\class QmitkImageStatisticsTableModel
Model that takes a mitk::ImageStatisticsContainer and represents it as model in context of the QT view-model-concept.
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
  QVariant m_label;
  QList<QmitkImageStatisticsTreeItem *> m_childItems;
  mitk::ImageStatisticsContainer::ImageStatisticsObject m_statistics;
  mitk::ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector m_statisticNames;
  QmitkImageStatisticsTreeItem *m_parentItem = nullptr;
};

#endif // QmitkImageStatisticsTreeItem_h
