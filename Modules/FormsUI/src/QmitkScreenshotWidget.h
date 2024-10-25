/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScreenshotWidget_h
#define QmitkScreenshotWidget_h

#include <QWidget>

namespace Ui
{
  class QmitkScreenshotWidget;
}

class QmitkScreenshotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkScreenshotWidget(QWidget* parent = nullptr);
  explicit QmitkScreenshotWidget(const QString& screenshotPath, QWidget* parent = nullptr);
  ~QmitkScreenshotWidget() override;

  void SetScreenshot(const QString& path);
  QString GetScreenshotPath() const;

signals:
  void Remove(QmitkScreenshotWidget* sender);

private:
  void OnRemoveButtonClicked();

  Ui::QmitkScreenshotWidget* m_Ui;
  QString m_ScreenshotPath;
};

#endif
