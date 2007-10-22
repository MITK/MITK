/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include "QmitkUserInputSimulation.h"

#include <qevent.h>
#include <qapplication.h>

#include <math.h>

#include <iostream>
#include <stdlib.h>

void QmitkUserInputSimulation::MouseDown( QWidget* widget, int button, int state )
{
  if (!widget) return;

  MouseDownXY( widget, widget->width()/2, widget->height()/2, button, state );
}


void QmitkUserInputSimulation::MouseDownXY( QWidget* widget, int x, int y, int button, int state )
{
  if (!widget) return;

  QMouseEvent* me = new QMouseEvent( QEvent::MouseButtonPress, QPoint( x, y ), button, state );
  QApplication::postEvent( widget, me );
  qApp->processEvents();
}


void QmitkUserInputSimulation::MouseMoveXY( QWidget* widget, int x, int y, int button, int state )
{
  if (!widget) return;

  QMouseEvent* me = new QMouseEvent( QEvent::MouseMove, QPoint( x, y ), widget->mapToGlobal(QPoint( x, y )), button, state );
  QApplication::postEvent( widget, me );
  qApp->processEvents();
}


void QmitkUserInputSimulation::MouseRelease( QWidget* widget, int button, int state )
{
  if (!widget) return;

  MouseReleaseXY( widget, widget->width()/2, widget->height()/2, button, state );
}


void QmitkUserInputSimulation::MouseReleaseXY( QWidget* widget, int x, int y, int button, int state )
{
  if (!widget) return;

  QMouseEvent* me = new QMouseEvent( QEvent::MouseButtonRelease, QPoint( x, y ), button, state );
  QApplication::postEvent( widget, me );
  qApp->processEvents();
}
    
void QmitkUserInputSimulation::MouseClick( QWidget* widget, int button, int state )
{
  if (!widget) return;

  MouseDown   ( widget, button, state );
  MouseRelease( widget, button, state );
}
    
void QmitkUserInputSimulation::MouseMoveScrollWheel( QWidget* widget, int delta )
{
  MouseMoveScrollWheel( widget, widget->width()/2, widget->height()/2, delta );
}

void QmitkUserInputSimulation::MouseMoveScrollWheel( QWidget* widget, int x, int y, int delta )
{
  if (!widget) return;

  QWheelEvent* we = new QWheelEvent( QPoint( x, y ), delta, 0 );
  QApplication::postEvent( widget, we );
  qApp->processEvents();
}
    
void QmitkUserInputSimulation::MouseDrawRandom( QWidget* widget, int button, unsigned int points )
{
  if (!widget) return;

  float w = (float)widget->width();
  float h = (float)widget->height();
  
  for (unsigned int i = 0; i <= points; ++i )
  {
    double r;
    r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
    float x = r * w;

    r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
    float y = r * h;
    
    if (i == 0 ) 
    {
      MouseDownXY( widget, (int)x, (int)y, button ); // mouse down
    }

    MouseMoveXY( widget, (int)x, (int)y, button );  // mouse move
    //std::cout << "(" << x << "," << y << ") " << std::flush;

    if (i == points ) 
    {
      MouseReleaseXY( widget, (int)x, (int)y, button ); // mouse release
    }
  }

  // TODO generate points first and write to file (reproducibility)
}

void QmitkUserInputSimulation::MouseDrawCircle( QWidget* widget, int button, float relativePositionX, float relativePositionY, float relativeRadius )
{
  if (!widget) return;

  int firstDegree = -180;
  int lastDegree = 180;
  int degreeStep = 10;
  int secondLastDegree = lastDegree - degreeStep;
  float w = (float)widget->width() * relativeRadius;
  float h = (float)widget->height() * relativeRadius;
  float x0 = relativePositionX * (float)widget->width();
  float y0 = relativePositionY * (float)widget->height();

  for (int i = firstDegree; i < lastDegree; i += degreeStep )
  {
    float rad = (float)i * 3.1415926535 / 180.0;
   
    float x = cos( rad ) * w + x0;
    float y = sin( rad ) * h + y0;
    
    if (i == firstDegree ) 
    {
      MouseDownXY( widget, (int)x, (int)y, button ); // mouse down
    }

    MouseMoveXY( widget, (int)x, (int)y, button );  // mouse move
    //std::cout << "(" << x << "," << y << ") " << std::flush;

    if (i == secondLastDegree ) 
    {
      MouseReleaseXY( widget, (int)x, (int)y, button ); // mouse release
    }
  }
}
    
void QmitkUserInputSimulation::MouseDrawLine( QWidget* widget, int button, Qt::Orientation orientation )
{
  if (!widget) return;

  float relativeradius = 0.4f;
  float w = (float)widget->width() * relativeradius;
  float h = (float)widget->height() * relativeradius;
  float x0 = (float)widget->width() / 2.0;
  float y0 = (float)widget->height() / 2.0;

  for (int i = -100; i < 100; i += 5 )
  {
    float x,y;

    if (orientation == Qt::Horizontal)
    {
      x = ((float)i/100.0) * w + x0;
      y = y0;
    }
    else
    {
      x = x0;
      y = ((float)i/100.0) * h + y0;
    }
    
    if (i == -100 ) 
    {
      MouseDownXY( widget, (int)x, (int)y, button ); // mouse down
    }

    MouseMoveXY( widget, (int)x, (int)y, button );  // mouse move
    //std::cout << "(" << x << "," << y << ") " << std::flush;

    if (i == 99 ) 
    {
      MouseReleaseXY( widget, (int)x, (int)y, button ); // mouse release
    }
  }
}

void QmitkUserInputSimulation::SimulateKeyboardTyping( QWidget* widget, const QString& text )
{
  for ( unsigned int i = 0; i < text.length(); ++i )
  {
    KeyboardTypeKey(widget, (char)text.at(i) );
  }
}

void QmitkUserInputSimulation::KeyboardTypeKey( QWidget* widget, char c )
{
  KeyboardKeyDown( widget, c );
  KeyboardKeyRelease( widget, c );
}

void QmitkUserInputSimulation::KeyboardKeyDown( QWidget* widget, char c )
{
  // TODO replace parameter 2 (Qt::Key_unknown) with something sensible
  QKeyEvent* ke = new QKeyEvent( QEvent::KeyPress, Qt::Key_unknown, QChar(c).latin1(), 0, QChar(c) );
  QApplication::postEvent( widget, ke );
  qApp->processEvents();
}

void QmitkUserInputSimulation::KeyboardKeyRelease( QWidget* widget, char c )
{
  // TODO replace parameter 2 (Qt::Key_unknown) with something sensible
  QKeyEvent* ke = new QKeyEvent( QEvent::KeyRelease, Qt::Key_unknown, QChar(c).latin1(), 0, QChar(c) );
  QApplication::postEvent( widget, ke );
  qApp->processEvents();
}

void KeyboardInput( QWidget* widget, int key, int state )
{
  QKeyEvent* ke = new QKeyEvent( QEvent::KeyPress, key, NULL, state );
  QApplication::postEvent( widget, ke );
  QKeyEvent* ke2 = new QKeyEvent( QEvent::KeyRelease, key, NULL, 0 );
  QApplication::postEvent( widget, ke2 );
  qApp->processEvents();
}


