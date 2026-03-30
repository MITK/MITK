/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file QmitkFileReaderOptionsDialog.h
 * \brief Declares QmitkFileReaderOptionsDialog, a dialog for selecting a file reader
 *        and configuring its options before loading a file.
 *
 * \ingroup QmitkModule
 */

#ifndef QmitkFileReaderOptionsDialog_h
#define QmitkFileReaderOptionsDialog_h

#include <mitkIOUtil.h>

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkFileReaderOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

/**
 * \brief Dialog that presents available file readers and their options for a given file.
 *
 * When multiple readers are registered for a file's MIME type, this dialog lets the user
 * choose which reader to use and configure reader-specific options. The selected reader
 * and its options are written back to the provided mitk::IOUtil::LoadInfo upon acceptance.
 *
 * \ingroup QmitkModule
 */
class QmitkFileReaderOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the dialog for the given load information.
   *
   * Populates the reader combo box and stacked option widgets from the
   * available readers in \a loadInfo.
   *
   * \param loadInfo Reference to the LoadInfo containing the file path and reader selector.
   * \param parent   Optional parent widget.
   */
  explicit QmitkFileReaderOptionsDialog(mitk::IOUtil::LoadInfo &loadInfo, QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkFileReaderOptionsDialog() override;

  /**
   * \brief Check whether the user requested to reuse these options for subsequent files.
   *
   * \return \c true if the "reuse options" checkbox is checked, \c false otherwise.
   */
  bool ReuseOptions() const;

  /**
   * \brief Accept the dialog, applying the selected reader and its options to the LoadInfo.
   */
  void accept() override;

protected slots:
  /**
   * \brief Switch the displayed options widget when the user selects a different reader.
   *
   * \param index The index of the newly selected reader in the combo box.
   */
  void SetCurrentReader(int index);

private:
  std::unique_ptr<Ui::QmitkFileReaderOptionsDialog> ui; ///< UI controls.
  mitk::IOUtil::LoadInfo &m_LoadInfo; ///< Reference to the load information being configured.
  std::vector<mitk::FileReaderSelector::Item> m_ReaderItems; ///< Cached list of available reader items.
};

#endif
