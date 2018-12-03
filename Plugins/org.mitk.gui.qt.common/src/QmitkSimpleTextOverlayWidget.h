/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITK_SIMPLE_TEXT_OVERLAY_WIDGET_H
#define QMITK_SIMPLE_TEXT_OVERLAY_WIDGET_H

#include "QmitkOverlayWidget.h"
#include "org_mitk_gui_qt_common_Export.h"

/** Simple overlay that renders a passed string.
 You may pass an html string that will be rendered accordingly
 respecting the current application style sheet.*/
class MITK_QT_COMMON QmitkSimpleTextOverlayWidget : public QmitkOverlayWidget
{
  Q_OBJECT
  Q_PROPERTY(QString overlayText READ GetOverlayText WRITE SetOverlayText)

public:
  explicit QmitkSimpleTextOverlayWidget(QWidget* parent = nullptr);
  virtual ~QmitkSimpleTextOverlayWidget();

  QString GetOverlayText() const;
  void SetOverlayText(const QString& text);

protected:
  virtual void paintEvent(QPaintEvent* event) override;

private:
  QString m_Text;
};

#endif
