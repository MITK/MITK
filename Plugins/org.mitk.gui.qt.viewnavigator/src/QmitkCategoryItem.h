/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCategoryItem_h
#define QmitkCategoryItem_h

#include <QRegularExpression>
#include <QStandardItem>

class QmitkCategoryItem : public QStandardItem
{
public:
  explicit QmitkCategoryItem(const QString& category);
  ~QmitkCategoryItem() override;

  /** \brief Check if any view item child of this category item matches the given regular expression.
   *
   * \sa QmitkViewItem::Match()
   */
  bool HasMatch(const QRegularExpression& re) const;

  /** \brief Get visibility of this item including its children.
   */
  bool GetVisible() const;

  /** \brief Enable or disable visibility of this item including its children.
   */
  void SetVisible(bool visible);
};

#endif
