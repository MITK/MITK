#include "QmitkUndoRedoButton.h"
// MITK header
#include "mitkLimitedLinearUndo.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include "mitkUndoController.h"
// STL header
#include <list>
#include <string>

QmitkUndoRedoButton::QmitkUndoRedoButton( QWidget* parent, const char* name, mitk::VerboseLimitedLinearUndo* undoModel )
: QUndoRedoButton(parent, name),
  m_UndoModel(undoModel),
  m_ListenerEmpty(NULL),
  m_ListenerNotEmpty(NULL)
{
  setEnabled(false);
  connect( this, SIGNAL(undoRedoLast(int)), this, SLOT(doUndoRedoLast(int)) );
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
  
    std::list<std::string> descriptions;
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

    for ( std::list<std::string>::iterator iter = descriptions.begin(); iter != descriptions.end(); ++iter )
    {
      insertItem( QString((*iter).c_str()) ); // append each item to the list
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
      for (int i = 0; i < soMany; ++i)
        m_UndoModel->Undo(true); // all objectEventIDs
      break;
    case Redo:
      for (int i = 0; i < soMany; ++i)
        m_UndoModel->Redo(true); // all objectEventIDs
      break;
    default:;
  } 
}
