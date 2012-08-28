/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKDIRECTORYLISTWIDGET_H
#define QMITKDIRECTORYLISTWIDGET_H

#include "ui_QmitkDirectoryListWidget.h"
#include <QWidget>

/**
 * \class QmitkDirectoryListWidget
 * \brief Widget to contain a ctkPathListWidget and a ctkPathListButtonsWidget
 */
class QmitkDirectoryListWidget : public QWidget, public Ui::QmitkDirectoryListWidget
{
  Q_OBJECT

public:
  QmitkDirectoryListWidget(QWidget* parent=0);

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

#endif // QMITKDIRECTORYLISTWIDGET_H
