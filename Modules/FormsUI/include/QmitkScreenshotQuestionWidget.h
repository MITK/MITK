/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScreenshotQuestionWidget_h
#define QmitkScreenshotQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkScreenshotQuestion.h>

class QMenu;
class QPushButton;

class MITKFORMSUI_EXPORT QmitkScreenshotQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  explicit QmitkScreenshotQuestionWidget(QWidget* parent = nullptr);
  ~QmitkScreenshotQuestionWidget() override;

  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;
  mitk::Forms::Question* GetQuestion() const override;
  void SetQuestion(mitk::Forms::Question* question) override;
  void Reset() override;

private:
  void OnTakeScreenshotButtonClicked();
  void OnPopupMenuTriggered(const QString& action);

  mitk::Forms::ScreenshotQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QPushButton* m_TakeScreenshotButton;
  QMenu* m_PopupMenu;
};

#endif
