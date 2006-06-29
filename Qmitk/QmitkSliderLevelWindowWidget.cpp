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
  scale = TRUE;
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
  
  
  float mr = lw.GetRange();

  if ( mr < 1 )
    mr = 1;
 
  float fact = (float) moveHeight / mr;
  
  painter.setPen(black);

  //begin draw scale
  if (scale)
  {
    painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact) , 15, moveHeight + (int)(lw.GetRangeMin()*fact));
    QString s = " 0";
    painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact + 3), s );

    int count = 1;
    for(int i = moveHeight + (int)(lw.GetRangeMin()*fact + 20*fact); i < moveHeight; i+=(int)(20*fact))
    {
      s = QString::number(-count*20);
      if (count % 5 && (((count*20*fact) - (count - 1)*20*fact) > 2.5) )
        painter.drawLine( 8, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact), 12, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact));
      else if (!(count % 5))
      {
        if (((count*20*fact) - (count - 5)*20*fact) > 7)
        {
          painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact), 15, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact));
          painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact + 3), s );
        }
        else if (!(count % 10) && (((count*20*fact) - (count - 10)*20*fact) > 7) )
        {
          painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact), 15, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact));
          painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact + 3), s );
        }
        else if (!(count % 30))
        {
          painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact), 15, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact));
          painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact + 3), s );
        }
      }
      i=moveHeight + (int)(lw.GetRangeMin()*fact + count*20*fact);
      count++;
    }

    count = 1;

    for(int i = moveHeight + (int)(lw.GetRangeMin()*fact); i >= 0; i-=(int)(20*fact))
    {
      s = QString::number(count*20);
      if(count % 5 && (((count*20*fact) - (count - 1)*20*fact) > 2.5))
        painter.drawLine( 8, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact), 12, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact));
      else if (!(count % 5))
      {
        if (((count*20*fact) - (count - 5)*20*fact) > 7)
        {
          painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact), 15, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact));
          painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact + 3), s );
        }
        else if (!(count % 10) && (((count*20*fact) - (count - 10)*20*fact) > 7))
        {
          painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact), 15, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact));
          painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact + 3), s );
        }
        else if (!(count % 30))
        {
          painter.drawLine( 5, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact), 15, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact));
          painter.drawText( 21, moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact + 3), s );
        }
      }
      i=moveHeight + (int)(lw.GetRangeMin()*fact - count*20*fact);
      count++;
    }
  }
  //end draw scale

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
}

QString QmitkSliderLevelWindowWidget::GetWindow()
{
  return QString::number( (int) lw.GetWindow() );//window;
}

QString QmitkSliderLevelWindowWidget::GetLevel()
{
  return QString::number( (int) lw.GetLevel() );//level;
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
      if ((mouseEvent->pos().y() >= (rect.bottomLeft().y() - 3) && mouseEvent->pos().y() <= (rect.bottomLeft().y() + 3)))
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

        if (diff == 0) return;
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
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact) , 17, (int) rectHeight );
  else
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact), 17, (int) rectHeight );

  //level = QString::number( (int) lw.GetLevel() );// + " L";
  //window = QString::number( (int) lw.GetWindow() );// + " W";


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
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact) , 17, (int) rectHeight );
  else
    rect.setRect( 2, (int) (moveHeight - (lw.GetMax() - lw.GetRangeMin()) * fact), 17, (int) rectHeight );

  //level = QString::number( (int) lineEditLevel );// + " L";
  //window = QString::number( (int) lineEditWindow );// + " W";

  update();

  // FIX: only emit the signal if the level window was changed by the user via mouse
  if (mouseDown) emit levelWindow( &lw );
}//end of updateFromLineEdit

void QmitkSliderLevelWindowWidget::contextMenuEvent( QContextMenuEvent * )
{  
  QPopupMenu *contextMenu = new QPopupMenu( this );
  Q_CHECK_PTR( contextMenu );
  if (scale)
    contextMenu->insertItem(tr("Hide scale"), this, SLOT(hideScale()));
  else
    contextMenu->insertItem(tr("Show scale"), this, SLOT(showScale()));
  presetSubmenu = new QPopupMenu( this );
  Q_CHECK_PTR( presetSubmenu );
  m_presetID = presetSubmenu->insertItem(tr("Preset definition"), this, SLOT(addPreset()));
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
  if (!(id == m_presetID))
  {
    double dlevel = pre.getLevel(std::string((const char*)item));
    double dwindow = pre.getWindow(std::string((const char*)item));
    if ((dlevel + dwindow/2) > lw.GetRangeMax())
    {
      double lowerBound = (dlevel - dwindow/2);
      if (!(lowerBound > lw.GetRangeMax()))
      {
        dwindow = lw.GetRangeMax() - lowerBound;
        dlevel = lowerBound + dwindow/2;
      }
      else
      {
        dlevel = lw.GetRangeMax() - 1;
        dwindow = 2;
      }
    }
    else if ((dlevel - dwindow/2) < lw.GetRangeMin())
    {
      double upperBound = (dlevel + dwindow/2);
      if (!(upperBound < lw.GetRangeMin()))
      {
        dwindow = lw.GetRangeMin() + upperBound;
        dlevel = upperBound - dwindow/2;
      }
      else
      {
        dlevel = lw.GetRangeMin() + 1;
        dwindow = 2;
      }
    }
    lw.SetLevel(dlevel);
    lw.SetWindow(dwindow);
    update();
    emit levelWindow( &lw );
  }
}

void QmitkSliderLevelWindowWidget::addPreset()
{
  QmitkLevelWindowPresetDefinition addPreset(this, "newPreset", TRUE);
  addPreset.setPresets(pre.getLevelPresets(), pre.getWindowPresets(), QString::number( (int) lw.GetLevel() ), QString::number( (int) lw.GetWindow() ));
  if(addPreset.exec())
  {
    pre.newPresets(addPreset.getLevelPresets(), addPreset.getWindowPresets());
  }
}

void QmitkSliderLevelWindowWidget::hideScale()
{
  scale = FALSE;
  repaint();
}

void QmitkSliderLevelWindowWidget::showScale()
{
  scale = TRUE;
  repaint();
}
