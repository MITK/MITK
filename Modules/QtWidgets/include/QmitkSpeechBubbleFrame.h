/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSpeechBubbleFrame_h
#define QmitkSpeechBubbleFrame_h

#include <MitkQtWidgetsExports.h>

#include <QColor>
#include <QPalette>
#include <QWidget>

/**
 * \brief A frame in the shape of a speech bubble, like the VoxTell icon, around a text field.
 *
 * The tail hangs from the bottom edge, right of the center. The frame leaves
 * room for its outline and the tail around its layout. Give the text field
 * that it holds no frame and a transparent background, so the bubble shows.
 *
 * The outline turns into the blue of the VoxTell icon while the text field
 * has the focus, and is grayed out while it is disabled.
 */
class MITKQTWIDGETS_EXPORT QmitkSpeechBubbleFrame : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSpeechBubbleFrame(QWidget* parent = nullptr);
  ~QmitkSpeechBubbleFrame() override;

  /**
   * \brief The fill color of the bubble in the current theme.
   *
   * For other places that show what the user wrote, so that they look like
   * the bubble.
   */
  static QColor GetFillColor(const QPalette& palette, bool enabled = true);

  /** \brief The outline color of the bubble in the current theme, for the same use as GetFillColor(). */
  static QColor GetBorderColor(const QPalette& palette, bool enabled = true);

protected:
  /** \brief Watches the children for changes of their focus and enabled state. */
  void childEvent(QChildEvent* event) override;

  bool eventFilter(QObject* watched, QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
};

#endif
