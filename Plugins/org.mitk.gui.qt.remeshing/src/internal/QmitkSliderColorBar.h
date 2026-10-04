/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSliderColorBar_h
#define QmitkSliderColorBar_h

#include <QColor>
#include <QWidget>

#include <vector>

class QLabel;
class QSlider;

/** \brief A thin bar showing a color for every value of a horizontal slider,
 * with an optional caption below it.
 *
 * The bar spans the slider's groove, and each color sits right below where
 * the handle of the slider is for that value. Place it in the same layout
 * column as the slider so that it covers the groove.
 */
class QmitkSliderColorBar : public QWidget
{
public:
  explicit QmitkSliderColorBar(QWidget* parent = nullptr);
  ~QmitkSliderColorBar() override;

  void SetSlider(QSlider* slider);

  /** \brief Set one color for each value of the slider, from its minimum to its maximum.
   *
   * Anything else, an empty list included, leaves the bar empty.
   */
  void SetColors(const std::vector<QColor>& colors);

  /** \brief Show a centered caption below the bar, or none if the text is empty.
   *
   * \param[in] text The caption.
   * \param[in] color The text color, or an invalid color for the usual one.
   */
  void SetCaption(const QString& text, const QColor& color = QColor());

  QSize sizeHint() const override;

protected:
  void changeEvent(QEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  class Bar;

  void ApplyCaptionColor();

  Bar* m_Bar;
  QLabel* m_Caption;
  QColor m_CaptionColor;
};

#endif
