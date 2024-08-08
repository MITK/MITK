/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkQuestionWidget_h
#define QmitkQuestionWidget_h

#include <MitkFormsUIExports.h>
#include <QFrame>

class QVBoxLayout;
class QLabel;

namespace mitk::Forms
{
  class Question;
}

class MITKFORMSUI_EXPORT QmitkQuestionWidget : public QFrame
{
  Q_OBJECT

public:
  explicit QmitkQuestionWidget(QWidget* parent = nullptr);
  ~QmitkQuestionWidget() override;

  virtual QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const = 0;
  virtual mitk::Forms::Question* GetQuestion() const = 0;
  virtual void SetQuestion(mitk::Forms::Question* question) = 0;
  virtual void Reset() = 0;

  void SetRequirementVisible(bool visible);
  void ShowRequirement();
  void HideRequirement();

protected:
  void InsertLayout(QLayout* layout);

private:
  QVBoxLayout* m_Layout;
  QLabel* m_QuestionLabel;
  QLabel* m_RequiredLabel;
};

#endif
