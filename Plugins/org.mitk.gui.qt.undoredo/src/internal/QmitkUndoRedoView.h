#ifndef QmitkUndoRedoView_h
#define QmitkUndoRedoView_h

// MITK includes
#include <QmitkAbstractView.h>
#include <mitkITKEventObserverGuard.h>

// Qt includes
#include <QItemSelection>
#include <QStandardItemModel>

// UI includes
#include "ui_QmitkUndoRedoView.h"

namespace mitk
{
  class VerboseLimitedLinearUndo;
}

/**
 * \class QmitkUndoRedoView
 * \brief A view that shows all available undo and redo steps stored in a mitk::VerboseLimitedLinearUndo instance
 *
 * This view provides a list of all undo and redo commands currently available
 * in the MITK undo stack. It allows users to see the history of operations and
 * navigate through them using undo and redo buttons.
 */
class QmitkUndoRedoView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkUndoRedoView();
  ~QmitkUndoRedoView() override;

protected:
  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  void NodeAdded(const mitk::DataNode* node) override;
  void NodeRemoved(const mitk::DataNode* node) override;

private slots:
  void OnUndoButtonClicked();
  void OnRedoButtonClicked();
  void OnListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
  mitk::VerboseLimitedLinearUndo* GetUndoModel() const;
  void UpdateUndoRedoList();
  void UpdateButtonStatus();
  void OnUndoStackChanged(const itk::EventObject& e);

  std::unique_ptr<Ui::QmitkUndoRedoViewControls> m_Controls;
  QStandardItemModel* m_UndoRedoModel;

  mitk::ITKEventObserverGuard m_UndoStackObserverGuard;
  std::unique_ptr<mitk::UndoController> m_UndoController;
};

#endif // QmitkUndoRedoView_h
