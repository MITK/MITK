/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkViewItem_h
#define QmitkViewItem_h

#include <QRegularExpression>
#include <QStandardItem>
#include <QStringList>

namespace berry
{
  struct IViewDescriptor;
}

class QmitkViewItem : public QStandardItem
{
public:
  static constexpr int KeywordsRole = Qt::UserRole + 2;

  explicit QmitkViewItem(const berry::IViewDescriptor* view);
  ~QmitkViewItem() override;

  /** \brief Enable or disable bold font for this item.
   */
  void SetBoldFont(bool enable);

  /** \brief Get view keywords as optionally specified in a corresponding plugin.xml file.
   */
  QStringList GetKeywords() const;

  /** \brief Match item against regular expression.
   *
   * View name (text), description (tool tip), and keywords are considered.
   */
  bool Match(const QRegularExpression& re) const;

private:
  void SetKeywords(const QStringList& keywords);
};

#endif
