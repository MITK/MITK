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

  void SetBoldFont(bool enable);

  QStringList GetKeywords() const;
  void SetKeywords(const QStringList& keywords);

  bool Match(const QRegularExpression& re) const;
};

#endif
