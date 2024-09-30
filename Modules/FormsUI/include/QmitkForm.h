/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkForm_h
#define QmitkForm_h

#include <mitkForm.h>

#include <MitkFormsUIExports.h>

#include <QWidget>

namespace Ui
{
  class QmitkForm;
}

class MITKFORMSUI_EXPORT QmitkForm : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkForm(QWidget* parent = nullptr);
  explicit QmitkForm(mitk::Forms::Form* form, QWidget* parent = nullptr);
  ~QmitkForm() override;

  mitk::Forms::Form* GetForm() const;
  void SetForm(mitk::Forms::Form* form);

  fs::path GetResponsesPath() const;
  void SetResponsesPath(const fs::path& csvPath);

signals:
  void Submit();

private:
  void CreateQuestionWidgets();
  bool ValidateCurrentSection();
  void Reset();

  void Update();
  void UpdateFormHeader();
  void UpdateSubmittedHeader();
  void UpdateSectionHeader();
  void UpdateQuestionWidgets();
  void UpdateFormButtons();

  void OnBackButtonClicked();
  void OnNextButtonClicked();
  void OnSubmitButtonClicked();
  void OnClearButtonClicked();
  void OnSubmitAnotherButtonClicked();

  Ui::QmitkForm* m_Ui;
  mitk::Forms::Form* m_Form;
  fs::path m_ResponsesPath;
  bool m_HasBeenSubmitted;
};

#endif
