/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYHELPCONTENTVIEW_H_
#define BERRYHELPCONTENTVIEW_H_

#include <berryQtViewPart.h>

#include <QTreeView>

class QSortFilterProxyModel;
class QHelpContentModel;

namespace berry {

class HelpContentWidget : public QTreeView
{
    Q_OBJECT

public:
  HelpContentWidget();
  QModelIndex indexOf(const QUrl &link);

  void setModel(QAbstractItemModel *model) override;
  QUrl GetUrl(const QModelIndex &index);
Q_SIGNALS:

  /**
   * This signal is emitted when a content item is activated and
   * its associated \a link should be shown.
   */
  void linkActivated(const QUrl &link);

private Q_SLOTS:

  /**
   * Returns the index of the content item with the \a link.
   * An invalid index is returned if no such an item exists.
   */
  void showLink(const QModelIndex &index);

private:
  bool searchContentItem(QHelpContentModel* model,
                         const QModelIndex &parent, const QString &path);
  QModelIndex m_syncIndex;
  QSortFilterProxyModel* m_SortModel;
  QAbstractItemModel* m_SourceModel;
};

class HelpContentView : public QtViewPart
{
  Q_OBJECT

public:

  HelpContentView();
  ~HelpContentView();

  void SetFocus() override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

protected Q_SLOTS:

  void linkActivated(const QUrl& link);

private Q_SLOTS:

  void showContextMenu(const QPoint &pos);

  void setContentsWidgetBusy();
  void unsetContentsWidgetBusy();

private:

  Q_DISABLE_COPY(HelpContentView)

  HelpContentWidget* m_ContentWidget;

};

} // namespace berry

#endif /*BERRYHELPCONTENTVIEW_H_*/
