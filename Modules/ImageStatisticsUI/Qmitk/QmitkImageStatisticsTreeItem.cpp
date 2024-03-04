/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsTreeItem.h"

QmitkImageStatisticsTreeItem::QmitkImageStatisticsTreeItem(
  const ImageStatisticsObject& statisticsData,
  const StatisticNameVector& statisticNames,
  QVariant itemText, bool isWIP,
  QmitkImageStatisticsTreeItem *parent, const mitk::DataNode* imageNode,
  const mitk::DataNode* maskNode, const mitk::Label* label)
  : m_statistics(statisticsData) , m_statisticNames(statisticNames), m_ItemText(itemText), m_parentItem(parent),
    m_ImageNode(imageNode), m_MaskNode(maskNode), m_Label(label), m_IsWIP(isWIP)
{
}

 QmitkImageStatisticsTreeItem::QmitkImageStatisticsTreeItem(const StatisticNameVector& statisticNames,
                                                           QVariant itemText,
                                                           bool isWIP,
                                                           QmitkImageStatisticsTreeItem *parentItem, const mitk::DataNode* imageNode,
   const mitk::DataNode* maskNode, const mitk::Label* label)
  : QmitkImageStatisticsTreeItem(ImageStatisticsObject(), statisticNames, itemText, isWIP, parentItem, imageNode, maskNode, label )
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

struct StatValueVisitor : boost::static_visitor<QVariant>
{
  QVariant operator()(const mitk::ImageStatisticsContainer::RealType& val) const
  {
    return QVariant(val);
  }

  QVariant operator()(const mitk::ImageStatisticsContainer::VoxelCountType& val) const
  {
    return QVariant::fromValue<mitk::ImageStatisticsContainer::VoxelCountType>(val);
  }

  QVariant operator()(const mitk::ImageStatisticsContainer::IndexType& val) const
  {
    std::stringstream ss;
    ss << val;
    return QVariant(QString::fromStdString(ss.str()));
  }

};

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
        if (m_statistics.HasStatistic(statisticKey))
        {
          return boost::apply_visitor(StatValueVisitor(), m_statistics.GetValueNonConverted(statisticKey));
        }
        else
        {
          return QVariant();
        }
      }
    }
    else
    {
      return QVariant();
    }
  }
  else if (column == 0)
  {
    result = m_ItemText;
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
}

mitk::Label::ConstPointer QmitkImageStatisticsTreeItem::GetLabelInstance() const
{
  return m_Label.Lock();
}
