/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDirectoryListWidget_h
#define QmitkDirectoryListWidget_h

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkPathListWidget;
}

/**
 * \class QmitkDirectoryListWidget
 * \brief Widget to contain a ctkPathListWidget and a ctkPathListButtonsWidget
 * and provide simple directory access for readable, executable directories.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 */
class QmitkDirectoryListWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkDirectoryListWidget(QWidget* parent=nullptr);
  ~QmitkDirectoryListWidget() override;

  void setText(const QString& text);

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

protected:
  std::unique_ptr<Ui::QmitkPathListWidget> m_Controls;
};

#endif
