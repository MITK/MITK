/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomeRecentDataPage_h
#define QmitkWelcomeRecentDataPage_h

#include "QmitkWelcomePalette.h"

#include <mitkRecentData.h>

#include <berryIWorkbenchWindow.h>

#include <QFutureWatcher>
#include <QSet>
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

/**
 * \brief Welcome screen page that lists recently opened projects and files.
 *
 * Clicking an entry loads it like File > Open, i.e., into the data that is
 * already loaded. Entries whose files no longer exist are shown but cannot be
 * opened. Whether they exist is checked in the background, since unreachable
 * network paths can take long to answer.
 */
class QmitkWelcomeRecentDataPage : public QWidget
{
  Q_OBJECT

public:
  QmitkWelcomeRecentDataPage(berry::IWorkbenchWindow::Pointer window, const QmitkWelcomePalette& palette, QWidget* parent = nullptr);
  ~QmitkWelcomeRecentDataPage() override;

  void SetPalette(const QmitkWelcomePalette& palette);

protected:
  void showEvent(QShowEvent* event) override;

private:
  struct RecentList
  {
    mitk::RecentData::Kind Kind;
    QListWidget* List;
    QLabel* Hint;
    QPushButton* ClearButton;
  };

  QWidget* CreateCard(const QString& title, const QString& hint, RecentList& recentList);
  void UpdateList(const RecentList& recentList);
  void UpdateLists();
  void UpdateAvailability(const RecentList& recentList);
  void CheckAvailability();

  void OnItemClicked(const QListWidgetItem* item);
  void OnContextMenuRequested(QListWidget* list, const QPoint& pos);
  void OnRecentDataChanged();
  void OnAvailabilityChecked();

  berry::IWorkbenchWindow::WeakPtr m_Window;
  QmitkWelcomePalette m_Palette;
  RecentList m_Projects;
  RecentList m_Files;
  QSet<QString> m_MissingPaths;
  QFutureWatcher<QSet<QString>> m_AvailabilityCheck;
  bool m_IsAvailabilityCheckPending;
};

#endif
