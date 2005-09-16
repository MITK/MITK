// SliderLevelWindowWidget.cpp

#include "QmitkSliderLevelWindowWidget.h"


/**
* Constructor
*/
QmitkSliderLevelWindowWidget::QmitkSliderLevelWindowWidget( QWidget * parent, const char * name, WFlags f )
: QWidget( parent, name, f ), brush( QBrush::SolidPattern ),mouseDown(false), lw()
{
  // Dense2Pattern
  // BDiagPattern
  lw.SetLevel(0);
  lw.SetWindow(200);
  lw.SetRangeMin(-2048);
  lw.SetRangeMax(2048);
  font.setPointSize( 6 );
  setMinimumSize ( QSize( 24, 50 ) );
  setMaximumSize ( QSize( 24, 2000 ) );
  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
  brush.setColor( QColor( 0, 0, 0 ) );
  moveHeight = height() - 25;
  update();
}


/**
*
*/
void QmitkSliderLevelWindowWidget::newPaintEvent()
{
  QPainter painter(this);

  painter.setFont( font );

  QColor c(93,144,169);
  QColor cl = c.light();
  QColor cd = c.dark();
  //std::cout << c.red() << "/" << cl.red() << "/" << cd.red() << std::endl; 
  painter.setBrush(c);
  painter.drawRect(rect);
  painter.setPen (cl);
  
  painter.drawLine(rect.topLeft(),rect.topRight());
  painter.drawLine(rect.topLeft(),rect.bottomLeft());

  //debug
  // painter.drawLine(rect.topLeft(),rect.bottomRight());
  
  painter.setPen (cd);
  painter.drawLine(rect.topRight(),rect.bottomRight());
  painter.drawLine(rect.bottomRight(),rect.bottomLeft());
  

  //painter.setPen ( brush.color () );
  //	painter.drawText ( 2, moveHeight + 10, level );
  //	painter.drawText ( 2, moveHeight + 20, window );
  //painter.fillRect ( rect, brush );
  //painter.setPen ( brush.color () );
  // painter.drawRect ( rect );
}

QString QmitkSliderLevelWindowWidget::GetWindow()
{
  return window;
}

QString QmitkSliderLevelWindowWidget::GetLevel()
{
  return level;
}

void QmitkSliderLevelWindowWidget::paintEvent( QPaintEvent* e ) 
{
  newPaintEvent();

  //QPainter painter(this);

  //painter.setFont( font );
  //painter.setPen ( brush.color () );
  //painter.drawText ( 2, moveHeight + 10, level );
  //painter.drawText ( 2, moveHeight + 20, window );
  //painter.fillRect ( rect, brush );
  //painter.setPen ( brush.color () );
  //painter.drawRect ( rect );
  /*
  QString text("Lewel: ");
  text += QString::number( lw.getLevel() );
  text += " / Window: ";
  text += QString::number( lw.getWindow() );
  painter.drawText ( 100, 50, text );

  text = "MaximumRange: ";
  text += QString::number( lw.getMaximumRange() );
  painter.drawText ( 100, 75, text );

  text = "Min: ";
  text += QString::number( lw.getMin() );
  painter.drawText ( 100, 100, text );

  text = "Max: ";
  text += QString::number( lw.getMax() );
  painter.drawText ( 100, 125, text );

  text = "Window: ";
  text += QString::number( lw.getMax() - lw.getMin() );
  painter.drawText ( 100, 150, text );
  */

}


/**
*
*/
void QmitkSliderLevelWindowWidget::mouseMoveEvent( QMouseEvent* mouseEvent ) {

  if ( mouseDown ) {

    float fact = (float) moveHeight / lw.GetRange();

    if ( leftbutton ) 
    {
      float maxv = lw.GetRangeMax();
      float minv = lw.GetRangeMin();
      float wh = lw.GetWindow() / 2;

      float value = (moveHeight - mouseEvent->pos().y()) / fact + minv;
      if ( value - wh < minv )
        lw.SetLevel( lw.GetRangeMin() + wh );

      else if ( value + wh > maxv )
        lw.SetLevel( lw.GetRangeMax() - wh );

      else
        lw.SetLevel( value );

    }
    else 
    {
      double diff = mouseEvent->pos().y();
      diff -= startPos.y();
      // scale down mouse move movement for more control

      if (diff == 0) return;

      if (lw.GetWindow() < 40.0 ) diff = diff /(vnl_math_abs(diff)/2.0) ;
      else if (lw.GetWindow() < 100.0 ) diff /= 5.0;

      float value = lw.GetWindow() - ( diff );

      if ( value < 1 )
        value = 1;

      lw.SetWindow( value );
    }


    update();
  }
}


/**
*
*/
void QmitkSliderLevelWindowWidget::mousePressEvent( QMouseEvent* mouseEvent ) {
  mouseDown = true;
  startPos = mouseEvent->pos();

  if ( mouseEvent->button() == QMouseEvent::LeftButton )
    leftbutton = true;
  else
    leftbutton = false;

  mouseMoveEvent( mouseEvent );
}

/**
*
*/
void QmitkSliderLevelWindowWidget::resizeEvent ( QResizeEvent * event ) {
  moveHeight = event->size().height() - 25;
  update();
}

/**
*
*/
void QmitkSliderLevelWindowWidget::mouseReleaseEvent( QMouseEvent* mouseEvent ) 
{
  mouseDown = false;
}


/**
*
*/
void QmitkSliderLevelWindowWidget::setLevelWindow( const mitk::LevelWindow& lw ) {
  this->lw = lw;
}

/**
*
*/
mitk::LevelWindow& QmitkSliderLevelWindowWidget::getLevelWindow() {
  return lw;
}

/**
*
*/
void QmitkSliderLevelWindowWidget::update( ipPicDescriptor* pic ) {

  double min, max;
  ipFuncExtr( pic, &min, &max );
  lw.SetRangeMin( min );
  lw.SetRangeMax( max );
  lw.SetLevel( (max - min) / 2 );
  lw.SetWindow( (max - min) / 2 );
  update();
}

/**
*
*/
void QmitkSliderLevelWindowWidget::update() {

  float mr = lw.GetRange();

  if ( mr < 1 )
    mr = 1;

  float fact = (float) moveHeight / mr;

  float rectHeight = lw.GetWindow() * fact;

  if ( rectHeight < 15 )
    rectHeight = 15;

  if ( lw.GetMin() < 0 )
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact) , 24, (int) rectHeight );
  else
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact), 24, (int) rectHeight );

  level = QString::number( (int) lw.GetLevel() ) + " L";
  window = QString::number( (int) lw.GetWindow() ) + " W";


  QWidget::repaint();

  // FIX: only emit the signal if the level window was changed by the user via mouse
  if (mouseDown) emit levelWindow( &lw );

}

void QmitkSliderLevelWindowWidget::updateFromLineEdit(int lineEditLevel, int lineEditWindow) 
{

  float mr = lw.GetRange();

  if ( mr < 1 )
    mr = 1;

  float fact = (float) moveHeight / mr;

  float rectHeight = lw.GetWindow() * fact;

  if ( rectHeight < 5 )
    rectHeight = 5;

  if ( lw.GetMin() < 0 )
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact) , 24, (int) rectHeight );
  else
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact), 24, (int) rectHeight );

  level = QString::number( (int) lineEditLevel ) + " L";
  window = QString::number( (int) lineEditWindow ) + " W";

  update();

  // FIX: only emit the signal if the level window was changed by the user via mouse
  if (mouseDown) emit levelWindow( &lw );
}//end of updateFromLineEdit
