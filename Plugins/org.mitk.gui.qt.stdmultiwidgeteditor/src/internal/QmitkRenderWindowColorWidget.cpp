/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowColorWidget.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QLinearGradient>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSignalBlocker>

namespace
{
  // Thickness of the decoration-colored frame. Thicker than the real 2 px
  // render-window border so it reads clearly and is a comfortable click target
  // for picking the decoration color.
  constexpr int FRAME_WIDTH = 12;
  constexpr int ANNOTATION_PADDING = 2;
}

QmitkRenderWindowColorWidget::QmitkRenderWindowColorWidget(QWidget* parent)
  : QWidget(parent),
    m_UpperColor(Qt::black),
    m_LowerColor(Qt::black),
    m_DecorationColor(Qt::white),
    m_AnnotationEdit(new QLineEdit(this)),
    m_GradientCheckBox(new QCheckBox("Gradient", this)),
    m_LogoVisible(true)
{
  this->setMinimumSize(140, 130);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  sizePolicy.setHeightForWidth(true);
  this->setSizePolicy(sizePolicy);

  m_AnnotationEdit->setFrame(false);

  m_GradientCheckBox->setToolTip("Enable a background gradient (adds a second color).");

  connect(m_GradientCheckBox, &QCheckBox::toggled, this, &QmitkRenderWindowColorWidget::OnGradientToggled);

  this->UpdateAnnotationStyle();
  this->UpdateToolTip();
}

QmitkRenderWindowColorWidget::~QmitkRenderWindowColorWidget()
{
}

void QmitkRenderWindowColorWidget::SetUpperColor(const QColor& color)
{
  m_UpperColor = color;
  this->update();
}

QColor QmitkRenderWindowColorWidget::GetUpperColor() const
{
  return m_UpperColor;
}

void QmitkRenderWindowColorWidget::SetLowerColor(const QColor& color)
{
  m_LowerColor = color;
  this->update();
}

QColor QmitkRenderWindowColorWidget::GetLowerColor() const
{
  return m_LowerColor;
}

void QmitkRenderWindowColorWidget::SetDecorationColor(const QColor& color)
{
  m_DecorationColor = color;
  this->UpdateAnnotationStyle();
  this->update();
}

QColor QmitkRenderWindowColorWidget::GetDecorationColor() const
{
  return m_DecorationColor;
}

void QmitkRenderWindowColorWidget::SetAnnotation(const QString& text)
{
  m_AnnotationEdit->setText(text);
}

QString QmitkRenderWindowColorWidget::GetAnnotation() const
{
  return m_AnnotationEdit->text();
}

void QmitkRenderWindowColorWidget::SetGradientEnabled(bool enabled)
{
  // Block the toggle handler: this is a programmatic update (preferences load
  // or 2D-window sync), so the lower color is taken as-is, not reset, and no
  // BackgroundChanged is emitted.
  QSignalBlocker blocker(m_GradientCheckBox);
  m_GradientCheckBox->setChecked(enabled);
  this->UpdateToolTip();
  this->update();
}

bool QmitkRenderWindowColorWidget::IsGradientEnabled() const
{
  return m_GradientCheckBox->isChecked();
}

void QmitkRenderWindowColorWidget::SetLogo(const QPixmap& logo)
{
  m_Logo = logo;
  this->UpdateToolTip();
  this->update();
}

void QmitkRenderWindowColorWidget::SetLogoVisible(bool visible)
{
  m_LogoVisible = visible;
  this->update();
}

bool QmitkRenderWindowColorWidget::IsLogoVisible() const
{
  return m_LogoVisible;
}

QRect QmitkRenderWindowColorWidget::PreviewRect() const
{
  const int stripHeight = m_GradientCheckBox->sizeHint().height();
  return QRect(0, 0, this->width(), this->height() - stripHeight);
}

int QmitkRenderWindowColorWidget::heightForWidth(int width) const
{
  // Keep the render-window preview at a 4:3 aspect ratio; the checkbox strip
  // sits below it.
  return (width * 3) / 4 + m_GradientCheckBox->sizeHint().height();
}

QRect QmitkRenderWindowColorWidget::LogoRect() const
{
  if (m_Logo.isNull())
    return QRect();

  const QRect interior = this->PreviewRect().adjusted(FRAME_WIDTH, FRAME_WIDTH, -FRAME_WIDTH, -FRAME_WIDTH);
  const int boxWidth = interior.width() / 2 - ANNOTATION_PADDING;
  const int boxHeight = interior.height() / 2;

  if (boxWidth <= 0 || boxHeight <= 0)
    return QRect();

  // Occupy the bottom-right half (mirroring the annotation on the left), keep
  // the logo's aspect ratio, and anchor it to the bottom-right corner.
  QSize logoSize = m_Logo.size();
  logoSize.scale(boxWidth, boxHeight, Qt::KeepAspectRatio);

  const int x = interior.right() - ANNOTATION_PADDING - logoSize.width();
  const int y = interior.bottom() - ANNOTATION_PADDING - logoSize.height();

  return QRect(QPoint(x, y), logoSize);
}

void QmitkRenderWindowColorWidget::paintEvent(QPaintEvent*)
{
  const QRect previewRect = this->PreviewRect();
  const QRect interior = previewRect.adjusted(FRAME_WIDTH, FRAME_WIDTH, -FRAME_WIDTH, -FRAME_WIDTH);

  QPainter painter(this);

  // Paint the decoration color across the whole preview; it stays visible as
  // the frame once the background is painted on top, inset by the frame width.
  // Filling the background strictly inside the frame keeps it from bleeding
  // past the edge as a thin outer ring.
  painter.fillRect(previewRect, m_DecorationColor);

  if (m_GradientCheckBox->isChecked())
  {
    QLinearGradient gradient(interior.topLeft(), interior.bottomLeft());
    gradient.setColorAt(0.0, m_UpperColor);
    gradient.setColorAt(1.0, m_LowerColor);
    painter.fillRect(interior, gradient);

    // Mark the boundary between the two clickable color halves (the same split
    // the click hit-test uses) with a dashed line in the decoration color, so
    // it is clear there are two colors to set.
    const int splitY = interior.center().y();
    QPen dashPen(m_DecorationColor);
    dashPen.setStyle(Qt::DashLine);
    painter.setPen(dashPen);
    painter.drawLine(interior.left(), splitY, interior.right(), splitY);
  }
  else
  {
    painter.fillRect(interior, m_UpperColor);
  }

  if (!m_Logo.isNull())
  {
    const QRect logoRect = this->LogoRect();
    if (logoRect.isValid())
    {
      painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setOpacity(m_LogoVisible ? 1.0 : 0.35);
      painter.drawPixmap(logoRect, m_Logo);
      painter.setOpacity(1.0);

      if (!m_LogoVisible)
      {
        // A red cross over the dimmed logo marks it as hidden.
        const QColor crossColor(Qt::red);
        QPen crossPen(crossColor);
        crossPen.setWidth(2);
        painter.setPen(crossPen);
        painter.drawLine(logoRect.topLeft(), logoRect.bottomRight());
        painter.drawLine(logoRect.topRight(), logoRect.bottomLeft());
      }
    }
  }
}

void QmitkRenderWindowColorWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  const int stripHeight = m_GradientCheckBox->sizeHint().height();
  m_GradientCheckBox->setGeometry(0, this->height() - stripHeight, this->width(), stripHeight);

  const QRect interior = this->PreviewRect().adjusted(FRAME_WIDTH, FRAME_WIDTH, -FRAME_WIDTH, -FRAME_WIDTH);
  const int annotationHeight = m_AnnotationEdit->sizeHint().height();
  m_AnnotationEdit->setGeometry(interior.left() + ANNOTATION_PADDING,
                                interior.bottom() - annotationHeight - ANNOTATION_PADDING,
                                interior.width() / 2,
                                annotationHeight);
}

void QmitkRenderWindowColorWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton)
    return;

  const QRect previewRect = this->PreviewRect();
  const QPoint pos = event->pos();

  if (!previewRect.contains(pos))
    return;

  // The logo toggle takes precedence over the background it is drawn on.
  if (!m_Logo.isNull() && this->LogoRect().contains(pos))
  {
    this->SetLogoVisible(!m_LogoVisible);
    return;
  }

  const QRect interior = previewRect.adjusted(FRAME_WIDTH, FRAME_WIDTH, -FRAME_WIDTH, -FRAME_WIDTH);

  if (!interior.contains(pos))
  {
    // On the frame: decoration color.
    if (this->PickColor(m_DecorationColor))
      this->UpdateAnnotationStyle();
  }
  else if (m_GradientCheckBox->isChecked() && pos.y() > interior.center().y())
  {
    if (this->PickColor(m_LowerColor))
      emit BackgroundChanged();
  }
  else
  {
    if (this->PickColor(m_UpperColor))
    {
      // Without a gradient there is only one color; keep the lower color in
      // step so the state stays consistent (gradient off => lower == upper).
      if (!m_GradientCheckBox->isChecked())
        m_LowerColor = m_UpperColor;

      emit BackgroundChanged();
    }
  }
}

bool QmitkRenderWindowColorWidget::PickColor(QColor& target)
{
  const QColor picked = QColorDialog::getColor(target, this);

  if (!picked.isValid())
    return false;

  target = picked;
  this->update();

  return true;
}

void QmitkRenderWindowColorWidget::UpdateAnnotationStyle()
{
  m_AnnotationEdit->setStyleSheet(
    QString("QLineEdit { background: transparent; border: none; color: %1; }").arg(m_DecorationColor.name()));
}

void QmitkRenderWindowColorWidget::UpdateToolTip()
{
  const QString backgroundHint = m_GradientCheckBox->isChecked()
    ? "Click top or bottom to change gradient color stops."
    : "Click to change background color.";

  this->setToolTip(backgroundHint + "\n"
                   "Click the frame to change decoration color.\n"
                   "Click the corner annotation to edit.");
}

void QmitkRenderWindowColorWidget::OnGradientToggled(bool enabled)
{
  // Turning the gradient off collapses the window to a single color, so the
  // preview (and, on save, the stored second color) mirrors the upper one.
  if (!enabled)
    m_LowerColor = m_UpperColor;

  this->UpdateToolTip();
  this->update();

  emit BackgroundChanged();
}
