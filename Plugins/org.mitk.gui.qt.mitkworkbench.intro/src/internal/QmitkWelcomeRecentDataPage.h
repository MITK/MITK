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
 * opened.
 */
class QmitkWelcomeRecentDataPage : public QWidget
{
  Q_OBJECT

public:
  QmitkWelcomeRecentDataPage(berry::IWorkbenchWindow::Pointer window, const QmitkWelcomePalette& palette, QWidget* parent = nullptr);

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

  void OnItemClicked(const QListWidgetItem* item);
  void OnContextMenuRequested(QListWidget* list, const QPoint& pos);

  berry::IWorkbenchWindow::WeakPtr m_Window;
  QmitkWelcomePalette m_Palette;
  RecentList m_Projects;
  RecentList m_Files;
};

#endif
