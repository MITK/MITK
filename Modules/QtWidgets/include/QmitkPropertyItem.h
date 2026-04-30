/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyItem_h
#define QmitkPropertyItem_h

#include <QList>
#include <QVariant>

/**
 * \brief A tree item used by QmitkPropertyItemModel to represent a property row.
 *
 * Each item stores a list of QVariant column data (typically property name and value)
 * and maintains parent-child relationships to form a tree structure. This is a
 * non-Qt helper class used internally by QmitkPropertyItemModel.
 *
 * \sa QmitkPropertyItemModel
 * \sa QmitkPropertyItemDelegate
 */
class QmitkPropertyItem
{
public:
  /**
   * \brief Constructs a property item with the given column data.
   * \param[in] data A list of QVariant values, one per column.
   */
  explicit QmitkPropertyItem(const QList<QVariant> &data);
  ~QmitkPropertyItem();

  /**
   * \brief Appends a child item to this item.
   * \param[in] child The child item to append. Takes ownership.
   */
  void AppendChild(QmitkPropertyItem *child);

  /**
   * \brief Returns the child item at the given row.
   * \param[in] row The zero-based row index.
   * \return Pointer to the child item, or nullptr if out of range.
   */
  QmitkPropertyItem *GetChild(int row) const;

  /**
   * \brief Returns the number of child items.
   * \return The child count.
   */
  int GetChildCount() const;

  /**
   * \brief Returns the number of data columns.
   * \return The column count.
   */
  int GetColumnCount() const;

  /**
   * \brief Returns the data for the given column.
   * \param[in] column The zero-based column index.
   * \return The column data as QVariant.
   */
  QVariant GetData(int column) const;

  /**
   * \brief Returns the parent item.
   * \return Pointer to the parent item, or nullptr for the root item.
   */
  QmitkPropertyItem *GetParent() const;

  /**
   * \brief Returns the row index of this item within its parent.
   * \return The zero-based row index.
   */
  int GetRow();

private:
  QmitkPropertyItem(const QmitkPropertyItem &);
  QmitkPropertyItem &operator=(const QmitkPropertyItem &);

  QList<QVariant> m_Data;
  QList<QmitkPropertyItem *> m_Children;
  QmitkPropertyItem *m_Parent;
};

#endif
