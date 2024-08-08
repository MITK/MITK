/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultipleChoiceQuestionWidget_h
#define QmitkMultipleChoiceQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkMultipleChoiceQuestion.h>

class QButtonGroup;
class QGridLayout;
class QLineEdit;
class QPushButton;

class MITKFORMSUI_EXPORT QmitkMultipleChoiceQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkMultipleChoiceQuestionWidget(QWidget* parent = nullptr);
  ~QmitkMultipleChoiceQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnIdClicked(int id);
  void OnTextEdited(const QString& text);
  void OnEditingFinished();
  void OnClearButtonClicked();

  mitk::Forms::MultipleChoiceQuestion* m_Question;

  QGridLayout* m_Layout;
  QButtonGroup* m_ButtonGroup;
  QLineEdit* m_OtherLineEdit;
  int m_OtherId;
  QPushButton* m_ClearButton;
};

#endif
