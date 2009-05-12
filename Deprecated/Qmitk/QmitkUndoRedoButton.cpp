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
#include "QmitkUndoRedoButton.h"
// MITK header
#include "mitkUndoController.h"
// STL header
#include <list>
#include <string>

QmitkUndoRedoButton::QmitkUndoRedoButton( QWidget* parent, const char* name, mitk::VerboseLimitedLinearUndo* undoModel )
: QUndoRedoButton(parent, name),
  m_UndoModel(undoModel),
  m_ListenerEmpty(NULL),
  m_ListenerNotEmpty(NULL),
  m_FineUndo(true)
{
  setEnabled(false);
  connect( this, SIGNAL(undoRedoLast(int)), this, SLOT(doUndoRedoLast(int)) );
  connect( this, SIGNAL(buttonClicked()), this, SLOT(doUndoClick()) );
}

QmitkUndoRedoButton::~QmitkUndoRedoButton()
{
  delete m_ListenerEmpty;     // save to call even with NULL pointers
  delete m_ListenerNotEmpty;
}

void QmitkUndoRedoButton::beforePopup()
{
  // get descriptions from associated undo model
  // and build up the listbox entries for the popup
  if (m_UndoModel)
  {
    clear(); // clear whole list
  
    switch (m_Mode)
    {
      case Undo:
        descriptions = m_UndoModel->GetUndoDescriptions(); // retrieve descriptions from undo model
        break;
      case Redo:
        descriptions = m_UndoModel->GetRedoDescriptions(); // retrieve descriptions from undo model
        break;
      default:;
    }

    for ( mitk::VerboseLimitedLinearUndo::StackDescription::iterator iter = descriptions.begin(); 
          iter != descriptions.end();
          ++iter )
    {
      insertItem( QString(iter->second.c_str()) ); // append each item to the list
    }

  }
}
   
mitk::VerboseLimitedLinearUndo* QmitkUndoRedoButton::getUndoModel()
{
  return m_UndoModel;
}

void QmitkUndoRedoButton::notifyStackEmpty()
{
  setEnabled(false);
}

void QmitkUndoRedoButton::notifyStackNotEmpty()
{
  setEnabled(true);
}

void QmitkUndoRedoButton::setUndoModel(mitk::VerboseLimitedLinearUndo* undoModel)
{
  if ( m_UndoModel != undoModel)
  {
    m_UndoModel = undoModel;  // remember model
    if (!m_UndoModel) return; // on NULL, do nothing
    
    switch (m_Mode)
    {
      case Undo:
        m_ListenerEmpty = new QItkEventQtSlotAdapter<Self>(m_UndoModel, mitk::UndoEmptyEvent(), 
                                                           this, &Self::notifyStackEmpty);
        m_ListenerNotEmpty = new QItkEventQtSlotAdapter<Self>(m_UndoModel, mitk::UndoNotEmptyEvent(), 
                                                              this, &Self::notifyStackNotEmpty);
        break;
      case Redo:
        m_ListenerEmpty = new QItkEventQtSlotAdapter<Self>(m_UndoModel, mitk::RedoEmptyEvent(), 
                                                           this, &Self::notifyStackEmpty);
        m_ListenerNotEmpty = new QItkEventQtSlotAdapter<Self>(m_UndoModel, mitk::RedoNotEmptyEvent(), 
                                                              this, &Self::notifyStackNotEmpty);
        break;
      default:;
    }
  }
}

void QmitkUndoRedoButton::doUndoRedoLast(int soMany)
{
  if (!m_UndoModel) return;

  // notify the UndoModel to undo/redo a selected number of steps
  switch (m_Mode)
  {
    case Undo:
      if (soMany <= 1) 
        m_UndoModel->Undo();
      else
        m_UndoModel->Undo( descriptions.at(soMany-1).first  );
      break;
    case Redo:
      if (soMany <= 1) 
        m_UndoModel->Redo();
      else
      m_UndoModel->Redo( descriptions.at(soMany-1).first  );
      break;
    default:;
  } 
}

void QmitkUndoRedoButton::doUndoClick()
{
  if (!m_UndoModel) return;
  switch (m_Mode)
  {
    case Undo:
      m_UndoModel->Undo();
      break;
    case Redo:
      m_UndoModel->Redo();
      break;
    default:;
  }
}

void QmitkUndoRedoButton::setFineUndo(bool fine)
{
  m_FineUndo = fine;
}
