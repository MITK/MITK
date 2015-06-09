/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef QMITK_COLORPROPERTYEDITOR_H_INCLUDED
#define QMITK_COLORPROPERTYEDITOR_H_INCLUDED

#include "QmitkColorPropertyView.h"
#include "MitkQtWidgetsExtExports.h"
#include <QFrame>
#include <mitkColorProperty.h>

class QListBox;

/// @ingroup Widgets
class MITKQTWIDGETSEXT_EXPORT QmitkPopupColorChooser : public QFrame
{
  Q_OBJECT

  public:

    QmitkPopupColorChooser(QWidget* parent = nullptr, unsigned int steps = 16, unsigned int size = 150);
    virtual ~QmitkPopupColorChooser();

    void setSteps(int);

    virtual void popup(QWidget* parent, const QPoint& point, const mitk::Color* = nullptr);  /// Call to popup this widget. parent determines popup position

  signals:

    void colorSelected(QColor);

  protected:

    virtual void keyReleaseEvent(QKeyEvent*) override;

    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void closeEvent(QCloseEvent*) override;

    virtual void paintEvent(QPaintEvent*) override;
    void drawGradient(QPainter* p);

  private:

    QWidget* m_popupParent;
    QWidget* my_parent;

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

    QmitkColorPropertyEditor( const mitk::ColorProperty*, QWidget* parent );
    virtual ~QmitkColorPropertyEditor();

  protected:

    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;

    static QmitkPopupColorChooser* colorChooser;
    static int colorChooserRefCount;

  protected slots:

    void onColorSelected(QColor);

  private:

};

#endif

