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

#include <qpainter.h>
#include <QmitkLevelWindowWidgetContextMenu.h>

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

public:

  QmitkSliderLevelWindowWidget( QWidget * parent=0, const char * name=0, WFlags f = false );

  /*!
  *	data structures which store the values manipulated
  *	by a QmitkSliderLevelWindowWidget
  */
  mitk::LevelWindow m_Lw;
  mitk::LevelWindowManager* m_Manager;

  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);
  void update( );
  void setDataTreeIteratorClone(mitk::DataTreeIteratorClone &it);

private:

  void contextMenuEvent ( QContextMenuEvent * );
  void OnPropertyModified(const itk::EventObject& e);

protected:

  /*!
  * helper for drawing the component
  */
  QRect m_Rect;

  /*!
  * helper for drawing the component
  */
  QPoint m_StartPos;

  bool m_Resize;
  bool m_Bottom;
  bool m_MouseDown;
  bool m_Leftbutton;
  bool m_CtrlPressed;
  bool m_SliderVisible;
  int m_MoveHeight;
  bool m_Scale;
  QRect m_LowerBound;
  QRect m_UpperBound;
  mitk::DataTreeIteratorClone m_It;
  unsigned long m_ObserverTag;
  
  QFont m_Font;
  QmitkLevelWindowWidgetContextMenu* m_Contextmenu;

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

  void hideScale();
  void showScale();

};

#endif //QSLIDERLEVELWINDOW_WIDGET
