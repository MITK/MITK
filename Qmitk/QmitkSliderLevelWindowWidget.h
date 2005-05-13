// SliderLevelWindowWidget.h

#ifndef QSLIDERLEVELWINDOW_WIDGET
#define QSLIDERLEVELWINDOW_WIDGET

/*! @file
*	DKFZ - Heidelberg
*
*	Module Name: QSliderLevelWindowWidget
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

class QSliderLevelWindowWidget : public QWidget {

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

  bool mouseDown;
  bool leftbutton;
  int moveHeight;
  QString level;
  QString window;
  QFont font;

public:

  QSliderLevelWindowWidget( QWidget * parent=0, const char * name=0, WFlags f = false );
  QBrush brush;

  /*!
  *	data structure which stores the values manipulated
  *	by a QSliderLevelWindowWidget
  */
  mitk::LevelWindow lw;

protected:

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
