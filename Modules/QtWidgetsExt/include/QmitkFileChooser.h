/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFileChooser_h
#define QmitkFileChooser_h

#include <MitkQtWidgetsExtExports.h>
#include <QWidget>

class QPushButton;
class QLineEdit;

/**
 * \brief Convenience widget showing a line edit with a file/directory path and a browse button.
 *
 * Provides a line edit displaying the current path and a "Select File" button
 * that opens a QFileDialog. Various options control whether files or directories
 * are selected, whether they must exist, and whether the line edit is editable.
 */
class MITKQTWIDGETSEXT_EXPORT QmitkFileChooser : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the file chooser with default settings.
   *
   * Defaults: vertical layout, file mode (not directory), file must exist, read-only line edit.
   *
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  explicit QmitkFileChooser(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /**
   * \brief Set horizontal or vertical layout for the button and line edit.
   * \param[in] horizontalLayout If true, button is placed to the right of the line edit.
   *                             If false (default), button is below.
   */
  void SetHorizotalLayout(bool horizontalLayout);

  /**
   * \brief Set whether the chooser selects directories instead of files.
   * \param[in] selectDir True for directory selection, false (default) for file selection.
   */
  void SetSelectDir(bool selectDir);

  /**
   * \brief Set whether the selected file or directory must already exist.
   * \param[in] fileMustExist True (default) to require existence, false to allow new paths.
   */
  void SetFileMustExist(bool fileMustExist);

  /**
   * \brief Programmatically set the file path.
   * \param[in] file The file or directory path to set.
   *
   * If fileMustExist is true and the path does not exist, the path is not set.
   */
  void SetFile(const std::string &file);

  /**
   * \brief Set the file filter pattern for the file dialog.
   * \param[in] filepattern A Qt file filter string (e.g. "Images (*.png *.jpg)").
   */
  void SetFilePattern(const std::string &filepattern);

  /**
   * \brief Set whether the line edit is read-only.
   * \param[in] ReadOnly True to make read-only (default), false to allow manual editing.
   */
  void SetReadOnly(bool ReadOnly);

  /**
   * \brief Check whether the currently entered file/directory exists.
   * \return True if the path exists on disk.
   */
  bool IsValidFile() const;

  /**
   * \brief Get the currently displayed file path.
   * \return The absolute file path as a string.
   */
  virtual std::string GetFile() const;

signals:
  /**
   * \brief Emitted when the file path changes, either programmatically or by the user.
   * \param[in] filePath The new file path.
   */
  void NewFileSelected(const std::string &filePath);

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
