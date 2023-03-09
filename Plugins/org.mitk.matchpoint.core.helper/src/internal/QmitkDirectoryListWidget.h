/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDirectoryListWidget_h
#define QmitkDirectoryListWidget_h

#include "ui_QmitkPathListWidget.h"
#include <QWidget>

/**
 * \class QmitkDirectoryListWidget
 * \brief Widget to contain a ctkPathListWidget and a ctkPathListButtonsWidget
 * and provide simple directory access for readable, executable directories.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 */
class QmitkDirectoryListWidget : public QWidget, public Ui::QmitkPathListWidget
{
  Q_OBJECT

public:
  QmitkDirectoryListWidget(QWidget* parent=nullptr);

  /**
   * \brief Get all directory entries.
   * \param absolutePath If <code>true</code>, resolve all entries to absolute paths.
   * \return A list of all directory entries.
   */
  QStringList directories(bool absolutePath = false) const;

  /**
   * \brief Sets the list of directory entries.
   * \param paths The new path list.
   */
  void setDirectories(const QStringList& paths);

Q_SIGNALS:

  void pathsChanged(const QStringList&, const QStringList&);

private Q_SLOTS:

  void OnPathsChanged(const QStringList&, const QStringList&);
};

#endif
