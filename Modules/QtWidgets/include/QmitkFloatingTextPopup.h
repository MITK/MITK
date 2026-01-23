/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFloatingTextPopup_h
#define QmitkFloatingTextPopup_h

#include <QWidget>
#include <QString>
#include <MitkQtWidgetsExports.h>


/** Self-contained floating semi-translucent popup widget that displays HTML content
 * and automatically follows the mouse cursor across the entire screen.
 *
 * The popup manages its own mouse tracking and visibility. Simply set the HTML
 * content and call SetEnabled(true) to activate it.*/
class MITKQTWIDGETS_EXPORT QmitkFloatingTextPopup : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString htmlContent READ GetHtmlContent WRITE SetHtmlContent)
  Q_PROPERTY(int opacity READ GetOpacity WRITE SetOpacity)
  Q_PROPERTY(bool popupEnabled READ IsEnabled WRITE SetEnabled)

public:
  explicit QmitkFloatingTextPopup(QWidget* parent = nullptr);
  ~QmitkFloatingTextPopup() override;

  QString GetHtmlContent() const;
  void SetHtmlContent(const QString& html);

  int GetOpacity() const;
  void SetOpacity(int opacity);

  /** Enable or disable the popup. When enabled and content is set,
   * the popup will follow the mouse cursor. */
  bool IsEnabled() const;
  void SetEnabled(bool enabled);

protected:
  void paintEvent(QPaintEvent* event) override;
  QSize sizeHint() const override;
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void updatePosition(const QPoint& globalPos);
  void updateGeometry();
  void updateVisibility();
  QPoint constrainToScreen(const QPoint& pos, const QSize& size) const;
  void installGlobalEventFilter();
  void removeGlobalEventFilter();

  QString m_HtmlContent;
  int m_Opacity = 200; // 0-255 range
  bool m_Enabled = false;
  bool m_EventFilterInstalled = false;
  QPoint m_CursorOffset{30, 30}; // Offset from cursor
  static constexpr int m_Padding = 2;
  static constexpr int m_MaxWidth = 200;
};

#endif
