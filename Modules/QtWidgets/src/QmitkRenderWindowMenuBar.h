/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowMenuBar_h
#define QmitkRenderWindowMenuBar_h

#include <QWidget>

#include <vector>

class QGraphicsOpacityEffect;
class QMenu;
class QToolButton;
class QVariantAnimation;

/**
 * \brief A row of render window menu buttons docked to a top corner of its parent.
 *
 * The bar is flush with both edges of its corner and rounds the corner that
 * faces the image. It rests at a reduced scale and opacity until the mouse
 * cursor gets close and then animates to its full appearance, independently
 * of any other bar on the same parent.
 *
 * Its background is the palette color of its background role, so style
 * sheets restyle it with background-color. The corner radius is available
 * to style sheets as qproperty-cornerRadius, in logical pixels at the
 * default scale.
 */
class QmitkRenderWindowMenuBar : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(int cornerRadius READ GetCornerRadius WRITE SetCornerRadius)

public:
  enum class Corner
  {
    TopLeft,
    TopRight
  };

  QmitkRenderWindowMenuBar(Corner corner, QWidget* parent);
  ~QmitkRenderWindowMenuBar() override;

  /**
   * \brief Appends a button whose size and position the bar controls.
   *
   * Hiding or showing the button later on re-arranges the bar.
   */
  QToolButton* AddButton(const QIcon& icon);

  /**
   * \brief Appends a button that opens \p menu below itself when clicked.
   *
   * The bar keeps its full appearance while the menu is open.
   */
  QToolButton* AddMenuButton(const QIcon& icon, QMenu* menu);

  /** \brief Sets the scale of the full appearance, 1.0 being the default size. */
  void SetScale(double scale);

  /**
   * \brief Sets the scale and opacity the bar rests at, relative to its full appearance.
   *
   * 1.0 for both makes the bar show its full appearance right away.
   */
  void SetRestingAppearance(double scale, double opacity);

  bool HasShownButtons() const;

  /** \brief The size of the full appearance, whatever the bar currently shows. */
  QSize GetFullSize() const;

  /** \brief Shows the bar at its resting appearance, unless it is shown already. */
  void Reveal();

  void Conceal();

  /** \brief Moves the bar into its corner, e.g. after the parent was resized. */
  void Dock();

  /**
   * \brief Animates the bar to its full appearance if \p cursor is close, and back otherwise.
   *
   * \param[in] cursor The mouse cursor position in the coordinates of the parent.
   */
  void UpdateProximity(const QPoint& cursor);

  int GetCornerRadius() const;
  void SetCornerRadius(int radius);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  void KeepFullWhileOpen(QMenu* menu);
  bool ChangesAppearanceAtRest() const;
  double GetCurrentScale() const;
  QSize ComputeSize(double scale) const;
  void SetEngaged(bool engaged);
  void SetEmphasis(double emphasis);
  void ApplyAppearance();

  Corner m_Corner;
  std::vector<QToolButton*> m_Buttons;

  double m_Scale;
  double m_RestingScale;
  double m_RestingOpacity;
  int m_CornerRadius;

  /** 0.0 at the resting appearance, 1.0 at the full one. */
  double m_Emphasis;
  bool m_Engaged;
  bool m_MenuOpen;

  QVariantAnimation* m_Animation;
  QGraphicsOpacityEffect* m_OpacityEffect;
};

#endif
