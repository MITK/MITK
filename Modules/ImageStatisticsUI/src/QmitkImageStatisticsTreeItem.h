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

#include <mitkImageStatisticsContainer.h>

/**
 * \brief An item that represents an entry (usually an ImageStatisticsObject) for the QmitkImageStatisticsTreeModel.
 *
 * Provides a hierarchical tree structure used internally by QmitkImageStatisticsTreeModel
 * to display image statistics in a QTreeView.
 */
class QmitkImageStatisticsTreeItem
{
public:
  using ImageStatisticsObject = mitk::ImageStatisticsContainer::ImageStatisticsObject;
  using StatisticNameVector = mitk::ImageStatisticsContainer::ImageStatisticsObject::StatisticNameVector;

  /** \brief Default constructor. Creates an empty item with no statistics data. */
  QmitkImageStatisticsTreeItem();

  /**
   * \brief Construct a tree item holding actual statistics data.
   *
   * \param statisticsData The image statistics object containing the computed values.
   * \param statisticNames Names of the statistics to display as columns.
   * \param itemText Display text for this tree item.
   * \param isWIP True if the statistics are still being computed (work in progress).
   * \param parentItem Optional parent item in the tree hierarchy.
   * \param imageNode Optional data node for the image.
   * \param maskNode Optional data node for the mask.
   * \param label Optional label associated with this item.
   */
  explicit QmitkImageStatisticsTreeItem(const ImageStatisticsObject& statisticsData,
    const StatisticNameVector& statisticNames, QVariant itemText, bool isWIP,
    QmitkImageStatisticsTreeItem* parentItem = nullptr, const mitk::DataNode* imageNode = nullptr,
    const mitk::DataNode* maskNode = nullptr, const mitk::Label* label = nullptr);

  /**
   * \brief Construct a tree item without statistics data (e.g. a group header).
   *
   * \param statisticNames Names of the statistics to display as columns.
   * \param itemText Display text for this tree item.
   * \param isWIP True if the statistics are still being computed.
   * \param isNA True if the statistics are not applicable for this item.
   * \param parentItem Optional parent item in the tree hierarchy.
   * \param imageNode Optional data node for the image.
   * \param maskNode Optional data node for the mask.
   * \param label Optional label associated with this item.
   */
  explicit QmitkImageStatisticsTreeItem(const StatisticNameVector& statisticNames,
    QVariant itemText, bool isWIP, bool isNA, QmitkImageStatisticsTreeItem *parentItem = nullptr, const mitk::DataNode* imageNode = nullptr,
    const mitk::DataNode* maskNode = nullptr, const mitk::Label* label = nullptr);

  ~QmitkImageStatisticsTreeItem();

  /**
   * \brief Append a child item to this item.
   * \param child The child item to add. This item takes ownership.
   */
  void appendChild(QmitkImageStatisticsTreeItem *child);

  /**
   * \brief Return the child item at the given row.
   * \param row The row index.
   * \return The child item, or nullptr if out of range.
   */
  QmitkImageStatisticsTreeItem *child(int row);

  /** \brief Return the number of child items. */
  int childCount() const;

  /** \brief Return the number of data columns. */
  int columnCount() const;

  /**
   * \brief Return the data for the given column.
   * \param column The column index.
   * \return The data value as QVariant.
   */
  QVariant data(int column) const;

  /** \brief Return this item's row index within its parent. */
  int row() const;

  /** \brief Return the parent item, or nullptr if this is the root. */
  QmitkImageStatisticsTreeItem *parentItem();

  /**
   * \brief Indicate whether the statistics are still being computed (work in progress).
   *
   * Returns true if the statistic container owned by this instance is only a dummy
   * WIP container and the calculation of the up-to-date statistic is not yet finished.
   */
  bool isWIP() const;

  /** \brief Return the label instance associated with this item, or nullptr. */
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
  bool m_NA;
};

#endif
