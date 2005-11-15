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
#include "popupundoredo.h"

#include <qlabel.h>

PopupUndoRedoListBox::PopupUndoRedoListBox(QWidget* parent, const char*  name)
: PopupActionHistory(parent, name),
  m_num_selected(0),
  m_initialPhrase(""),
  m_mouseMovePhrase("")
{
}

void PopupUndoRedoListBox::setInfoPhrases(const QString& initialPhrase, const QString& mouseMovePhrase)
{
  m_initialPhrase = initialPhrase;
  m_mouseMovePhrase = mouseMovePhrase;
}

void PopupUndoRedoListBox::popup(QWidget* parent)
{
  m_num_selected = 0;
  m_label->setText(tr(m_initialPhrase));
  PopupActionHistory::popup(parent);
}

void PopupUndoRedoListBox::mouseMoveEvent (QMouseEvent* e)
{  
  // determine item under mouse
  int to = m_listbox->index(m_listbox->itemAt(e->pos()));
  m_num_selected = to+1;
  
  // if there are more items than visible, grow visible listbox
  // when 3 are visible, the box should grow, when the mouse is over the 2nd item
  if ( to > 0 && to >= m_listbox->numItemsVisible()-1 && m_listbox->numItemsVisible() < m_listbox->numRows() )
  {
    resize( QSize(width(), height() + m_listbox->itemHeight()) );
  }
  
  // select all items above and including item under mouse, deselect all others
  for (int i = 0, total = m_listbox->numRows(); i < total; ++i)
  {
    m_listbox->setSelected(i, i <= to);
  }
    
  m_label->setText(tr(m_mouseMovePhrase).arg(m_num_selected));

  PopupActionHistory::mouseMoveEvent(e);
}

void PopupUndoRedoListBox::closeEvent (QCloseEvent*e)
{
  if (m_num_selected > 0) emit selectedRange(m_num_selected);
  
  PopupActionHistory::closeEvent(e);
}

