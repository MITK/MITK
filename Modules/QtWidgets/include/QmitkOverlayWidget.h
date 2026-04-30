/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOverlayWidget_h
#define QmitkOverlayWidget_h

#include <QWidget>

#include <MitkQtWidgetsExports.h>

/**
 * \brief Simple widget that can be used to achieve overlays above a parent widget.
 *
 * This implementation just renders a semi transparent black background. To add content to the
 * overlay derive from this class.
 *
 * \sa QmitkSimpleTextOverlayWidget
 * \sa QmitkButtonOverlayWidget
 */
class MITKQTWIDGETS_EXPORT QmitkOverlayWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool transparentForMouseEvents READ isTransparentForMouseEvents WRITE setTransparentForMouseEvents)

public:
  explicit QmitkOverlayWidget(QWidget* parent = nullptr);
  ~QmitkOverlayWidget() override;

  /** \brief Returns whether mouse events pass through the overlay to the parent. */
  bool isTransparentForMouseEvents() const;
  /**
   * \brief Sets whether mouse events pass through the overlay.
   * \param[in] transparent If true, mouse events are forwarded to the parent widget.
   */
  void setTransparentForMouseEvents(bool transparent = true);

  /** \brief Returns the current opacity value (0-255). */
  int getOpacity() const;

public slots:
  /**
   * \brief Sets the overlay opacity.
   * \param[in] opacity The opacity value (0 = fully transparent, 255 = fully opaque).
   */
  void setOpacity(int opacity);

protected:
  bool event(QEvent* e) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  void installEventFilterOnParent();
  void removeEventFilterFromParent();

  int m_Opacity = 63;

};

#endif
