/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkParagraphQuestionWidget_h
#define QmitkParagraphQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkParagraphQuestion.h>

class QTextEdit;

class MITKFORMSUI_EXPORT QmitkParagraphQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkParagraphQuestionWidget(QWidget* parent = nullptr);
  ~QmitkParagraphQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void AdjustHeight();
  void OnTextChanged();

  mitk::Forms::ParagraphQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QTextEdit* m_TextEdit;
};

#endif
