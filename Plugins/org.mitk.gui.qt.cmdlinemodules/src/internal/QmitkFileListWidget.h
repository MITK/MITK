/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFileListWidget_h
#define QmitkFileListWidget_h

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
  QmitkFileListWidget(QWidget* parent=nullptr);

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

#endif
