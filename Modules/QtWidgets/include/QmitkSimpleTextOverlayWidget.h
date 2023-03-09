/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleTextOverlayWidget_h
#define QmitkSimpleTextOverlayWidget_h

#include "QmitkOverlayWidget.h"
#include <MitkQtWidgetsExports.h>

/** Simple overlay that renders a passed string.
 You may pass an html string that will be rendered accordingly
 respecting the current application style sheet.*/
class MITKQTWIDGETS_EXPORT QmitkSimpleTextOverlayWidget : public QmitkOverlayWidget
{
  Q_OBJECT
  Q_PROPERTY(QString overlayText READ GetOverlayText WRITE SetOverlayText)

public:
  explicit QmitkSimpleTextOverlayWidget(QWidget* parent = nullptr);
  ~QmitkSimpleTextOverlayWidget() override;

  QString GetOverlayText() const;
  void SetOverlayText(const QString& text);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  QString m_Text;
};

#endif
