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

#ifndef QMITKSLIDERLEVELWINDOW_WIDGET
#define QMITKSLIDERLEVELWINDOW_WIDGET

#include <MitkQtWidgetsExports.h>

#include <QWidget>
#include <mitkLevelWindowManager.h>

class QmitkLevelWindowWidgetContextMenu;

/**
 * \ingroup QmitkModule
 *
 * \brief Provides a widget with a slider to change the level and
 * window value of the current image.
 *
 * This documentation actually refers to the QmitkLevelWindowWidget
 * and is only put in this class due to technical issues (should be
 * moved later).
 *
 * The QmitkLevelWindowWidget is a kind of container for a
 * QmitkSliderLevelWindowWidget (this is the cyan bar above the text
 * input fields) and a QmitkLineEditLevelWindowWidget (with two text
 * input fields). It holds a reference to a mitk::LevelWindowManager
 * variable, which keeps the LevelWindowProperty of the currently
 * selected image. Level/Window is manipulated by the text inputs and
 * the Slider to adjust brightness/contrast of a single image. All
 * changes on the slider or in the text input fields affect the current
 * image by giving new values to LevelWindowManager. LevelWindowManager
 * then sends a signal to tell other listeners about changes.
 *
 * Which image is changed is determined by mitkLevelWindowManager. If
 * m_AutoTopMost is true, always the topmost image in data tree (layer
 * property) is affected by changes. The image which is affected by
 * changes can also be changed by QmitkLevelWindowWidgetContextMenu,
 * the context menu for QmitkSliderLevelWindowWidget and
 * QmitkLineEditLevelWindowWidget. There you have the possibility to
 * set a certain image or always the topmost image in the data tree
 * (layer property) to be affected by changes.
 *
 * The internal mitk::LevelWindow variable contains a range that is
 * valid for a given image. It should not be possible to move the
 * level/window parameters outside this range. The range can be changed
 * and reset to its default values by QmitkLevelWindowWidgetContextMenu,
 * the context menu for QmitkSliderLevelWindowWidget and
 * QmitkLineEditLevelWindowWidget.
 *
 * Now for the behaviour of the text inputs: The upper one contains the
 * value of the level (brightness), the lower one shows the window (contrast).
 *
 * The behaviour of the cyan bar is more obvious: the scale in the
 * background shows the valid range. The cyan bar in front displays the
 * currently selected level/window setting. You can change the level by
 * dragging the bar with the left mouse button or clicking somewhere inside
 * the scalerange with the left mouse button. The window is changed by
 * moving the mouse on the upper or lower bound of the bar until the cursor
 * becomes an vertical double-arrowed symbol. Then you can change the
 * windowsize by clicking the left mouse button and move the mouse upwards
 * or downwards. The bar becomes greater upwards as well as downwards. If
 * you want to change the size of the window in only one direction you
 * have to press the CTRL-key while doing the same as mentioned above.
 * This information is also presented by a tooltip text when moving the
 * mouse on the upper or lower bound of the bar.
 */
class MITKQTWIDGETS_EXPORT QmitkSliderLevelWindowWidget : public QWidget {

  Q_OBJECT

public:

  /// constructor
  QmitkSliderLevelWindowWidget( QWidget * parent=0, Qt::WindowFlags f = 0 );

  /// destructor
  ~QmitkSliderLevelWindowWidget();

  /*!
  *  data structure which stores the values manipulated
  *  by a QmitkSliderLevelWindowWidget
  */
  mitk::LevelWindow m_LevelWindow;

  /// manager who is responsible to collect and deliver changes on Level/Window
  mitk::LevelWindowManager::Pointer m_Manager;

  /// sets the manager who is responsible to collect and deliver changes on Level/Window
  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);

  /// sets the DataStorage which holds all image-nodes
  void setDataStorage(mitk::DataStorage* ds);

  /// returns the manager who is responsible to collect and deliver changes on Level/Window
  mitk::LevelWindowManager* GetManager();

private:

  /// creates the contextmenu for this widget from class QmitkLevelWindowWidgetContextMenu
  void contextMenuEvent ( QContextMenuEvent * );

  /// change notifications from the mitkLevelWindowManager
  void OnPropertyModified(const itk::EventObject& e);

protected:
  /// recalculate the size and position of the slider bar
  virtual void update( );

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
  int m_MoveHeight;
  bool m_ScaleVisible;
  QRect m_LowerBound;
  QRect m_UpperBound;
  unsigned long m_ObserverTag;
  bool m_IsObserverTagSet;

  QFont m_Font;

  /*!
  *  data structure which creates the contextmenu for QmitkLineEditLevelWindowWidget
  */
  QmitkLevelWindowWidgetContextMenu* m_Contextmenu;

  /*!
  * repaint the slider and the scale
  */
  void paintEvent( QPaintEvent* e );

  /*!
  * method implements the component behaviour
  *
  * checks if cursor is on upper or lower bound of slider bar and changes cursor symbol
  *
  * checks if left mouse button is pressed and if CTRL is pressed and changes sliderbar in movedirection accordingly
  */
  void mouseMoveEvent( QMouseEvent* mouseEvent );

  void enterEvent ( QEvent * event  );

  /*!
  *  registers events when a mousebutton is pressed
  *
  * if leftbutton is pressed m_Leftbutton is set to true
  *
  * also checks if CTRL is pressed and sets the bool variable m_CtrlPressed
  */
  void mousePressEvent( QMouseEvent* mouseEvent );

  /*!
  *  sets the variable m_MouseDown to false
  */
  void mouseReleaseEvent( QMouseEvent* mouseEvent );

  /*!
  * causes an update of the sliderbar when resizing the window
  */
  void virtual resizeEvent ( QResizeEvent * event );

protected slots:

  /// hides the scale if "Hide Scale" is selected in contextmenu
  void hideScale();

  /// shows the scale if "Show Scale" is selected in contextmenu
  void showScale();
};
#endif //QMITKSLIDERLEVELWINDOW_WIDGET
