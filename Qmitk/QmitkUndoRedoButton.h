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
#ifndef QMITK_UNDOREDOBUTTON_H_IQ23EW32
#define QMITK_UNDOREDOBUTTON_H_IQ23EW32

#include "mitkCommon.h"
#include "qundoredobutton.h"
#include "QmitkItkEventQtSlotAdapter.h"
#include "mitkVerboseLimitedLinearUndo.h"

/*!
 * \brief A Qt widget for undo/redo, which is able to listen to MITK undo models.
 *
 * This widget provides a Word-like undo button, which can either undo only the last 
 * action (simple click on the button) or display the undo history to the user 
 * (click on arrow) so multiple actions can be undone in one step.
 *
 * A property m_Mode is inherited from QUndoRedoButton, which can be either "Undo" or "Redo".
 * Depending on this mode, differenct functions of the associated undo model are called.
 * \ingroup Widgets
 */
class 
MITK_EXPORT
QmitkUndoRedoButton : public QUndoRedoButton
{
  Q_OBJECT;

  typedef QmitkUndoRedoButton Self;
  
  public:

    /// Either give the undoModel right here
    QmitkUndoRedoButton( QWidget* parent = 0, const char* name = 0, mitk::VerboseLimitedLinearUndo* undoModel = 0 );
    virtual ~QmitkUndoRedoButton();

    mitk::VerboseLimitedLinearUndo* getUndoModel();

    /// You can change the undoModel after construction
    void setUndoModel(mitk::VerboseLimitedLinearUndo* undoModel);

    /// Fills popup list just before popup
    virtual void beforePopup(); 

    /// \brief This is not really used until now.
    ///
    /// Perhaps later one could use the grounp IDs of the undo models to group some items of the
    /// action history. Then this function would be useful to determine what should happen
    /// on a single click on the button.
    void setFineUndo(bool);

  public slots:
    
    /// Connected to an itk::Event of the undo model
    void notifyStackEmpty();
    /// Connected to an itk::Event of the undo model
    void notifyStackNotEmpty();

    /// Connected to undoRedoLast(int) of the super class
    void doUndoRedoLast(int);
    /// Connected to buttonClicked(int) of the super class
    void doUndoClick();
  
  private:

    /// The associated undo model
    mitk::VerboseLimitedLinearUndo* m_UndoModel;
    
    /// The undo/redo history as reported by the undo model
    mitk::VerboseLimitedLinearUndo::StackDescription descriptions;

    /// Adapter for itk-Events
    QItkEventQtSlotAdapter<Self>* m_ListenerEmpty;
    /// Adapter for itk-Events
    QItkEventQtSlotAdapter<Self>* m_ListenerNotEmpty;

    bool m_FineUndo;
};

#endif
