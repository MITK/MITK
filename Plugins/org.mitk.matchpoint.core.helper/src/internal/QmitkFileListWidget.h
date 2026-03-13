/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFileListWidget_h
#define QmitkFileListWidget_h

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkPathListWidget;
}

/**
 * \class QmitkFileListWidget
 * \brief Widget to contain a ctkPathListWidget and a ctkPathListButtonsWidget
 * and provide simple file access for readable, executable files.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 */
class QmitkFileListWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkFileListWidget(QWidget* parent=nullptr);
  ~QmitkFileListWidget() override;

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

  void setText(const QString& text);

Q_SIGNALS:

  void pathsChanged(const QStringList&, const QStringList&);

private Q_SLOTS:

  void OnPathsChanged(const QStringList&, const QStringList&);

protected:
  std::unique_ptr<Ui::QmitkPathListWidget> m_Controls;
};

#endif
