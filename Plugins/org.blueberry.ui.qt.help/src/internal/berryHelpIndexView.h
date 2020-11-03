/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHELPINDEXVIEW_H_
#define BERRYHELPINDEXVIEW_H_

#include <berryQtViewPart.h>

#include <QModelIndex>
#include <QListView>

class ctkSearchBox;
class QHelpIndexWidget;

namespace berry {

class HelpIndexWidget : public QListView
{
  Q_OBJECT

Q_SIGNALS:

  /**
   * This signal is emitted when an item is activated and its
   * associated \a link should be shown. To know where the link
   * belongs to, the \a keyword is given as a second paremeter.
   */
  void linkActivated(const QUrl &link, const QString &keyword);

  /**
   * This signal is emitted when the item representing the \a keyword
   * is activated and the item has more than one link associated.
   * The \a links consist of the document title and their URL.
   */
  void linksActivated(const QMap<QString, QUrl> &links,
                      const QString &keyword);

public:

  HelpIndexWidget();

public Q_SLOTS:

  /**
   * Filters the indices according to \a filter or \a wildcard.
   * The item with the best match is set as current item.
   */
  void filterIndices(const QString &filter,
                     const QString &wildcard = QString());

  /**
   * Activates the current item which will result eventually in
   * the emitting of a linkActivated() or linksActivated()
   * signal.
   */
  void activateCurrentItem();

private Q_SLOTS:
  void showLink(const QModelIndex &index);
};

class HelpIndexView : public QtViewPart
{
  Q_OBJECT

public:

  HelpIndexView();
  ~HelpIndexView() override;

  void SetFocus() override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

  void setSearchLineEditText(const QString &text);
  QString searchLineEditText() const;

protected Q_SLOTS:

  void linkActivated(const QUrl& link);
  void linksActivated(const QMap<QString, QUrl> &links, const QString &keyword);

private Q_SLOTS:

  void filterIndices(const QString &filter);
  void enableSearchLineEdit();
  void disableSearchLineEdit();

  void setIndexWidgetBusy();
  void unsetIndexWidgetBusy();

private:

  bool eventFilter(QObject *obj, QEvent *e) override;
  void focusInEvent(QFocusEvent *e);
  void open(HelpIndexWidget *indexWidget, const QModelIndex &index);

  Q_DISABLE_COPY(HelpIndexView)

  ctkSearchBox* m_SearchLineEdit;
  HelpIndexWidget* m_IndexWidget;

};

} // namespace berry

#endif /*BERRYHELPINDEXVIEW_H_*/
