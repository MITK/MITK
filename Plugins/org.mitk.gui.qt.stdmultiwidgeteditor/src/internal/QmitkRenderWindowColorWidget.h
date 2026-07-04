/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowColorWidget_h
#define QmitkRenderWindowColorWidget_h

#include <QColor>
#include <QPixmap>
#include <QWidget>

class QCheckBox;
class QLineEdit;

/**
 * @brief Interactive preview of a single render window's colors.
 *
 * Draws a miniature of one render window - a background (solid, or a vertical
 * gradient from the upper to the lower color) inside a frame painted in the
 * decoration color, with the corner annotation overlaid in that same color.
 * The preview doubles as its own editor: clicking the upper or lower half of
 * the background picks the respective background color, clicking the frame
 * picks the decoration color, and the annotation is edited in place. A
 * "Gradient" checkbox controls whether the lower color is used at all; with
 * the gradient off the lower color mirrors the upper one, so a single color
 * fully describes the window.
 */
class QmitkRenderWindowColorWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkRenderWindowColorWidget(QWidget* parent = nullptr);
  ~QmitkRenderWindowColorWidget() override;

  void SetUpperColor(const QColor& color);
  QColor GetUpperColor() const;

  void SetLowerColor(const QColor& color);
  QColor GetLowerColor() const;

  void SetDecorationColor(const QColor& color);
  QColor GetDecorationColor() const;

  void SetAnnotation(const QString& text);
  QString GetAnnotation() const;

  void SetGradientEnabled(bool enabled);
  bool IsGradientEnabled() const;

  /** @brief Set the logo shown in the bottom-right corner. A null pixmap disables the logo control. */
  void SetLogo(const QPixmap& logo);

  void SetLogoVisible(bool visible);
  bool IsLogoVisible() const;

  int heightForWidth(int width) const override;

signals:
  /** @brief Emitted when the user changes a background color or toggles the gradient. */
  void BackgroundChanged();

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

private:
  /** @brief The painted render-window preview, excluding the checkbox strip below it. */
  QRect PreviewRect() const;

  /** @brief Rectangle the logo is drawn in (bottom-right of the preview); empty if there is no logo. */
  QRect LogoRect() const;

  /** @brief Open a color dialog seeded with \p target; on accept store it, repaint and return true. */
  bool PickColor(QColor& target);

  void UpdateAnnotationStyle();
  void UpdateToolTip();
  void OnGradientToggled(bool enabled);

  QColor m_UpperColor;
  QColor m_LowerColor;
  QColor m_DecorationColor;

  QLineEdit* m_AnnotationEdit;
  QCheckBox* m_GradientCheckBox;

  QPixmap m_Logo;
  bool m_LogoVisible;
};

#endif
