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
#include <QmitkColorPropertyEditor.h>

#include <qlayout.h>
#include <qpainter.h>
#include <qapplication.h>
#include <mitkRenderingManager.h>

//----- QmitkPopupColorChooser ---------------------------------------------------------

QmitkPopupColorChooser::QmitkPopupColorChooser(QWidget* parent, unsigned int steps, unsigned int size, const char* name)
: QFrame (parent, name, WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM),
  my_parent(parent)
{
  setSteps(steps);
  
  setLineWidth(2);
  setMouseTracking ( TRUE );
  
  setMargin(0);
  setAutoMask( FALSE );
  setFrameStyle ( QFrame::Panel | QFrame::Raised );
  setLineWidth( 1 );
  polish();
  resize(size, size);

  hide();
}

QmitkPopupColorChooser::~QmitkPopupColorChooser()
{
}

void QmitkPopupColorChooser::setSteps(int steps)
{
  m_Steps = steps;
  m_Steps2 = m_Steps / 2;
  m_HStep = 360 / m_Steps;
  m_SStep = 512 / m_Steps;
  m_VStep = 512 / m_Steps;
}

void QmitkPopupColorChooser::keyReleaseEvent(QKeyEvent*)
{
  emit colorSelected( m_OriginalColor );
  close();
}

void QmitkPopupColorChooser::mouseMoveEvent (QMouseEvent* e)
{  
  double x(e->pos().x());
  double y(e->pos().y());
  x /= width();
  
  if ( x >= 0.0 ) 
  {
    //x = (int)(x / (1.0/m_Steps)) * (1.0/m_Steps); // div stepsize * stepsize
    x = (int)(x * (float)(m_Steps-1)) / (float)(m_Steps-1); // same as above
    if (x > 1.0) x = 1.0;
    if (x < 0.0) x = 0.0;
  }
  
  y /= height();
  if (y >= 1.0) y = 0.9;
  if (y < 0.0) y = 0.0;
  y = (int)(y * (float)m_Steps) / (float)m_Steps;

  m_H = static_cast<int>( y * 359.0 );
  if ( x >= 0.5 )
  {
    m_S = static_cast<int>( (1.0 - x) * 511.0 );
    if ( m_S > 255 ) m_S = 255;
    m_V = 255;
  }
  else
  {
    m_S = 255;
    if ( x < 0.0 )
      m_V = 0;
    else
    {
      m_V = static_cast<int>( x * 511.0 + 511.0 / (float)(m_Steps-1) );
      if ( m_V > 255 ) m_V = 255;
    }
  }

  QColor color;
  color.setHsv(m_H, m_S, m_V);

  emit colorSelected( color );
}

void QmitkPopupColorChooser::mouseReleaseEvent (QMouseEvent*)
{
  close ();
}

void QmitkPopupColorChooser::closeEvent (QCloseEvent*e)
{
  e->accept ();

  releaseKeyboard();
  releaseMouse();

  if (!m_popupParent) return;

  // remember that we (as a popup) might recieve the mouse release
  // event instead of the popupParent. This is due to the fact that
  // the popupParent popped us up in its mousePressEvent handler. To
  // avoid the button remaining in pressed state we simply send a
  // faked mouse button release event to it.
  // Maleike: parent should not pop us on MouseRelease!
  QMouseEvent  me( QEvent::MouseButtonRelease, QPoint(0,0), QPoint(0,0), QMouseEvent::LeftButton, QMouseEvent::NoButton);
  QApplication::sendEvent ( m_popupParent, &me );
}

void QmitkPopupColorChooser::popup(QWidget* parent, const QPoint& point, const mitk::Color* color)
{
  m_popupParent = parent;
  if (m_popupParent)
  {
    QPoint newPos;

    if (color)
    {
      QColor qcolor( (int)((*color)[0] * 255.0) , (int)((*color)[1] * 255.0) , (int)((*color)[2] * 255.0) );
      int h,s,v;
      qcolor.getHsv(&h, &s, &v);
      
      if ( h == -1 ) // set by Qt if color is achromatic ( but this widget does not display grays )
        h = 10;  // red

      int x, y;
      float cellwidth = (float)width() / (float)(m_Steps);

      if ( s > v ) // restrict to the colors we can display
      {  // left side, ramp from v = 255/m_Steps to v = 255
        s = 255;
        x = (int)(
           (
             ((float)v / 255.0)      
            *
             ((float)m_Steps2)
            -
             1.0
           )
          *
           cellwidth
          + cellwidth/2
          );
      }
      else
      {
        v = 255;
        x = (int)(
           (
             (1.0 - ((float)s / 255.0))      
            *
             ((float)m_Steps2)
           )
          *
           cellwidth
          + cellwidth/2
          
          + width() / 2
          );
      }

      y = (int)( (float)h/360.0 * (float)m_Steps * cellwidth );
   
      m_OriginalColor.setHsv(h,s,v);
      
      // move to color
      newPos.setX( point.x() - x );
      newPos.setY( point.y() - y );
    }
    else
    {
      // center widget
      m_OriginalColor.setHsv(-1, 0, 0);

      newPos.setX( point.x() - width() / 2 );
      newPos.setY( point.y() - height() / 2 );
    }
    move ( m_popupParent->mapToGlobal( newPos ) );
  }
  
  show();
  raise();
  grabMouse();
  grabKeyboard();
}

void QmitkPopupColorChooser::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  drawGradient( &painter );
}

void QmitkPopupColorChooser::drawGradient( QPainter* p)
{
  p->setWindow( 0, 0, m_Steps-1, m_Steps );       // defines coordinate system
  p->setPen( Qt::NoPen ); 

  QColor c;
  for ( unsigned int h = 0; h < m_Steps; ++h )
  {
    for ( unsigned int v = 1; v < m_Steps2; ++v )
    {                
      c.setHsv( h*m_HStep, 255, v*m_VStep );             // rainbow effect
      p->setBrush( c );                  // solid fill with color c
      p->drawRect( v-1, h, m_Steps2, m_Steps );         // draw the rectangle
    }
    for ( unsigned int s = 0; s < m_Steps2; ++s )
    {                
      c.setHsv( h*m_HStep, 255 - s*m_SStep, 255 );             // rainbow effect
      p->setBrush( c );                  // solid fill with color c
      p->drawRect( m_Steps2+s-1, h, m_Steps2, m_Steps );         // draw the rectangle
    }
  }
}

//----- QmitkColorPropertyEditor --------------------------------------------------

// initialization of static pointer to color picker widget
QmitkPopupColorChooser* QmitkColorPropertyEditor::colorChooser = NULL;
                    int QmitkColorPropertyEditor::colorChooserRefCount = 0;

QmitkColorPropertyEditor::QmitkColorPropertyEditor( const mitk::ColorProperty* property, QWidget* parent, const char* name )
: QmitkColorPropertyView( property, parent, name )
{
  // our popup belongs to the whole screen, so it could be drawn outside the toplevel window's borders
  int scr;
  if ( QApplication::desktop()->isVirtualDesktop() )
    scr = QApplication::desktop()->screenNumber( parent->mapToGlobal( pos() ) ); 
  else
    scr = QApplication::desktop()->screenNumber( parent ); 

  if ( colorChooserRefCount == 0 )
  {
    colorChooser = new QmitkPopupColorChooser( QApplication::desktop()->screen( scr ), 50 );
  }
  ++colorChooserRefCount;
}

QmitkColorPropertyEditor::~QmitkColorPropertyEditor()
{
  --colorChooserRefCount;
  if (!colorChooserRefCount)
  {
    delete colorChooser;
    colorChooser = NULL;
  }
}

void QmitkColorPropertyEditor::mousePressEvent(QMouseEvent* e)
{
  connect( colorChooser, SIGNAL(colorSelected(QColor)), this, SLOT(onColorSelected(QColor)) );
  if (m_ColorProperty)
  {
    colorChooser->popup( this, e->pos(), &(m_ColorProperty->GetColor()) );
  }
}

void QmitkColorPropertyEditor::mouseReleaseEvent(QMouseEvent*)
{
  disconnect( colorChooser, SIGNAL(colorSelected(QColor)), this, SLOT(onColorSelected(QColor)) );
}

void QmitkColorPropertyEditor::onColorSelected(QColor c)
{
  if (m_ColorProperty)
  {
    int r,g,b;
    c.getRgb( &r, &g, &b );
    const_cast<mitk::ColorProperty*>(m_ColorProperty)->SetColor( r / 255.0, g / 255.0, b / 255.0 );
    const_cast<mitk::ColorProperty*>(m_ColorProperty)->Modified();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

