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
#include "popupactionhistory.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>

#include <algorithm>

  
PassOnMouseListBox::PassOnMouseListBox(QWidget* parent, const char* name)
:QListBox(parent, name),
 my_parent(parent)
{
  setSelectionMode(QListBox::Multi);
  setAutoScrollBar(false);
  setMouseTracking (true);
}

void PassOnMouseListBox::mousePressEvent(QMouseEvent * e)
{
  QApplication::sendEvent( my_parent, new QMouseEvent(QEvent::MouseButtonPress, mapToParent(e->pos()), e->button(), e->state()) );
}

void PassOnMouseListBox::mouseMoveEvent(QMouseEvent* e)
{
  QApplication::sendEvent( my_parent, new QMouseEvent(QEvent::MouseMove, mapToParent(e->pos()), Qt::NoButton, e->state()) );
}

void PassOnMouseListBox::mouseReleaseEvent(QMouseEvent * e)
{
  QApplication::sendEvent( my_parent, new QMouseEvent(QEvent::MouseButtonRelease, mapToParent(e->pos()), e->button(), e->state()) );
}


PopupActionHistory::PopupActionHistory(QWidget* parent, const char* name)
: QFrame (parent, name, WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM),
  m_moves(0),
  my_parent(parent)
{
  QVBoxLayout* vbox = new QVBoxLayout(this, lineWidth());
  
  setLineWidth(2);
  setMouseTracking ( TRUE );
 
  m_listbox = new PassOnMouseListBox( this, 0 );
  m_listbox->show();

  vbox->addWidget(m_listbox);
  
  m_label = new QLabel( 0, this, 0 );
  m_label->setAlignment ( AlignCenter );
  m_label->show();
  vbox->addWidget(m_label);
  
  setMargin(0);
  setAutoMask( FALSE );
  setFrameStyle ( QFrame::Panel | QFrame::Raised );
  setLineWidth( 2 );
  polish();
  adjustSize();

  hide();
}

void PopupActionHistory::mouseMoveEvent (QMouseEvent*)
{  
  m_moves++;
}

void PopupActionHistory::mouseReleaseEvent (QMouseEvent* e)
{
  if  (rect().contains( e->pos () ) || m_moves) 
    close ();
  ++m_moves;
}

void PopupActionHistory::closeEvent (QCloseEvent*e)
{
  e->accept ();

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

void PopupActionHistory::popup(QWidget* parent)
{
  m_popupParent = parent;
  // TODO: make the following move more intelligent, 
  // e.g. when popupParent is near the bottom of the screen
  // Problem: then the layout of label and listbox has to be changed, plus the logic in mouseMoveEvent! Items have to be ordered from bottom to top
  if (m_popupParent)
  {
    move ( m_popupParent->mapToGlobal( m_popupParent->rect().bottomLeft() ) );
  }
  
  m_listbox->clearSelection();
  
  // TODO: could the following be done nicer, in a more general way?
  resize(   std::max((int)m_listbox->maxItemWidth() + m_listbox->lineWidth()*2 + frameWidth()*2, 150),
            4 * m_listbox->itemHeight() 
          + m_label->height() 
          + lineWidth()*2 
        );
  
  show();
  raise();
  grabMouse();
  m_moves = 0;
}

