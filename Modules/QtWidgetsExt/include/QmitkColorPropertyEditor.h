/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkColorPropertyEditor_h
#define QmitkColorPropertyEditor_h

#include <MitkQtWidgetsExtExports.h>
#include <QmitkColorPropertyView.h>
#include <QFrame>
#include <mitkColorProperty.h>

class QListBox;

/**
 * \brief A frameless popup widget for choosing colors from an HSV gradient.
 * \ingroup Widgets
 *
 * Displays a rectangular HSV color gradient that the user can click to select
 * a color. The widget grabs mouse and keyboard input when shown, and closes
 * on mouse release or key press. If a key is pressed, the original color is
 * restored.
 *
 * \sa QmitkColorPropertyEditor
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPopupColorChooser : public QFrame
{
  Q_OBJECT

public:
  /**
   * \brief Construct the popup color chooser.
   * \param[in] parent The parent widget.
   * \param[in] steps Number of discrete color steps in the gradient.
   * \param[in] size Width and height of the popup in pixels.
   */
  QmitkPopupColorChooser(QWidget *parent = nullptr, unsigned int steps = 16, unsigned int size = 150);

  /** \brief Destructor. */
  ~QmitkPopupColorChooser() override;

  /**
   * \brief Set the number of discrete steps for the color gradient.
   * \param[in] steps Number of gradient steps.
   */
  void setSteps(int steps);

  /**
   * \brief Show the popup at the given position.
   *
   * If a color is provided, the popup is positioned so that the matching
   * color is under the cursor.
   *
   * \param[in] parent The widget that triggered the popup.
   * \param[in] point The position in parent coordinates.
   * \param[in] color Optional initial color to center the popup on.
   */
  virtual void popup(QWidget *parent,
                     const QPoint &point,
                     const mitk::Color *color = nullptr);

signals:
  /**
   * \brief Emitted when the user moves the mouse over a color or releases the mouse.
   * \param[in] color The selected QColor.
   */
  void colorSelected(QColor color);

protected:
  void keyReleaseEvent(QKeyEvent *) override;

  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void closeEvent(QCloseEvent *) override;

  void paintEvent(QPaintEvent *) override;
  void drawGradient(QPainter *p);

private:
  QWidget *m_popupParent;
  QWidget *my_parent;

  unsigned int m_Steps;
  unsigned int m_Steps2;
  unsigned int m_HStep;
  unsigned int m_SStep;
  unsigned int m_VStep;

  int m_H;
  int m_S;
  int m_V;
  QColor m_OriginalColor;
};

/**
 * \brief Editable color swatch for a mitk::ColorProperty.
 * \ingroup Widgets
 *
 * Extends QmitkColorPropertyView with editing capability. On mouse press,
 * a QmitkPopupColorChooser appears allowing the user to pick a new color.
 * The selected color is written back to the associated mitk::ColorProperty
 * and a rendering update is requested.
 *
 * A shared static QmitkPopupColorChooser instance is used across all editors.
 *
 * \sa QmitkColorPropertyView, QmitkPopupColorChooser
 */
class MITKQTWIDGETSEXT_EXPORT QmitkColorPropertyEditor : public QmitkColorPropertyView
{
  Q_OBJECT

public:
  /**
   * \brief Construct an editor for the given color property.
   * \param[in] property The mitk::ColorProperty to edit.
   * \param[in] parent The parent widget.
   */
  QmitkColorPropertyEditor(const mitk::ColorProperty *property, QWidget *parent);

  /** \brief Destructor. Decrements the shared color chooser reference count. */
  ~QmitkColorPropertyEditor() override;

protected:
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  static QmitkPopupColorChooser *colorChooser;
  static int colorChooserRefCount;

protected slots:

  void onColorSelected(QColor);

private:
};

#endif
