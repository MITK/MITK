#ifndef QMITK_UNDOREDOBUTTON_H_IQ23EW32
#define QMITK_UNDOREDOBUTTON_H_IQ23EW32

#include "mitkCommon.h"
#include "qundoredobutton.h"
#include "QmitkItkEventQtSlotAdapter.h"

namespace mitk { class VerboseLimitedLinearUndo; };


class 
MITK_EXPORT
QmitkUndoRedoButton : public QUndoRedoButton
{
  Q_OBJECT;

  typedef QmitkUndoRedoButton Self;
  
  public:

    QmitkUndoRedoButton( QWidget* parent = 0, const char* name = 0, mitk::VerboseLimitedLinearUndo* undoModel = 0 );
    virtual ~QmitkUndoRedoButton();

    mitk::VerboseLimitedLinearUndo* getUndoModel();
    void setUndoModel(mitk::VerboseLimitedLinearUndo* undoModel);

    virtual void beforePopup(); /// fill popup list just before popup

  public slots:

    void notifyStackEmpty();
    void notifyStackNotEmpty();

    void doUndoRedoLast(int);
  
  private:

    mitk::VerboseLimitedLinearUndo* m_UndoModel;

    // adapters for itk-Events
    QItkEventQtSlotAdapter<Self>* m_ListenerEmpty;
    QItkEventQtSlotAdapter<Self>* m_ListenerNotEmpty;
};

#endif
