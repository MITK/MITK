/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLinearScaleQuestionWidget_h
#define QmitkLinearScaleQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkLinearScaleQuestion.h>

class QButtonGroup;
class QGridLayout;
class QPushButton;

class MITKFORMSUI_EXPORT QmitkLinearScaleQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkLinearScaleQuestionWidget(QWidget* parent = nullptr);
  ~QmitkLinearScaleQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnIdClicked(int id);
  void OnClearButtonClicked();

  mitk::Forms::LinearScaleQuestion* m_Question;

  QGridLayout* m_Layout;
  QButtonGroup* m_ButtonGroup;
  QPushButton* m_ClearButton;
};

#endif
