/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCopyLabelToGroupDialog_h
#define QmitkCopyLabelToGroupDialog_h

#include <MitkSegmentationUIExports.h>
#include <QDialog>
#include <memory>

namespace mitk
{
  class Label;
  class MultiLabelSegmentation;
}

namespace Ui
{
  class QmitkCopyLabelToGroupDialog;
}

/**
 * \brief Dialog for copying a label to another group within a multi-label segmentation.
 *
 * Allows the user to select a destination group within the segmentation and copies the
 * source label to that group. The destination label will have a different label value since
 * all label values in a MultiLabelSegmentation must be unique.
 *
 * \sa QmitkMultiLabelInspector
 * \sa mitk::MultiLabelSegmentation
 */
class MITKSEGMENTATIONUI_EXPORT QmitkCopyLabelToGroupDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the dialog for copying a label to another group.
   * \param[in] segmentation The multi-label segmentation containing the label.
   * \param[in] label The source label to copy.
   * \param[in] parent Optional parent widget.
   */
  QmitkCopyLabelToGroupDialog(mitk::MultiLabelSegmentation* segmentation, mitk::Label* label, QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkCopyLabelToGroupDialog() override;

  /** \brief Accepts the dialog and performs the label copy operation. */
  void accept() override;

  /**
   * \brief Returns the destination label after the copy operation.
   * \return Pointer to the newly created destination label, or nullptr if not yet accepted.
   */
  mitk::Label* GetDestinationLabel() const;

private:
  std::unique_ptr<Ui::QmitkCopyLabelToGroupDialog> m_Ui;

  mitk::MultiLabelSegmentation* m_Segmentation;
  mitk::Label* m_SourceLabel;
  mitk::Label* m_DestinationLabel;
};

#endif
