// SliderLevelWindowWidget.cpp

#include "QmitkSliderLevelWindowWidget.h"
#include <qcursor.h>
#include <qpopupmenu.h>
#include <mitkConfig.h>
#include <itksys/SystemTools.hxx>
#include <mitkLevelWindowPreset.h>
#include <qdialog.h>
#include <qlayout.h>
#include <QmitkLevelWindowPresetDefinition.h>
#include <qlineedit.h>

/**
* Constructor
*/
QmitkSliderLevelWindowWidget::QmitkSliderLevelWindowWidget( QWidget * parent, const char * name, WFlags f )
: QWidget( parent, name, f ), mouseDown(false), brush( QBrush::SolidPattern ), lw(), pre()
{
  // Dense2Pattern
  // BDiagPattern
  pre.LoadPreset();
  setMouseTracking(TRUE);
  resize = FALSE;
  bottom = FALSE;
  lw.SetLevel(0);
  lw.SetWindow(200);
  lw.SetRangeMin(-2048);
  lw.SetRangeMax(2048);
  font.setPointSize( 6 );
  setMinimumSize ( QSize( 50, 50 ) );
  setMaximumSize ( QSize( 50, 2000 ) );
  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
  brush.setColor( QColor( 0, 0, 0 ) );
  moveHeight = height() - 25;
  factor = 0;
  update();
}


/**
*
*/
void QmitkSliderLevelWindowWidget::newPaintEvent()
{
  setUpdatesEnabled(FALSE);
  QPainter painter(this);

  painter.setFont( font );

  QColor c(93,144,169);
  QColor cl = c.light();
  QColor cd = c.dark();
  //std::cout << c.red() << "/" << cl.red() << "/" << cd.red() << std::endl; 
  painter.setBrush(c);
  painter.drawRect(rect);
  
  
  float mr = lw.GetRange();

  if ( mr < 1 )
    mr = 1;

  float fact = (float) moveHeight / mr;
  
  painter.setPen(black);
  painter.drawLine( 9, moveHeight + lw.GetRangeMin()*fact , 19, moveHeight + lw.GetRangeMin()*fact);

  //painter.drawLine( 9, moveHeight/2, 19, moveHeight/2);
  QString s = " 0";
  painter.drawText( 21, moveHeight + lw.GetRangeMin()*fact + 3, s );

  int count = 1;
  for(int i = moveHeight + lw.GetRangeMin()*fact + 20*fact; i < moveHeight; i+=20*fact)
  {
    s = QString::number(-count*20);
    if (count % 5)
      painter.drawLine( 12, moveHeight + lw.GetRangeMin()*fact + count*20*fact, 16, moveHeight + lw.GetRangeMin()*fact + count*20*fact);
    else 
    {
      painter.drawLine( 9, moveHeight + lw.GetRangeMin()*fact + count*20*fact, 19, moveHeight + lw.GetRangeMin()*fact + count*20*fact);
      painter.drawText( 21, moveHeight + lw.GetRangeMin()*fact + count*20*fact + 3, s );
    }
    i=moveHeight + lw.GetRangeMin()*fact + count*20*fact;
    count++;
  }

  count = 1;

  for(int i = moveHeight + lw.GetRangeMin()*fact; i >= 0; i-=20*fact)
  {
    s = QString::number(count*20);
    if(count % 5)
      painter.drawLine( 12, moveHeight + lw.GetRangeMin()*fact - count*20*fact, 16, moveHeight + lw.GetRangeMin()*fact - count*20*fact);
    else
    {
      painter.drawLine( 9, moveHeight + lw.GetRangeMin()*fact - count*20*fact, 19, moveHeight + lw.GetRangeMin()*fact - count*20*fact);
      painter.drawText( 21, moveHeight + lw.GetRangeMin()*fact - count*20*fact + 3, s );
    }
    i=moveHeight + lw.GetRangeMin()*fact - count*20*fact;
    count++;
  }

  /*int count = 1;
  for(int i = moveHeight/2 + 20*fact; i < moveHeight; i+=20*fact)
  {
    s = QString::number(-count*20);
    if (count % 5)
      painter.drawLine( 12, moveHeight / 2 + count*20*fact, 16, moveHeight / 2 + count*20*fact);
    else 
    {
      painter.drawLine( 9, moveHeight / 2 + count*20*fact, 19, moveHeight / 2 + count*20*fact);
      painter.drawText( 21, moveHeight / 2 + count*20*fact+20*fact, s );
    }
    i=moveHeight/2 + count*20*fact;
    count++;
  }

  count = 1;

  for(int i = moveHeight/2; i >= 0; i-=20*fact)
  {
    s = QString::number(count*20);
    if(count % 5)
      painter.drawLine( 12, moveHeight/2 - count*20*fact, 16, moveHeight/2 - count*20*fact);
    else
    {
      painter.drawLine( 9, moveHeight/2 - count*20*fact, 19, moveHeight/2 - count*20*fact);
      painter.drawText( 21, moveHeight / 2 - count*20*fact+20*fact, s );
    }
    i=moveHeight/2 - count*20*fact;
    count++;
  }*/


  painter.setPen (cl);
  painter.drawLine(rect.topLeft(),rect.topRight());
  painter.drawLine(rect.topLeft(),rect.bottomLeft());

  //debug
  // painter.drawLine(rect.topLeft(),rect.bottomRight());
  
  painter.setPen (cd);
  painter.drawLine(rect.topRight(),rect.bottomRight());
  painter.drawLine(rect.bottomRight(),rect.bottomLeft());
  painter.end();
  

  //painter.setPen ( brush.color () );
  //	painter.drawText ( 2, moveHeight + 10, level );
  //	painter.drawText ( 2, moveHeight + 20, window );
  //painter.fillRect ( rect, brush );
  //painter.setPen ( brush.color () );
  // painter.drawRect ( rect );

  setUpdatesEnabled(TRUE);
}

QString QmitkSliderLevelWindowWidget::GetWindow()
{
  return window;
}

QString QmitkSliderLevelWindowWidget::GetLevel()
{
  return level;
}

void QmitkSliderLevelWindowWidget::paintEvent( QPaintEvent* itkNotUsed(e) ) 
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
  if (!mouseDown)
  {
    if (((mouseEvent->pos().y() >= (rect.topLeft().y() - 3) && mouseEvent->pos().y() <= (rect.topLeft().y() + 3))
      || (mouseEvent->pos().y() >= (rect.bottomLeft().y() - 3) && mouseEvent->pos().y() <= (rect.bottomLeft().y() + 3)))
      && mouseEvent->pos().x() >= rect.topLeft().x() && mouseEvent->pos().x() <= rect.topRight().x())
    {
      setCursor(SizeVerCursor);
      resize = TRUE;
      if (mouseEvent->pos().y() == rect.bottomLeft().y())
        bottom = TRUE;
    }
    else
    {
      setCursor(ArrowCursor);
      resize = FALSE;
      bottom = FALSE;
    }
  }

  if ( mouseDown ) {

    float fact = (float) moveHeight / lw.GetRange();

    if ( leftbutton ) 
    {
      if (resize)
      {
        double diff = (mouseEvent->pos().y()) / fact;
        diff -= (startPos.y()) / fact;
        startPos = mouseEvent->pos();
        // scale down mouse move movement for more control

        if (diff == 0) return;

        //if (lw.GetWindow() < 40.0 ) diff = diff /(vnl_math_abs(diff)/2.0) ;
        //else if (lw.GetWindow() < 100.0 ) diff /= 5.0;
        float value;
        if (bottom)
          value = lw.GetWindow() + ( ( 2 * diff ) );
        else
          value = lw.GetWindow() - ( ( 2 * diff ) );

        if ( value < 1 )
          value = 1;

        lw.SetWindow( value );
      } 
      else
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
    }
    /*else 
    {
      double diff = mouseEvent->pos().y();
      diff -= startPos.y();
      startPos = mouseEvent->pos();
      // scale down mouse move movement for more control

      if (diff == 0) return;

      //if (lw.GetWindow() < 40.0 ) diff = diff /(vnl_math_abs(diff)/2.0) ;
      //else if (lw.GetWindow() < 100.0 ) diff /= 5.0;

      float value = lw.GetWindow() - ( ( diff ) * 5 );

      if ( value < 1 )
        value = 1;

      lw.SetWindow( value );
    }*/
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
void QmitkSliderLevelWindowWidget::mouseReleaseEvent( QMouseEvent* /* mouseEvent  */) 
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

  level = QString::number( (int) lw.GetLevel() );// + " L";
  window = QString::number( (int) lw.GetWindow() );// + " W";


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

  level = QString::number( (int) lineEditLevel );// + " L";
  window = QString::number( (int) lineEditWindow );// + " W";

  update();

  // FIX: only emit the signal if the level window was changed by the user via mouse
  if (mouseDown) emit levelWindow( &lw );
}//end of updateFromLineEdit

void QmitkSliderLevelWindowWidget::contextMenuEvent( QContextMenuEvent * )
{
  
  //  std::cout << (*iter).first << " is " << (*iter).second << endl;
  
  QPopupMenu *contextMenu = new QPopupMenu( this );
  Q_CHECK_PTR( contextMenu );
  /*QPopupMenu *imageSubmenu = new QPopupMenu( this );
  Q_CHECK_PTR( imageSubmenu );
  imageSubmenu->insertItem( "Bild1", this, SLOT(changeImage()) );
  imageSubmenu->insertItem( "Bild2", this, SLOT(changeImage()) );
  imageSubmenu->insertItem( "Bild3", this, SLOT(changeImage()) );
  contextMenu->insertItem( "Image",  imageSubmenu );*/

  /*QPopupMenu *lutSubmenu = new QPopupMenu( this );
  Q_CHECK_PTR( lutSubmenu );
  lutSubmenu->insertItem( "Lut1", this, SLOT(changeImage()) );
  lutSubmenu->insertItem( "Lut2", this, SLOT(changeImage()) );
  lutSubmenu->insertItem( "Lut3", this, SLOT(changeImage()) );
  contextMenu->insertItem( "LUT",  lutSubmenu );*/
  
  presetSubmenu = new QPopupMenu( this );
  Q_CHECK_PTR( presetSubmenu );
  presetSubmenu->insertItem("New Preset", this, SLOT(addPreset()));
  presetSubmenu->insertSeparator();
  std::map<std::string, double> pres = pre.getLevelPresets();
  for( std::map<std::string, double>::iterator iter = pres.begin(); iter != pres.end(); iter++ ) {
    QString item = ((*iter).first.c_str());
    presetSubmenu->insertItem(item);
  }
  connect(presetSubmenu, SIGNAL(activated(int)), this, SLOT(setPreset(int)));
  contextMenu->insertItem( "Presets",  presetSubmenu );

    
  contextMenu->exec( QCursor::pos() );
  delete contextMenu;
  
}

void QmitkSliderLevelWindowWidget::changeImage()
{

}

void QmitkSliderLevelWindowWidget::setPreset(int id)
{
  QString item = presetSubmenu->text(id);
  if (!(item == "New Preset"))
  {
    double level = pre.getLevel(std::string((const char*)item));
    double window = pre.getWindow(std::string((const char*)item));
    if ((level + window/2) > lw.GetRangeMax())
    {
      double lowerBound = (level - window/2);
      if (!(lowerBound > lw.GetRangeMax()))
      {
        window = lw.GetRangeMax() - lowerBound;
        level = lowerBound + window/2;
      }
      else
      {
        level = lw.GetRangeMax() - 1;
        window = 2;
      }
    }
    else if ((level - window/2) < lw.GetRangeMin())
    {
      double upperBound = (level + window/2);
      if (!(upperBound < lw.GetRangeMin()))
      {
        window = lw.GetRangeMin() + upperBound;
        level = upperBound - window/2;
      }
      else
      {
        level = lw.GetRangeMin() + 1;
        window = 2;
      }
    }
    lw.SetLevel(level);
    lw.SetWindow(window);
    update();
    emit levelWindow( &lw );
  }
}

void QmitkSliderLevelWindowWidget::addPreset()
{
  QmitkLevelWindowPresetDefinition addPreset(this, "newPreset", TRUE);//, level, window);
  addPreset.setPresets(pre.getLevelPresets(), pre.getWindowPresets(), level, window);
  if(addPreset.exec())
  {
    pre.newPresets(addPreset.getLevelPresets(), addPreset.getWindowPresets());
  }
  
}
