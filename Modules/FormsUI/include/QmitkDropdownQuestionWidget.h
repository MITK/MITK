/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDropdownQuestionWidget_h
#define QmitkDropdownQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkDropdownQuestion.h>

class QComboBox;

class MITKFORMSUI_EXPORT QmitkDropdownQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkDropdownQuestionWidget(QWidget* parent = nullptr);
  ~QmitkDropdownQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnCurrentIndexChanged(int index);

  mitk::Forms::DropdownQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QComboBox* m_ComboBox;
};

#endif
