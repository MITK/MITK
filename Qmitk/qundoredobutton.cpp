#include "qundoredobutton.h"
#include "popupundoredo.h"
#include "popup_arrow.xpm"
#include "undoarrow.xpm"
#include "redoarrow.xpm"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qapplication.h>

QUndoRedoButton::QUndoRedoButton( QWidget* parent, const char* name)
:QFrame( parent, name ),
 m_Mode(Undo),
 m_actionName("Undo")
{
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  setLineWidth(2);
  
  // create HBoxLayout with two buttons
  m_layout = new QHBoxLayout(this);
  m_layout->setMargin(1);
 
  // the "do 1 undo/redo" button
  m_iconbutton = new QPushButton("", this);
  m_iconbutton->setFlat(true);
  m_layout->addWidget(m_iconbutton);

  // some separation line
  QFrame* frame = new QFrame(this);
  frame->setFrameStyle( QFrame::VLine | QFrame::Plain );
  m_layout->addWidget( frame );
 
  // the "popup undo/redo list" button
  m_arrowbutton = new QPushButton("", this);
  m_arrowbutton->setPixmap( QPixmap(popup_arrow_xpm) );
  m_arrowbutton->setFlat(true);
  m_layout->addWidget(m_arrowbutton);

  m_popup = new PopupUndoRedoListBox(parent);
  connect( m_popup, SIGNAL( selectedRange(int) ), this, SLOT(popupItemsSelected(int)) );
  connect( m_iconbutton, SIGNAL( clicked() ) , this, SLOT(iconButtonClicked()) );
  connect( m_arrowbutton, SIGNAL( pressed() ) , this, SLOT(arrowButtonClicked()) );

  setMode(Undo);
}

QUndoRedoButton::~QUndoRedoButton()
{
  // nothing to do
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
  emit undoRedoLast(1);
}

void QUndoRedoButton::arrowButtonClicked()
{
  //m_popup->popup(m_arrowbutton);

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

void QUndoRedoButton::insertStrList( const QStrList* sl, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertStrList(sl,index);
}

void QUndoRedoButton::insertStrList( const QStrList& sl, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertStrList(sl,index);
}

void QUndoRedoButton::insertStrList( const char ** strs, int numStrings, int index )
{
  if (!m_popup) return;
  m_popup->listbox()->insertStrList(strs,numStrings,index);
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
