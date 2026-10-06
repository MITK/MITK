/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFeedbackDialog_h
#define QmitkFeedbackDialog_h

#include <mitkEnvironmentInfo.h>
#include <mitkFeedbackReport.h>

#include <QDialog>

#include <memory>
#include <vector>

class QCheckBox;

namespace Ui
{
  class QmitkFeedbackDialog;
}

/**
 * \brief Lets the user assemble, review, and hand off a feedback report.
 *
 * Every item that may go into the report is shown with its own include
 * choice, and the preview shows the exact report text that a hand-off
 * emits.
 */
class QmitkFeedbackDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkFeedbackDialog(const mitk::EnvironmentInfo& environment, QWidget* parent = nullptr);
  ~QmitkFeedbackDialog() override;

private:
  void AddEnvironmentItems(const mitk::EnvironmentInfo& environment);

  /** Exactly what the preview shows and a hand-off emits. */
  mitk::FeedbackReport AssembleReport() const;

  void UpdatePreview();

  std::unique_ptr<Ui::QmitkFeedbackDialog> m_Controls;
  std::vector<std::pair<QCheckBox*, mitk::FeedbackItem>> m_EnvironmentItems;
};

#endif
