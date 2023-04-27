/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkButtonOverlayWidget_h
#define QmitkButtonOverlayWidget_h

#include "QmitkOverlayWidget.h"
#include <MitkQtWidgetsExports.h>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

/** Overlay that renders a passed string and draws an icon as a push button.
 You may pass an html string that will be rendered accordingly
 respecting the current application style sheet.
 The button will return the 'Clicked' signal which can be connected to
 a slot in the calling class.*/
class MITKQTWIDGETS_EXPORT QmitkButtonOverlayWidget : public QmitkOverlayWidget
{
  Q_OBJECT

public:

  explicit QmitkButtonOverlayWidget(QWidget* parent = nullptr);
  ~QmitkButtonOverlayWidget() override;

  QString GetOverlayText() const;
  void SetOverlayText(const QString& text);
  QString GetButtonText() const;
  void SetButtonText(const QString& text);
  QIcon GetButtonIcon() const;
  void SetButtonIcon(const QIcon& icon);

Q_SIGNALS:

  void Clicked();

private:

  QLabel* m_MessageLabel;
  QPushButton* m_PushButton;

};

#endif
