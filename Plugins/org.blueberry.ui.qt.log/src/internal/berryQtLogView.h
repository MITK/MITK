/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTLOGVIEW_H
#define BERRYQTLOGVIEW_H

#include <QWidget>

#include <memory>

class QTimer;

namespace Ui
{
  class QtLogViewClass;
}

namespace berry {

class QtLogFilterProxyModel;
class QtPlatformLogModel;

class QtLogView : public QWidget
{
    Q_OBJECT

public:
    QtLogView(QWidget *parent = nullptr);
    ~QtLogView() override;

    /** Decides whether a message needs a tool tip, which depends on the column
     *  width and so cannot be answered by the model.
     */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void ApplyFilter();
    void OnMinimumLevelChanged(int index);
    void OnShowDetailsToggled(bool checked);
    void OnClearClicked();
    void OnCopyToClipboardClicked();
    void OnRowsAboutToBeInserted();
    void OnRowsInserted(const QModelIndex& parent, int first, int last);
    void OnSectionResized(int logicalIndex);

    void ApplyShowDetails(bool showDetails);
    void SizeColumnsToContents();
    void FillMessageColumn();
    bool IsElided(const QModelIndex& index) const;

    std::unique_ptr<Ui::QtLogViewClass> ui;

    QtPlatformLogModel* m_Model;
    QtLogFilterProxyModel* m_FilterModel;

    /** Coalesces the keystrokes in the filter into a single filter pass. */
    QTimer* m_FilterTimer;

    /** Whether the newest entry was in view when the last insertion started. */
    bool m_FollowNewEntries;

    /** Guards the setup that must not undo what the user has done since:
     *  the column widths and the scroll position are theirs from the first
     *  time the view is shown.
     */
    bool m_FirstShow;
};

}

#endif // BERRYQTLOGVIEW_H
