/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file QmitkFileWriterOptionsDialog.h
 * \brief Declares QmitkFileWriterOptionsDialog, a dialog for selecting a file writer
 *        and configuring its options before saving data.
 *
 * \ingroup QmitkModule
 */

#ifndef QmitkFileWriterOptionsDialog_h
#define QmitkFileWriterOptionsDialog_h

#include <mitkIOUtil.h>

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkFileWriterOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

/**
 * \brief Dialog that presents available file writers and their options for a given save operation.
 *
 * When multiple writers are registered for a data object's MIME type, this dialog lets
 * the user choose which writer to use and configure writer-specific options. The selected
 * writer and its options are written back to the provided mitk::IOUtil::SaveInfo upon acceptance.
 *
 * \ingroup QmitkModule
 */
class QmitkFileWriterOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the dialog for the given save information.
   *
   * Populates the writer combo box and stacked option widgets from the
   * available writers in \a saveInfo.
   *
   * \param saveInfo Reference to the SaveInfo containing the file path and writer selector.
   * \param parent   Optional parent widget.
   */
  explicit QmitkFileWriterOptionsDialog(mitk::IOUtil::SaveInfo &saveInfo, QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkFileWriterOptionsDialog() override;

  /**
   * \brief Check whether the user requested to reuse these options for subsequent files.
   *
   * \return \c true if the "reuse options" checkbox is checked, \c false otherwise.
   */
  bool ReuseOptions() const;

  /**
   * \brief Accept the dialog, applying the selected writer and its options to the SaveInfo.
   */
  void accept() override;

private:
  std::unique_ptr<Ui::QmitkFileWriterOptionsDialog> ui; ///< UI controls.
  mitk::IOUtil::SaveInfo &m_SaveInfo; ///< Reference to the save information being configured.
  std::vector<mitk::FileWriterSelector::Item> m_WriterItems; ///< Cached list of available writer items.
};

#endif
