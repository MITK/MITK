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
#include "qundoredobutton.h"
#include "popupundoredo.h"
#include "popup_arrow.xpm"
#include "undoarrow.xpm"
#include "redoarrow.xpm"

#include <qlayout.h>
#include <qtoolbutton.h>
//#include <qpushbutton.h>
#include <qapplication.h>

QUndoRedoButton::QUndoRedoButton( QWidget* parent, const char* name)
:QFrame( parent, name ),
 m_Mode(Undo),
 m_actionName("Undo")
{
  setFrameStyle( QFrame::NoFrame );
  setLineWidth(0);
  
  // create HBoxLayout with two buttons
  m_layout = new QHBoxLayout(this);
  m_layout->setMargin(0);
 
  // the "do 1 undo/redo" button
  m_iconbutton = new QToolButton(this);
  m_iconbutton->setAutoRaise(true);
  m_layout->addWidget(m_iconbutton);

  // the "popup undo/redo list" button
  m_arrowbutton = new QToolButton(this);
  m_arrowbutton->setPixmap( QPixmap(popup_arrow_xpm) );
  m_arrowbutton->setAutoRaise(true);
  m_layout->addWidget(m_arrowbutton);
  
  // our popup belongs to the whole screen, so it could be drawn outside the toplevel window's borders
  int scr;
  if ( QApplication::desktop()->isVirtualDesktop() )
    scr = QApplication::desktop()->screenNumber( parent->mapToGlobal( pos() ) );
  else
    scr = QApplication::desktop()->screenNumber( parent );
 
  m_popup = new PopupUndoRedoListBox(QApplication::desktop()->screen( scr ));
  
  connect( m_popup, SIGNAL( selectedRange(int) ), this, SLOT(popupItemsSelected(int)) );
  connect( m_iconbutton, SIGNAL( clicked() ) , this, SLOT(iconButtonClicked()) );
  connect( m_arrowbutton, SIGNAL( pressed() ) , this, SLOT(arrowButtonClicked()) );

  setMode(Undo);

  polish();
  adjustSize();
}

QUndoRedoButton::~QUndoRedoButton()
{
  // nothing to do
  delete m_popup;
}

const QPixmap* QUndoRedoButton::pixmap() const
{
  return m_iconbutton->pixmap();
}

void QUndoRedoButton::setPixmap( const QPixmap& pm)
{
  m_iconbutton->setPixmap(pm);
}

QUndoRedoButton::Mode QUndoRedoButton::getMode() const
{
  return m_Mode;
}

void QUndoRedoButton::setMode(Mode m)
{
  m_Mode = m;
  switch (m_Mode)
  {
    case Undo:
      m_iconbutton->setPixmap( QPixmap(undoarrow_xpm) );
      setActionName("Undo");
      break;
    case Redo:
      m_iconbutton->setPixmap( QPixmap(redoarrow_xpm) );
      setActionName("Redo");
      break;
    default:;
  }
}

const QString& QUndoRedoButton::actionName() const
{
  return m_actionName;
}
  
void QUndoRedoButton::setActionName( const QString& description )
{
  m_actionName = description;
  if (!m_popup) return;
  m_popup->setInfoPhrases( QString("Select action to ") + description, description + " %1 actions");
}
    
void QUndoRedoButton::popupItemsSelected(int num)
{
  emit undoRedoLast(num);
}

void QUndoRedoButton::iconButtonClicked()
{
  emit buttonClicked();
}

void QUndoRedoButton::arrowButtonClicked()
{
  beforePopup();  // for derived classes to fill the popuplist
  
  qApp->processEvents();
    
  m_popup->popup(this);
}

QListBox* QUndoRedoButton::listbox() const
{
  if (!m_popup) return NULL;
  return m_popup->listbox();
}

void QUndoRedoButton::mouseReleaseEvent (QMouseEvent*)
{
  m_arrowbutton->setDown(false);
}

void QUndoRedoButton::insertStringList( const QStringList& sl, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertStringList(sl,index);
}

void QUndoRedoButton::insertItem( const QListBoxItem* lbi, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertItem(lbi,index);
}

void QUndoRedoButton::insertItem( const QListBoxItem* lbi, const QListBoxItem *after )
{
  if (!m_popup) return;
  m_popup->listbox()->insertItem(lbi,after);
}

void QUndoRedoButton::insertItem( const QString &text, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertItem(text,index);
}

void QUndoRedoButton::insertItem( const QPixmap &pixmap, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertItem(pixmap,index);
}

void QUndoRedoButton::insertItem( const QPixmap &pixmap, const QString &text, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertItem(pixmap,text,index);
}

void QUndoRedoButton::removeItem( int index )
{
  if (!m_popup) return;
  m_popup->listbox()->removeItem(index);
}

void QUndoRedoButton::changeItem( const QListBoxItem* lbi, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->changeItem(lbi, index);
}

void QUndoRedoButton::changeItem( const QString &text, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->changeItem(text, index);
}

void QUndoRedoButton::changeItem( const QPixmap &pixmap, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->changeItem(pixmap, index);
}

void QUndoRedoButton::changeItem( const QPixmap &pixmap, const QString &text, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->changeItem(pixmap, text, index);
}

void QUndoRedoButton::clear()
{
  if (!m_popup) return;
  m_popup->listbox()->clear();
}
