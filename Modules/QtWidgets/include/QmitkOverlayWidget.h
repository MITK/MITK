/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITK_OVERLAY_WIDGET_H
#define QMITK_OVERLAY_WIDGET_H

#include <QWidget>

#include <MitkQtWidgetsExports.h>

/** Simple widget that can be used to achive overlays. The overlay will lie above its parent.
 * This implementation just renders a semi transparent black background. To add content to the
 * overlay derive from this class.*/
class MITKQTWIDGETS_EXPORT QmitkOverlayWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool transparentForMouseEvents READ isTransparentForMouseEvents WRITE setTransparentForMouseEvents)

public:
  explicit QmitkOverlayWidget(QWidget* parent = nullptr);
  ~QmitkOverlayWidget() override;

  bool isTransparentForMouseEvents() const;
  void setTransparentForMouseEvents(bool transparent = true);

protected:
  bool event(QEvent* e) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  void installEventFilterOnParent();
  void removeEventFilterFromParent();
};

#endif
