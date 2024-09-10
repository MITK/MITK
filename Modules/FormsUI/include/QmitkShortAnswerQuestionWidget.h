/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkShortAnswerQuestionWidget_h
#define QmitkShortAnswerQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkShortAnswerQuestion.h>

class QLineEdit;

class MITKFORMSUI_EXPORT QmitkShortAnswerQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkShortAnswerQuestionWidget(QWidget* parent = nullptr);
  ~QmitkShortAnswerQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void OnTextEdited(const QString& text);

  mitk::Forms::ShortAnswerQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QLineEdit* m_LineEdit;
};

#endif
