/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsTreeItem.h"

QmitkImageStatisticsTreeItem::QmitkImageStatisticsTreeItem(
  ImageStatisticsObject statisticsData,
  StatisticNameVector statisticNames,
  QVariant label, bool isWIP,
  QmitkImageStatisticsTreeItem *parent)
  : m_statistics(statisticsData) , m_statisticNames(statisticNames), m_label(label), m_parentItem(parent), m_IsWIP(isWIP)
{
}

 QmitkImageStatisticsTreeItem::QmitkImageStatisticsTreeItem(StatisticNameVector statisticNames,
                                                           QVariant label,
                                                           bool isWIP,
                                                           QmitkImageStatisticsTreeItem *parentItem)
  : QmitkImageStatisticsTreeItem(ImageStatisticsObject(), statisticNames, label, isWIP, parentItem )
{
}

 QmitkImageStatisticsTreeItem::QmitkImageStatisticsTreeItem() : QmitkImageStatisticsTreeItem(StatisticNameVector(), QVariant(), false, nullptr ) {}

QmitkImageStatisticsTreeItem::~QmitkImageStatisticsTreeItem()
{
  qDeleteAll(m_childItems);
}

void QmitkImageStatisticsTreeItem::appendChild(QmitkImageStatisticsTreeItem *item)
{
  m_childItems.append(item);
}

QmitkImageStatisticsTreeItem *QmitkImageStatisticsTreeItem::child(int row)
{
  return m_childItems.value(row);
}

int QmitkImageStatisticsTreeItem::childCount() const
{
  return m_childItems.count();
}

int QmitkImageStatisticsTreeItem::columnCount() const
{
  return m_statisticNames.size() + 1;
}

QVariant QmitkImageStatisticsTreeItem::data(int column) const
{
  QVariant result;
  if (column > 0 && !m_statisticNames.empty())
  {
    if (column - 1 < static_cast<int>(m_statisticNames.size()))
    {
      if (m_IsWIP)
      {
        result = QVariant(QString("..."));
      }
      else
      {
        auto statisticKey = m_statisticNames.at(column - 1);
        std::stringstream ss;
        if (m_statistics.HasStatistic(statisticKey))
        {
          ss << m_statistics.GetValueNonConverted(statisticKey);
        }
        else
        {
          return QVariant();
        }
        result = QVariant(QString::fromStdString(ss.str()));
      }
    }
    else
    {
      return QVariant();
    }
  }
  else if (column == 0)
  {
    result = m_label;
  }
  return result;
}

QmitkImageStatisticsTreeItem *QmitkImageStatisticsTreeItem::parentItem()
{
  return m_parentItem;
}

int QmitkImageStatisticsTreeItem::row() const
{
  if (m_parentItem)
    return m_parentItem->m_childItems.indexOf(const_cast<QmitkImageStatisticsTreeItem *>(this));

  return 0;
}

bool QmitkImageStatisticsTreeItem::isWIP() const
{
  return m_IsWIP;
};
