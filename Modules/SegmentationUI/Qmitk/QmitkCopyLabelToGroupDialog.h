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

namespace mitk
{
  class Label;
  class LabelSetImage;
}

namespace Ui
{
  class QmitkCopyLabelToGroupDialog;
}

class MITKSEGMENTATIONUI_EXPORT QmitkCopyLabelToGroupDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkCopyLabelToGroupDialog(mitk::LabelSetImage* segmentation, mitk::Label* label, QWidget* parent = nullptr);
  ~QmitkCopyLabelToGroupDialog() override;

  void accept() override;

  mitk::Label* GetDestinationLabel() const;

private:
  Ui::QmitkCopyLabelToGroupDialog* m_Ui;

  mitk::LabelSetImage* m_Segmentation;
  mitk::Label* m_SourceLabel;
  mitk::Label* m_DestinationLabel;
};

#endif
