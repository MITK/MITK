/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFileChooser_h_Included
#define QmitkFileChooser_h_Included

#include "MitkQtWidgetsExtExports.h"
#include <QWidget>

class QPushButton;
class QLineEdit;

///
/// \brief Convenience Widget showing a line edit with the path of
/// a file or directory and a button which invokes a file choose dialog
///
/// Various methods are given to influence the behaviour or presentation
///
class MITKQTWIDGETSEXT_EXPORT QmitkFileChooser : public QWidget
{
  Q_OBJECT

public:
  ///
  /// standard ctor, init values to defaults (see methods for values)
  ///
  explicit QmitkFileChooser(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  ///
  /// determines whether the button "Select file" is shown on the left
  /// underneath the line edit, default=false (=is shown in a vertical layout)
  ///
  void SetHorizotalLayout(bool horizontalLayout);
  ///
  /// determines whether the selection must be a directory
  /// default=false
  ///
  void SetSelectDir(bool selectDir);
  ///
  /// determines whether the file/directory mustexist
  /// default=true
  ///
  void SetFileMustExist(bool fileMustExist);
  ///
  /// sets the file input, default=""
  ///
  void SetFile(const std::string &file);
  ///
  /// sets a file pattern to be selected, default=""
  ///
  void SetFilePattern(const std::string &filepattern);
  ///
  /// sets whether the user can edit the input, default=false
  ///
  void SetReadOnly(bool ReadOnly);
  ///
  /// returns whether the selected file/directory exists
  ///
  bool IsValidFile() const;
  ///
  /// returns the currently set file (an absolute path)
  ///
  virtual std::string GetFile() const;

signals:
  ///
  /// emitted when the input changed programatically or by the user
  ///
  void NewFileSelected(const std::string &);

protected slots:
  ///
  /// show dialog here
  ///
  virtual void OnSelectFileClicked(bool /*checked=false*/);
  ///
  /// check for valid here
  ///
  virtual void OnFileEditingFinished();

protected:
  ///
  /// \see SetSelectDir()
  ///
  bool m_SelectDir;
  ///
  /// \see SetFileMustExist()
  ///
  bool m_FileMustExist;

  ///
  /// \see SetFilePattern()
  ///
  QString m_FilePattern;
  ///
  /// the select file button
  ///
  QPushButton *m_SelectFile;
  ///
  /// the line edit to show the current file
  ///
  QLineEdit *m_File;
};

#endif
