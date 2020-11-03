/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITK_COLORPROPERTYEDITOR_H_INCLUDED
#define QMITK_COLORPROPERTYEDITOR_H_INCLUDED

#include "MitkQtWidgetsExtExports.h"
#include "QmitkColorPropertyView.h"
#include <QFrame>
#include <mitkColorProperty.h>

class QListBox;

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkPopupColorChooser : public QFrame
{
  Q_OBJECT

public:
  QmitkPopupColorChooser(QWidget *parent = nullptr, unsigned int steps = 16, unsigned int size = 150);
  ~QmitkPopupColorChooser() override;

  void setSteps(int);

  virtual void popup(QWidget *parent,
                     const QPoint &point,
                     const mitk::Color * = nullptr); /// Call to popup this widget. parent determines popup position

signals:

  void colorSelected(QColor);

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

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkColorPropertyEditor : public QmitkColorPropertyView
{
  Q_OBJECT

public:
  QmitkColorPropertyEditor(const mitk::ColorProperty *, QWidget *parent);
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
