/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#ifndef QSLIDERLEVELWINDOW_WIDGET
#define QSLIDERLEVELWINDOW_WIDGET

#include <qwidget.h> 
#include <qpainter.h>
#include <ipFunc/ipFunc.h>
#include <mitkLevelWindow.h>
#include <qstring.h>
#include <mitkLevelWindowPreset.h>
#include <qpopupmenu.h>

#include "mitkDataTreeNode.h"
#include "mitkDataTree.h"

/**

  This documentation actually refers to the QmitkLevelWindowWidget and is only put in this class due to technical issues (should be moved later).
  
  The QmitkLevelWindowWidget is a kind of container for a
  QmitkSliderLevelWindowWidget (this is the cyan bar above the text input
  fields) and two text input fields. It holds a reference to a
  mitk::LevelWindow variable, which is manipulated by the text inputs and the
  bar to adjust brightness/contrast of a single image.

  Which image is changed is determined by the QmitkStdMultiWidget or another
  widget that contains the level window widget. In case of the
  QmitkStdMultiWidget the level window widget should always refer to the
  top-most image in the data tree (top-most is determined by the "layer"
  property).

  The internal mitk::LevelWindow variable contains a range that is valid for
  a given image. It should not be possible to move the level/window
  parameters outside this range.

  Now for the behaviour of the text inputs: The upper one contains the
  value of the level (brightness), the lower one shows the window (contrast).
  In a cleaned up version of the widget, I would like to show the upper and
  lower limit of the window.

  The behaviour of the cyan bar is more obvious: the height of the gray
  background stands for the valid range. The cyan bar in front displays the
  currently selected level/window setting. You can change the level by
  dragging the bar with the left mouse button. The window is changed by
  dragging with the right mouse button.

  */
class QmitkSliderLevelWindowWidget : public QWidget {

  Q_OBJECT

protected:



  /*!
  * helper for drawing the component
  */
  QRect rect;

  /*!
  * helper for drawing the component
  */
  QPoint startPos;

  bool resize;
  bool bottom;
  float factor;
  bool mouseDown;
  bool leftbutton;
  bool ctrlPressed;
  int moveHeight;
  int m_presetID;
  int m_defaultID;
  bool scale;
  QRect m_LowerBound;
  QRect m_UpperBound;
  mitk::ScalarType m_upperLimit;
  mitk::ScalarType m_lowerLimit;
  
  QFont font;
  QPopupMenu *presetSubmenu;
  

public:

  QmitkSliderLevelWindowWidget( QWidget * parent=0, const char * name=0, WFlags f = false );
  QBrush brush;

  /*!
  *	data structures which store the values manipulated
  *	by a QmitkSliderLevelWindowWidget
  */
  mitk::LevelWindow lw;
  mitk::LevelWindowPreset pre;

protected:

  bool getPresets();

  /*! 
  * repaint the slider
  */
  void paintEvent( QPaintEvent* e );

  /*!
  * method implements the component behaviour
  */
  void mouseMoveEvent( QMouseEvent* mouseEvent );

  /*!
  *		???
  */
  void mousePressEvent( QMouseEvent* mouseEvent );

  /*!
  *		???
  */
  void mouseReleaseEvent( QMouseEvent* mouseEvent );

  /*!
  *		???
  */
  void virtual resizeEvent ( QResizeEvent * event );

protected slots:

void setPreset(int id);
void addPreset();
void setDefaultLevelWindow();
void setDefaultScaleRange();
void changeScaleRange();
void hideScale();
void showScale();

private:
   void contextMenuEvent ( QContextMenuEvent * );
   

public:

  void setLevelWindow( const mitk::LevelWindow& levelWindow );
  mitk::LevelWindow& getLevelWindow();
  void update( ipPicDescriptor* pic );
  void update( );
  void updateFromLineEdit();
  QString GetWindow();
  QString GetLevel();

signals:

  void levelWindow(mitk::LevelWindow* lw );
  void newRange(mitk::LevelWindow* lw);

};

#endif //QSLIDERLEVELWINDOW_WIDGET
