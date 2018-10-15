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

// semantic relations UI module
#include "QmitkPatientTableHeaderView.h"

// qt
#include <QPainter>
#include <QSize>

QmitkPatientTableHeaderView::QmitkPatientTableHeaderView(QWidget* parent/* = nullptr*/)
  : QHeaderView(Qt::Horizontal, parent)
  , m_HeaderModel(nullptr)
{
  // nothing here
}

QmitkPatientTableHeaderView::~QmitkPatientTableHeaderView()
{
  // nothing here
}

void QmitkPatientTableHeaderView::setModel(QAbstractItemModel* model)
{
  // retrieve the header model from the given table model
  QVariant variant = model->data(QModelIndex(), HorizontalHeaderDataRole);
  if (variant.isValid() && variant.canConvert<QStandardItemModel*>())
  {
    m_HeaderModel = variant.value<QStandardItemModel*>();
  }

  QHeaderView::setModel(model);
}

void QmitkPatientTableHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
  if (rect.isValid())
  {
    int top = rect.y();

    QModelIndex leafIndex = HeaderIndex(logicalIndex);
    QModelIndexList indexes = ParentIndexList(leafIndex);
    for (const auto& index : indexes)
    {
      top = PaintHeader(painter, index, logicalIndex, rect, top);
    }

    return;
  }

  QHeaderView::paintSection(painter, rect, logicalIndex);
}

QSize QmitkPatientTableHeaderView::sectionSizeFromContents(int logicalIndex) const
{
  if (nullptr != m_HeaderModel)
  {
    QModelIndex headerIndex = HeaderIndex(logicalIndex);
    if (headerIndex.isValid())
    {
      QSize headerSize = HeaderSize(headerIndex);
      headerIndex = headerIndex.parent();
      while (headerIndex.isValid())
      {
        QSize currentHeaderSize = HeaderSize(headerIndex);
        headerSize.rheight() += currentHeaderSize.height();
        if (currentHeaderSize.width() > headerSize.width())
        {
          headerSize.rwidth() = currentHeaderSize.width();
        }

        headerIndex = headerIndex.parent();
      }
      return headerSize;
    }
  }

  return QHeaderView::sectionSizeFromContents(logicalIndex);
}

int QmitkPatientTableHeaderView::PaintHeader(QPainter* painter, const QModelIndex& currentIndex, int logicalIndex, const QRect& sectionRect, int top) const
{
  QModelIndex headerIndex = HeaderIndex(logicalIndex);
  int height = HeaderSize(currentIndex).height();
  if (currentIndex == headerIndex)
  {
    height = sectionRect.height() - top;
  }
  int left = CurrentHeaderLeft(currentIndex, headerIndex, logicalIndex, sectionRect.left());
  int width = CurrentHeaderWidth(currentIndex, headerIndex, logicalIndex);

  QStyleOptionHeader headerStyleOptions;
  initStyleOption(&headerStyleOptions);
  headerStyleOptions.text = currentIndex.data(Qt::DisplayRole).toString();
  headerStyleOptions.textAlignment = Qt::AlignCenter;
  painter->save();

  QRect rect(left, top, width, height);
  headerStyleOptions.rect = rect;
  style()->drawControl(QStyle::CE_Header, &headerStyleOptions, painter, this);
  painter->restore();

  return top + height;
}

QSize QmitkPatientTableHeaderView::HeaderSize(const QModelIndex& index) const
{
  QFont font = this->font();
  font.setBold(true);
  QFontMetrics fontMetrics(font);
  QSize fontSize = fontMetrics.size(0, index.data(Qt::DisplayRole).toString());
  QSize emptyFontSize = fontMetrics.size(0, "");

  return fontSize + emptyFontSize;
}

int QmitkPatientTableHeaderView::CurrentHeaderLeft(const QModelIndex& currentIndex, const QModelIndex& headerIndex, int sectionIndex, int left) const
{
  QModelIndexList headerList = ListHeader(currentIndex);
  if (!headerList.empty())
  {
    int index = headerList.indexOf(headerIndex);
    int firstHeaderSectionIndex = sectionIndex - index;
    --index;
    for (; index >= 0; --index)
    {
      left -= sectionSize(firstHeaderSectionIndex + index);
    }
  }

  return left;
}

int QmitkPatientTableHeaderView::CurrentHeaderWidth(const QModelIndex& currentIndex, const QModelIndex& headerIndex, int sectionIndex) const
{
  QModelIndexList headerList = ListHeader(currentIndex);
  if (headerList.empty())
  {
    return sectionSize(sectionIndex);
  }

  int width = 0;
  int index = headerList.indexOf(headerIndex);
  int firstHeaderSectionIndex = sectionIndex - index;
  for (int i = 0; i < headerList.size(); ++i)
  {
    width += sectionSize(firstHeaderSectionIndex + i);
  }

  return width;
}

QModelIndexList QmitkPatientTableHeaderView::ParentIndexList(QModelIndex index) const
{
  QModelIndexList indexList;
  while (index.isValid())
  {
    indexList.push_front(index);
    index = index.parent();
  }

  return indexList;
}

QModelIndex QmitkPatientTableHeaderView::HeaderIndex(int sectionIndex) const
{
  if (nullptr != m_HeaderModel)
  {
    int currentHeaderIndex = -1;
    for (int i = 0; i < m_HeaderModel->columnCount(); ++i)
    {
      QModelIndex modelIndex = FindHeader(m_HeaderModel->index(0, i), sectionIndex, currentHeaderIndex);
      if (modelIndex.isValid())
      {
        return modelIndex;
      }
    }
  }

  return QModelIndex();
}

QModelIndex QmitkPatientTableHeaderView::FindHeader(const QModelIndex& currentIndex, int sectionIndex, int& currentHeaderIndex) const
{
  if (currentIndex.isValid())
  {
    int childCount = currentIndex.model()->columnCount(currentIndex);
    if (childCount > 0)
    {
      for (int i = 0; i < childCount; ++i)
      {
        QModelIndex modelIndex = FindHeader(currentIndex.child(0, i), sectionIndex, currentHeaderIndex);
        if (modelIndex.isValid())
        {
          return modelIndex;
        }
      }
    }
    else
    {
      ++currentHeaderIndex;
      if (currentHeaderIndex == sectionIndex)
      {
        return currentIndex;
      }
    }
  }

  return QModelIndex();
}

QModelIndexList QmitkPatientTableHeaderView::ListHeader(const QModelIndex& currentIndex) const
{
  QModelIndexList headerList;
  if (currentIndex.isValid())
  {
    int childCount = currentIndex.model()->columnCount(currentIndex);
    if (childCount > 0)
    {
      for (int i = 0; i < childCount; ++i)
      {
        headerList += ListHeader(currentIndex.child(0, i));
      }
    }
    else
    {
      headerList.push_back(currentIndex);
    }
  }
  return headerList;
}
