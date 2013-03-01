/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKFILELISTWIDGET_H
#define QMITKFILELISTWIDGET_H

#include "ui_QmitkPathListWidget.h"
#include <QWidget>

/**
 * \class QmitkFileListWidget
 * \brief Widget to contain a ctkPathListWidget and a ctkPathListButtonsWidget
 * and provide simple file access for readable, executable files.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 */
class QmitkFileListWidget : public QWidget, public Ui::QmitkPathListWidget
{
  Q_OBJECT

public:
  QmitkFileListWidget(QWidget* parent=0);

  /**
   * \brief Get all file entries.
   * \param absolutePath If <code>true</code>, resolve all entries to absolute paths.
   * \return A list of all file entries.
   */
  QStringList files(bool absolutePath = false) const;

  /**
   * \brief Sets the list of file entries.
   * \param paths The new path list.
   */
  void setFiles(const QStringList& paths);

Q_SIGNALS:

  void pathsChanged(const QStringList&, const QStringList&);

private Q_SLOTS:

  void OnPathsChanged(const QStringList&, const QStringList&);
};

#endif // QMITKDIRECTORYLISTWIDGET_H
