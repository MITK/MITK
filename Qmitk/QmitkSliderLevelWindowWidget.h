// SliderLevelWindowWidget.h

#ifndef QSLIDERLEVELWINDOW_WIDGET
#define QSLIDERLEVELWINDOW_WIDGET

/*! @file
*	DKFZ - Heidelberg
*
*	Module Name: QmitkSliderLevelWindowWidget
* 
*	Module Klasse: 	ADT --- nee, GUI Komponente
*
*	Beschreibung: should be a widget for adjusting level/window 
*									values with one slider
*					
*					
*
*	Exportierte Klassen und Funktionen:
*									
*
*	Importierte Klassen und Funktionen: mitk::LevelWindow
*					
*
*	@warning		Dieses Modul wurde noch nicht abgenommen!  - ist klar...
*
*	@version		0.1
*
*  @date			
*
*  @author			Marcus Vetter
*
* changes by Thomas B. :
*
*			exchanged old LevelWindow for mitk::LevelWindow
*
*
*/

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
  int moveHeight;
  int m_presetID;
  
  QFont font;
  QPopupMenu *presetSubmenu;
  

public:

  QmitkSliderLevelWindowWidget( QWidget * parent=0, const char * name=0, WFlags f = false );
  QBrush brush;

  /*!
  *	data structure which stores the values manipulated
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

  /*! 
  * method triggers repainting of the component and emits a signal
  */
  //void update();
  void newPaintEvent();

protected slots:

void changeImage();
void setPreset(int id);
void addPreset();

private:
   void contextMenuEvent ( QContextMenuEvent * );
   

public:

  void setLevelWindow( const mitk::LevelWindow& levelWindow );
  mitk::LevelWindow& getLevelWindow();
  void update( ipPicDescriptor* pic );
  void update( );
  void updateFromLineEdit(int lineEditLevel, int lineEditWindow);
  QString GetWindow();
  QString GetLevel();

signals:

  void levelWindow(mitk::LevelWindow* lw );

};

#endif //QSLIDERLEVELWINDOW_WIDGET
