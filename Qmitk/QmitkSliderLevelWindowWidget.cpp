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

#include "QmitkSliderLevelWindowWidget.h"
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <mitkConfig.h>
#include <itksys/SystemTools.hxx>
#include <mitkLevelWindowPreset.h>
#include <qdialog.h>
#include <qlayout.h>
#include <QmitkLevelWindowPresetDefinition.h>
#include <QmitkLevelWindowRangeChange.h>
#include <qlineedit.h>
#include <qtooltip.h>
#include <qpixmap.h>

/**
* Constructor
*/
QmitkSliderLevelWindowWidget::QmitkSliderLevelWindowWidget( QWidget * parent, const char * name, WFlags f )
: QWidget( parent, name, f ), mouseDown(false), brush( QBrush::SolidPattern ), lw(), pre()
{
  pre.LoadPreset();
  setMouseTracking(TRUE);
  resize = FALSE;
  bottom = FALSE;
  ctrlPressed = FALSE;
  lw.SetLevelWindow(0, 200);
  lw.SetDefaultLevelWindow(0, 200);
  m_lowerLimit = -2048;
  lw.SetRangeMinMax(-2048, 2048);
  lw.SetDefaultRangeMinMax(-2048, 2048);
  m_upperLimit = 2048;
  
  //refresh validators for line edits
  emit newRange( &lw);
  
  font.setPointSize( 6 );
  setMinimumSize ( QSize( 50, 50 ) );
  setMaximumSize ( QSize( 50, 2000 ) );
  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
  brush.setColor( QColor( 0, 0, 0 ) );
  moveHeight = height() - 25;
  factor = 0;
  scale = TRUE;

  setBackgroundMode( Qt::NoBackground );

  update();
}

void QmitkSliderLevelWindowWidget::paintEvent( QPaintEvent* itkNotUsed(e) ) 
{
  QPixmap pm(width(), height());
  pm.fill(backgroundColor());
  QPainter painter(&pm);

  painter.setFont( font );

  QColor c(93,144,169);
  QColor cl = c.light();
  QColor cd = c.dark();

  painter.setBrush(c);
  painter.drawRect(rect);
  
  // test if slider range has changed, if true -> refresh validator for line edits
  if (!(m_lowerLimit == lw.GetRangeMin() && m_upperLimit == lw.GetRangeMax()))
  {
    m_lowerLimit = lw.GetRangeMin();
    m_upperLimit = lw.GetRangeMax();
    emit newRange(&lw);
  } // end test if slider range has changed

  float mr = lw.GetRange();

  if ( mr < 1 )
    mr = 1;
 
  float fact = (float) moveHeight / mr;
  
  painter.setPen(black);

  //begin draw scale
  if (scale)
  {
    int minRange = (int)lw.GetRangeMin();
    int maxRange = (int)lw.GetRangeMax();
    int yValue = moveHeight + (int)(minRange*fact);
    painter.drawLine( 5, yValue , 15, yValue);
    QString s = " 0";
    painter.drawText( 21, yValue + 3, s );

    int count = 1;
    int k = 5;
    bool enoughSpace = false;
    bool enoughSpace2 = false;

    for(int i = moveHeight + (int)(minRange*fact); i < moveHeight;)
    {
      if (-count*20 < minRange)
        break;
      yValue = moveHeight + (int)((minRange + count*20)*fact);
      s = QString::number(-count*20);
      if (count % k && ((20*fact) > 2.5))
      {
        painter.drawLine( 8, yValue, 12, yValue);
        enoughSpace = true;
      }
      else if (!(count % k))
      {
        if ((k*20*fact) > 7)
        {
          painter.drawLine( 5, yValue, 15, yValue);
          painter.drawText( 21, yValue + 3, s );
          enoughSpace2 = true;
        }
        else
        {
          k += 5;
        }
      }
      if (enoughSpace)
      {
        i=yValue;
        count++;
      }
      else if (enoughSpace2)
      {
        i=yValue;
        count += k;
      }
      else
      {
        i=yValue;
        count = k;
      }
    }

    count = 1;
    k = 5;
    enoughSpace = false;
    enoughSpace2 = false;

    for(int i = moveHeight + minRange*fact; i >= 0;)
    {
      if (count*20 > maxRange)
        break;
      yValue = moveHeight + (int)((minRange - count*20)*fact);
      s = QString::number(count*20);
      if(count % k && ((20*fact) > 2.5))
      {
        if (!(minRange > 0 && (count*20) < minRange))
          painter.drawLine( 8, yValue, 12, yValue);
        enoughSpace = true;
      }
      else if (!(count % k))
      {
        if ((k*20*fact) > 7)
        {
          if (!(minRange > 0 && (count*20) < minRange))
          {
            painter.drawLine( 5, yValue, 15, yValue);
            painter.drawText( 21, yValue + 3, s );
          }
          enoughSpace2 = true;
        }
        else
        {
          k += 5;
        }
      }
      if (enoughSpace)
      {
        i=yValue;
        count++;
      }
      else if (enoughSpace2)
      {
        i=yValue;
        count += k;
      }
      else
      {
        i=yValue;
        count = k;
      }
    }
  }
  //end draw scale

  painter.setPen (cl);
  painter.drawLine(rect.topLeft(),rect.topRight());
  painter.drawLine(rect.topLeft(),rect.bottomLeft());
  
  painter.setPen (cd);
  painter.drawLine(rect.topRight(),rect.bottomRight());
  painter.drawLine(rect.bottomRight(),rect.bottomLeft());
  painter.end();
  QPainter p (this);
  p.drawPixmap(0, 0, pm);
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
      QToolTip::remove(this, m_UpperBound);
      QToolTip::remove(this, m_LowerBound);
      m_LowerBound.setRect(rect.bottomLeft().x(), rect.bottomLeft().y() - 3, 17, 7);
      QToolTip::add(this, m_LowerBound, "Ctrl + left click to change only lower bound");
      m_UpperBound.setRect(rect.topLeft().x(), rect.topLeft().y() - 3, 17, 7);
      QToolTip::add(this, m_UpperBound, "Ctrl + left click to change only upper bound");
      resize = TRUE;
      if ((mouseEvent->pos().y() >= (rect.bottomLeft().y() - 3) && mouseEvent->pos().y() <= (rect.bottomLeft().y() + 3)))
        bottom = TRUE;
    }
    else
    {
      QToolTip::remove(this, m_LowerBound);
      QToolTip::remove(this, m_UpperBound);
      setCursor(ArrowCursor);
      resize = FALSE;
      bottom = FALSE;
    }
  }

  if ( mouseDown ) {

    float fact = (float) moveHeight / lw.GetRange();

    if ( leftbutton ) 
    {
      if (resize && !ctrlPressed)
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
      else if(resize && ctrlPressed)
      {
        if (!bottom)
        {
          double diff = (mouseEvent->pos().y()) / fact;
          diff -= (startPos.y()) / fact;
          startPos = mouseEvent->pos();

          if (diff == 0) return;
          float value;
          
          value = lw.GetWindow() - ( ( diff ) );

          if ( value < 1 )
            value = 1;
          float oldWindow;
          float oldLevel;
          float newLevel;
          oldWindow = lw.GetWindow();
          oldLevel = lw.GetLevel();
          newLevel = oldLevel + (value - oldWindow)/2;
          if (!((newLevel + value/2) > lw.GetRangeMax())) 
            lw.SetLevelWindow( newLevel, value );
        }
        else
        {
          double diff = (mouseEvent->pos().y()) / fact;
          diff -= (startPos.y()) / fact;
          startPos = mouseEvent->pos();

          if (diff == 0) return;
          float value;
          
          value = lw.GetWindow() + ( ( diff ) );

          if ( value < 1 )
            value = 1;
          float oldWindow;
          float oldLevel;
          float newLevel;
          oldWindow = lw.GetWindow();
          oldLevel = lw.GetLevel();
          newLevel = oldLevel - (value - oldWindow)/2;
          if (!((newLevel - value/2) < lw.GetRangeMin())) 
            lw.SetLevelWindow( newLevel, value );
        }
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
      update();
    }
  }
}

/**
*
*/
void QmitkSliderLevelWindowWidget::mousePressEvent( QMouseEvent* mouseEvent ) {
  mouseDown = true;
  startPos = mouseEvent->pos();

  if ( mouseEvent->button() == QMouseEvent::LeftButton )
  {
    if (mouseEvent->state() == Qt::ControlButton || mouseEvent->state() == Qt::ShiftButton)
    {
      ctrlPressed = true;
    }
    else
    {
      ctrlPressed = false;
    }
    leftbutton = true;
  }
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
void QmitkSliderLevelWindowWidget::mouseReleaseEvent( QMouseEvent* ) 
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
  lw.SetRangeMinMax(min, max);
  lw.SetLevelWindow( (max - min) / 2, (max - min) / 2 );
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

  QWidget::repaint();

  // FIX: only emit the signal if the level window was changed by the user via mouse
  if (mouseDown && leftbutton) emit levelWindow( &lw );
}

void QmitkSliderLevelWindowWidget::updateFromLineEdit() 
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

  update();
}//end of updateFromLineEdit

void QmitkSliderLevelWindowWidget::contextMenuEvent( QContextMenuEvent * )
{  
  QPopupMenu *contextMenu = new QPopupMenu( this );
  Q_CHECK_PTR( contextMenu );
  if (scale)
    contextMenu->insertItem(tr("Hide scale"), this, SLOT(hideScale()));
  else
    contextMenu->insertItem(tr("Show scale"), this, SLOT(showScale()));
  contextMenu->insertSeparator();
  contextMenu->insertItem(tr("Default Level/Window"), this, SLOT(setDefaultLevelWindow()));
  contextMenu->insertSeparator();
  contextMenu->insertItem(tr("Change Scale Range"), this, SLOT(changeScaleRange()));
  contextMenu->insertItem(tr("Default Scale Range"), this, SLOT(setDefaultScaleRange()));
  contextMenu->insertSeparator();
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
    lw.SetLevelWindow(dlevel, dwindow);
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

void QmitkSliderLevelWindowWidget::setDefaultLevelWindow()
{
  lw.SetLevelWindow(lw.GetDefaultLevel(), lw.GetDefaultWindow());
  update();
  emit levelWindow( &lw );
}

void QmitkSliderLevelWindowWidget::hideScale()
{
  scale = FALSE;
  update();
}

void QmitkSliderLevelWindowWidget::showScale()
{
  scale = TRUE;
  update();
}

void QmitkSliderLevelWindowWidget::setDefaultScaleRange()
{
  lw.SetRangeMinMax(lw.GetDefaultRangeMin(), lw.GetDefaultRangeMax());
  lw.SetLevelWindow(lw.GetLevel(), lw.GetWindow());
  update();
  emit levelWindow( &lw );
}

void QmitkSliderLevelWindowWidget::changeScaleRange()
{
  QmitkLevelWindowRangeChange changeRange(this, "changeRange", TRUE);
  changeRange.setLowerLimit((int)lw.GetRangeMin());
  changeRange.setUpperLimit((int)lw.GetRangeMax());
  if(changeRange.exec())
  {
    lw.SetRangeMinMax(changeRange.getLowerLimit(), changeRange.getUpperLimit());
    lw.SetLevelWindow(lw.GetLevel(), lw.GetWindow());
  }
  update();
  emit levelWindow( &lw );
}
