/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef QMITK_COLORPROPERTYEDITOR_H_INCLUDED
#define QMITK_COLORPROPERTYEDITOR_H_INCLUDED

#include "QmitkColorPropertyView.h"
#include "QmitkExtExports.h"
#include <QFrame>
#include <mitkColorProperty.h>

class QListBox;

/// @ingroup Widgets
class QmitkExt_EXPORT QmitkPopupColorChooser : public QFrame
{
  Q_OBJECT
    
  public:

    QmitkPopupColorChooser(QWidget* parent = 0, unsigned int steps = 16, unsigned int size = 150);
    virtual ~QmitkPopupColorChooser();
    
    void setSteps(int);
    
    virtual void popup(QWidget* parent, const QPoint& point, const mitk::Color* = 0);  /// Call to popup this widget. parent determines popup position
   
  signals:
    
    void colorSelected(QColor);

  protected:

    virtual void keyReleaseEvent(QKeyEvent*);

    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void closeEvent(QCloseEvent*);
    
    virtual void paintEvent(QPaintEvent*);
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
class QmitkExt_EXPORT QmitkColorPropertyEditor : public QmitkColorPropertyView
{
  Q_OBJECT

  public:
    
    QmitkColorPropertyEditor( const mitk::ColorProperty*, QWidget* parent );
    virtual ~QmitkColorPropertyEditor();
      
  protected:

    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

    static QmitkPopupColorChooser* colorChooser;
    static int colorChooserRefCount;

  protected slots:

    void onColorSelected(QColor);

  private:

};

#endif

