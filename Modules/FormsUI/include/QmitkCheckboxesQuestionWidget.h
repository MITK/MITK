/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCheckboxQuestionWidget_h
#define QmitkCheckboxQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkCheckboxesQuestion.h>

class QButtonGroup;
class QGridLayout;
class QLineEdit;

class MITKFORMSUI_EXPORT QmitkCheckboxesQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkCheckboxesQuestionWidget(QWidget* parent = nullptr);
  ~QmitkCheckboxesQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnIdToggled(int id, bool checked);
  void OnTextEdited(const QString& text);
  void OnEditingFinished();

  mitk::Forms::CheckboxesQuestion* m_Question;

  QGridLayout* m_Layout;
  QButtonGroup* m_ButtonGroup;
  QLineEdit* m_OtherLineEdit;
  int m_OtherId;
};

#endif
