/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSliderColorBar.h"

#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QResizeEvent>
#include <QSlider>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QVBoxLayout>

class QmitkSliderColorBar::Bar : public QWidget
{
public:
  explicit Bar(QWidget* parent)
    : QWidget(parent)
  {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }

  void SetSlider(QSlider* slider)
  {
    m_Slider = slider;
    this->update();
  }

  void SetColors(const std::vector<QColor>& colors)
  {
    if (colors == m_Colors)
      return;

    m_Colors = colors;
    this->update();
  }

  QSize sizeHint() const override
  {
    return QSize(100, Height);
  }

  QSize minimumSizeHint() const override
  {
    return QSize(0, Height);
  }

protected:
  void paintEvent(QPaintEvent*) override
  {
    if (m_Slider.isNull())
      return;

    const int minimum = m_Slider->minimum();
    const int maximum = m_Slider->maximum();

    if (m_Colors.empty() || static_cast<int>(m_Colors.size()) != maximum - minimum + 1)
      return;

    // Filled in like QSlider does for itself. Style sheets select their slider
    // rules by the horizontal state, and without it a handle has no extent.
    QStyleOptionSlider option;
    option.initFrom(m_Slider);
    option.state |= QStyle::State_Horizontal;
    option.orientation = Qt::Horizontal;
    option.upsideDown = m_Slider->invertedAppearance() != (option.direction == Qt::RightToLeft);
    option.minimum = minimum;
    option.maximum = maximum;

    auto* style = m_Slider->style();

    auto toBar = [&](const QPoint& sliderPoint) {
      return this->mapFromGlobal(m_Slider->mapToGlobal(sliderPoint)).x();
    };

    // Asked of the slider's own style rather than computed, since styles differ
    // in how far the handle travels and where its range starts.
    auto handleCenter = [&](int value) {
      option.sliderPosition = value;
      option.sliderValue = value;
      return toBar(style->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, m_Slider).center());
    };

    const auto groove = style->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, m_Slider);
    const int grooveRight = toBar(groove.topRight()) + 1;

    QPainter painter(this);

    // Faded like the rest of a disabled widget, which the colors would
    // otherwise not be.
    if (!this->isEnabled())
      painter.setOpacity(0.35);

    int left = toBar(groove.topLeft());

    for (int value = minimum; value <= maximum;)
    {
      const auto& color = m_Colors[value - minimum];
      int last = value;

      while (last < maximum && m_Colors[last + 1 - minimum] == color)
        ++last;

      // Neighboring values share the space between their handle positions.
      const int right = last < maximum
        ? (handleCenter(last) + handleCenter(last + 1)) / 2
        : grooveRight;

      painter.fillRect(QRect(left, 0, right - left, this->height()), color);

      left = right;
      value = last + 1;
    }
  }

private:
  static constexpr int Height = 6;

  QPointer<QSlider> m_Slider;
  std::vector<QColor> m_Colors;
};

QmitkSliderColorBar::QmitkSliderColorBar(QWidget* parent)
  : QWidget(parent),
    m_Bar(new Bar(this)),
    m_Caption(new QLabel(this))
{
  // Never taller than its contents, even in a layout that has space to give
  // away and no other row allowed to take it.
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  m_Caption->setAlignment(Qt::AlignCenter);
  m_Caption->setWordWrap(true);
  m_Caption->setVisible(false);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(2);
  layout->addWidget(m_Bar);
  layout->addWidget(m_Caption);
}

QmitkSliderColorBar::~QmitkSliderColorBar() = default;

void QmitkSliderColorBar::SetSlider(QSlider* slider)
{
  m_Bar->SetSlider(slider);
}

void QmitkSliderColorBar::SetColors(const std::vector<QColor>& colors)
{
  m_Bar->SetColors(colors);
}

void QmitkSliderColorBar::SetCaption(const QString& text, const QColor& color)
{
  m_Caption->setText(text);
  m_CaptionColor = color;
  this->ApplyCaptionColor();
  m_Caption->setVisible(!text.isEmpty());
}

void QmitkSliderColorBar::ApplyCaptionColor()
{
  // A color of its own would override the disabled text color of the palette.
  m_Caption->setStyleSheet(m_CaptionColor.isValid() && this->isEnabled()
    ? QString("color: %1").arg(m_CaptionColor.name())
    : QString());
}

void QmitkSliderColorBar::changeEvent(QEvent* event)
{
  QWidget::changeEvent(event);

  if (event->type() == QEvent::EnabledChange)
  {
    this->ApplyCaptionColor();
    m_Bar->update();
  }
}

QSize QmitkSliderColorBar::sizeHint() const
{
  auto hint = QWidget::sizeHint();

  // A fixed height is the hinted one, and how high a wrapping caption is
  // depends on the width it gets.
  if (this->width() > 0)
    hint.setHeight(this->heightForWidth(this->width()));

  return hint;
}

void QmitkSliderColorBar::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  if (event->oldSize().width() != event->size().width())
    this->updateGeometry();
}
