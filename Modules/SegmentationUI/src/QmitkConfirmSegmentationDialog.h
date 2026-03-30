/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkConfirmSegmentationDialog_h
#define QmitkConfirmSegmentationDialog_h

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkConfirmSegmentationDialog;
}

/**
 * \brief Dialog asking the user whether to overwrite an existing segmentation or create a new one.
 *
 * Presents three options: overwrite the existing segmentation, create a new
 * segmentation, or cancel the operation.
 */
class QmitkConfirmSegmentationDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the confirmation dialog.
   * \param parent Optional parent widget.
   */
  explicit QmitkConfirmSegmentationDialog(QWidget *parent = nullptr);
  ~QmitkConfirmSegmentationDialog() override;

  /**
   * \brief Set the name of the segmentation to display in the dialog.
   * \param name The segmentation name.
   */
  void SetSegmentationName(QString name);

  enum
  {
    OVERWRITE_SEGMENTATION,
    CREATE_NEW_SEGMENTATION,
    CANCEL_SEGMENTATION
  };

protected slots:

  void OnOverwriteExistingSegmentation();

  void OnCreateNewSegmentation();

  void OnCancelSegmentation();

private:
  std::unique_ptr<Ui::QmitkConfirmSegmentationDialog> m_Controls;
};

#endif
