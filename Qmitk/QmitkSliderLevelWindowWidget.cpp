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

#include <QmitkSliderLevelWindowWidget.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <itkCommand.h>
#include <QmitkLevelWindowWidgetContextMenu.h>
#include <mitkRenderingManager.h>

/**
* Constructor
*/
QmitkSliderLevelWindowWidget::QmitkSliderLevelWindowWidget( QWidget * parent, const char * name, WFlags f )
: QWidget( parent, name, f )
{
  m_Manager = mitk::LevelWindowManager::New();

  itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::New();
  command->SetCallbackFunction(this, &QmitkSliderLevelWindowWidget::OnPropertyModified);
  m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
  m_IsObserverTagSet = true;

  setMouseTracking(true);
  m_Resize = false;
  m_Bottom = false;
  m_CtrlPressed = false;
  m_MouseDown = false;
  
  m_Font.setPointSize( 6 );
  
  m_MoveHeight = height() - 25;
  m_ScaleVisible = true;
  m_Contextmenu = new QmitkLevelWindowWidgetContextMenu(this, "contextMenu", true);

  setBackgroundMode( Qt::NoBackground );

  this->hide();
  update();
}

QmitkSliderLevelWindowWidget::~QmitkSliderLevelWindowWidget()
{
  if ( m_IsObserverTagSet)
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
}

void QmitkSliderLevelWindowWidget::setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager)
{
  if ( m_IsObserverTagSet)
  {
    m_Manager->RemoveObserver(m_ObserverTag);
   m_IsObserverTagSet = false;
  }
  m_Manager = levelWindowManager;
  if ( m_Manager.IsNotNull() )
  {
    itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::New();
    command->SetCallbackFunction(this, &QmitkSliderLevelWindowWidget::OnPropertyModified);
    m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
    m_IsObserverTagSet = true;
  }
}

void QmitkSliderLevelWindowWidget::OnPropertyModified(const itk::EventObject& )
{
  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();
    this->show();
    update();
  }
  catch(...)
  {
    try
    {
      this->hide();
    }
    catch(...)
    {
    }
  }
}

void QmitkSliderLevelWindowWidget::paintEvent( QPaintEvent* itkNotUsed(e) ) 
{
  QPixmap pm(width(), height());
  pm.fill( static_cast<QWidget*>(parent())->paletteBackgroundColor() );
  QPainter painter(&pm);

  painter.setFont( m_Font );
  painter.setPen(static_cast<QWidget*>(parent())->paletteForegroundColor());

  QColor c(93,144,169);
  QColor cl = c.light();
  QColor cd = c.dark();

  painter.setBrush(c);
  painter.drawRect(m_Rect);

  float mr = m_LevelWindow.GetRange();

  if ( mr < 1 )
    mr = 1;
 
  float fact = (float) m_MoveHeight / mr;

  //begin draw scale
  if (m_ScaleVisible)
  {
    int minRange = (int)m_LevelWindow.GetRangeMin();
    int maxRange = (int)m_LevelWindow.GetRangeMax();
    int yValue = m_MoveHeight + (int)(minRange*fact);
    QString s = " 0";
    if (minRange <= 0 && maxRange >= 0)
    {
      painter.drawLine( 5, yValue , 15, yValue);
      painter.drawText( 21, yValue + 3, s );
    }

    int count = 1;
    int k = 5;
    bool enoughSpace = false;
    bool enoughSpace2 = false;

    for(int i = m_MoveHeight + (int)(minRange*fact); i < m_MoveHeight;)
    {
      if (-count*20 < minRange)
        break;
      yValue = m_MoveHeight + (int)((minRange + count*20)*fact);
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

    for(int i = m_MoveHeight + (int)(minRange*fact); i >= 0;)
    {
      if (count*20 > maxRange)
        break;
      yValue = m_MoveHeight + (int)((minRange - count*20)*fact);
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
  painter.drawLine(m_Rect.topLeft(),m_Rect.topRight());
  painter.drawLine(m_Rect.topLeft(),m_Rect.bottomLeft());
  
  painter.setPen (cd);
  painter.drawLine(m_Rect.topRight(),m_Rect.bottomRight());
  painter.drawLine(m_Rect.bottomRight(),m_Rect.bottomLeft());
  painter.end();

  QPainter p (this);
  p.drawPixmap(0, 0, pm);
}

/**
*
*/
void QmitkSliderLevelWindowWidget::mouseMoveEvent( QMouseEvent* mouseEvent ) {
  if ( m_LevelWindow.IsFixed() )
    return;
  if (!m_MouseDown)
  {
    if (((mouseEvent->pos().y() >= (m_Rect.topLeft().y() - 3) && mouseEvent->pos().y() <= (m_Rect.topLeft().y() + 3))
      || (mouseEvent->pos().y() >= (m_Rect.bottomLeft().y() - 3) && mouseEvent->pos().y() <= (m_Rect.bottomLeft().y() + 3)))
      && mouseEvent->pos().x() >= m_Rect.topLeft().x() && mouseEvent->pos().x() <= m_Rect.topRight().x())
    {
      setCursor(SizeVerCursor);
      QToolTip::remove(this, m_UpperBound);
      QToolTip::remove(this, m_LowerBound);
      m_LowerBound.setRect(m_Rect.bottomLeft().x(), m_Rect.bottomLeft().y() - 3, 17, 7);
      QToolTip::add(this, m_LowerBound, "Ctrl + left click to change only lower bound");
      m_UpperBound.setRect(m_Rect.topLeft().x(), m_Rect.topLeft().y() - 3, 17, 7);
      QToolTip::add(this, m_UpperBound, "Ctrl + left click to change only upper bound");
      m_Resize = true;
      if ((mouseEvent->pos().y() >= (m_Rect.bottomLeft().y() - 3) && mouseEvent->pos().y() <= (m_Rect.bottomLeft().y() + 3)))
        m_Bottom = true;
    }
    else
    {
      QToolTip::remove(this, m_LowerBound);
      QToolTip::remove(this, m_UpperBound);
      setCursor(ArrowCursor);
      m_Resize = false;
      m_Bottom = false;
    }
  }

  else {

    float fact = (float) m_MoveHeight / m_LevelWindow.GetRange();

    if ( m_Leftbutton ) 
    {
      if (m_Resize && !m_CtrlPressed)
      {
        double diff = (mouseEvent->pos().y()) / fact;
        diff -= (m_StartPos.y()) / fact;
        m_StartPos = mouseEvent->pos();

        if (diff == 0) return;
        float value;
        if (m_Bottom)
          value = m_LevelWindow.GetWindow() + ( ( 2 * diff ) );
        else
          value = m_LevelWindow.GetWindow() - ( ( 2 * diff ) );

        if ( value < 1 )
          value = 1;

        m_LevelWindow.SetLevelWindow( m_LevelWindow.GetLevel(), value );
      } 
      else if(m_Resize && m_CtrlPressed)
      {
        if (!m_Bottom)
        {
          double diff = (mouseEvent->pos().y()) / fact;
          diff -= (m_StartPos.y()) / fact;
          m_StartPos = mouseEvent->pos();

          if (diff == 0) return;
          float value;
          
          value = m_LevelWindow.GetWindow() - ( ( diff ) );

          if ( value < 1 )
            value = 1;
          float oldWindow;
          float oldLevel;
          float newLevel;
          oldWindow = m_LevelWindow.GetWindow();
          oldLevel = m_LevelWindow.GetLevel();
          newLevel = oldLevel + (value - oldWindow)/2;
          if (!((newLevel + value/2) > m_LevelWindow.GetRangeMax())) 
            m_LevelWindow.SetLevelWindow( newLevel, value );
        }
        else
        {
          double diff = (mouseEvent->pos().y()) / fact;
          diff -= (m_StartPos.y()) / fact;
          m_StartPos = mouseEvent->pos();

          if (diff == 0) return;
          float value;
          
          value = m_LevelWindow.GetWindow() + ( ( diff ) );

          if ( value < 1 )
            value = 1;
          float oldWindow;
          float oldLevel;
          float newLevel;
          oldWindow = m_LevelWindow.GetWindow();
          oldLevel = m_LevelWindow.GetLevel();
          newLevel = oldLevel - (value - oldWindow)/2;
          if (!((newLevel - value/2) < m_LevelWindow.GetRangeMin())) 
            m_LevelWindow.SetLevelWindow( newLevel, value );
        }
      }
      else
      {
        float maxv = m_LevelWindow.GetRangeMax();
        float minv = m_LevelWindow.GetRangeMin();
        float wh = m_LevelWindow.GetWindow() / 2;
  
        float value = (m_MoveHeight - mouseEvent->pos().y()) / fact + minv;
       
        if ( value - wh < minv )
          m_LevelWindow.SetLevelWindow( m_LevelWindow.GetRangeMin() + wh, m_LevelWindow.GetWindow() );

        else if ( value + wh > maxv )
          m_LevelWindow.SetLevelWindow( m_LevelWindow.GetRangeMax() - wh, m_LevelWindow.GetWindow() );

        else
          m_LevelWindow.SetLevelWindow( value, m_LevelWindow.GetWindow() );
      }
      m_Manager->SetLevelWindow(m_LevelWindow);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

/**
*
*/
void QmitkSliderLevelWindowWidget::mousePressEvent( QMouseEvent* mouseEvent ) {
  if ( m_LevelWindow.IsFixed() )
    return;
  m_MouseDown = true;
  m_StartPos = mouseEvent->pos();

  if ( mouseEvent->button() == QMouseEvent::LeftButton )
  {
    if (mouseEvent->state() == Qt::ControlButton || mouseEvent->state() == Qt::ShiftButton)
    {
      m_CtrlPressed = true;
    }
    else
    {
      m_CtrlPressed = false;
    }
    m_Leftbutton = true;
  }
  else
    m_Leftbutton = false;

  mouseMoveEvent( mouseEvent );
}

/**
*
*/
void QmitkSliderLevelWindowWidget::resizeEvent ( QResizeEvent * event ) {
  m_MoveHeight = event->size().height() - 25;
  update();
}

/**
*
*/
void QmitkSliderLevelWindowWidget::mouseReleaseEvent( QMouseEvent* ) 
{
  if ( m_LevelWindow.IsFixed() )
    return;
  m_MouseDown = false;
}

/**
*
*/
void QmitkSliderLevelWindowWidget::update() {
  int rectWidth;
  if(m_ScaleVisible)
  {
    rectWidth = 17;
    setMinimumSize ( QSize( 50, 50 ) );
    setMaximumSize ( QSize( 50, 2000 ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
  }
  else
  {
    rectWidth = 26;
    setMinimumSize ( QSize( 40, 50 ) );
    setMaximumSize ( QSize( 50, 2000 ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
  }
  float mr = m_LevelWindow.GetRange();

  if ( mr < 1 )
    mr = 1;

  float fact = (float) m_MoveHeight / mr;

  float rectHeight = m_LevelWindow.GetWindow() * fact;

  if ( rectHeight < 15 )
    rectHeight = 15;

  if ( m_LevelWindow.GetMin() < 0 )
    m_Rect.setRect( 2, (int) (m_MoveHeight - (m_LevelWindow.GetMax() - m_LevelWindow.GetRangeMin()) * fact) , rectWidth, (int) rectHeight );
  else
    m_Rect.setRect( 2, (int) (m_MoveHeight - (m_LevelWindow.GetMax() - m_LevelWindow.GetRangeMin()) * fact), rectWidth, (int) rectHeight );
  
  QWidget::repaint();
}

void QmitkSliderLevelWindowWidget::contextMenuEvent( QContextMenuEvent * )
{ 
  m_Contextmenu->setLevelWindowManager(m_Manager.GetPointer());
  QPopupMenu *contextMenu = new QPopupMenu( this );
  Q_CHECK_PTR( contextMenu );
  if (m_ScaleVisible)
    contextMenu->insertItem(tr("Hide Scale"), this, SLOT(hideScale()));
  else
    contextMenu->insertItem(tr("Show Scale"), this, SLOT(showScale()));
  contextMenu->insertSeparator();
  m_Contextmenu->getContextMenu(contextMenu);
}

void QmitkSliderLevelWindowWidget::hideScale()
{
  m_ScaleVisible = false;
  update();
}

void QmitkSliderLevelWindowWidget::showScale()
{
  m_ScaleVisible = true;
  update();
}

void QmitkSliderLevelWindowWidget::setDataTree(mitk::DataTree* tree)
{
  m_Manager->SetDataTree(tree);
}

mitk::LevelWindowManager* QmitkSliderLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
