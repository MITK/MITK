#include "QmitkUndoRedoView.h"

// MITK includes
#include <mitkRenderingManager.h>
#include <mitkUndoController.h>
#include <mitkVerboseLimitedLinearUndo.h>
#include "mitkCoreServices.h"
#include "mitkIPreferencesService.h"
#include "mitkIPreferences.h"

#include <QmitkRenderWindow.h>

// Qt includes
#include <QMessageBox>
#include <QStandardItem>
#include <QIcon>
#include <QInputDialog>
#include <QmitkStyleManager.h>



namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("/General/UndoRedo");
  }

  void SetUndoLimitPreference(unsigned int limit)
  {
    auto* prefs = GetPreferences();

    if (prefs != nullptr)
    {
      prefs->PutInt("UndoLimit", limit);
    }
  }
}

const std::string QmitkUndoRedoView::VIEW_ID = "org.mitk.views.undoredoview";

QmitkUndoRedoView::QmitkUndoRedoView()
  : m_Controls(new Ui::QmitkUndoRedoViewControls()), m_UndoRedoModel(nullptr)
{
  m_UndoController.reset(new mitk::UndoController(mitk::UndoController::VERBOSE_LIMITEDLINEARUNDO));
}

QmitkUndoRedoView::~QmitkUndoRedoView()
{
}

void QmitkUndoRedoView::CreateQtPartControl(QWidget* parent)
{
  // Setup UI
  m_Controls->setupUi(parent);

  // Get the undo model from the global undo stack
  auto undoModel = dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel());

  if (!undoModel)
  {
    QMessageBox::warning(parent, "Warning", "No VerboseLimitedLinearUndo model found. Undo/Redo functionality will be unavailable.");
    m_Controls->undoButton->setEnabled(false);
    m_Controls->redoButton->setEnabled(false);
    m_Controls->undoRedoListView->setEnabled(false);
    return;
  }

  auto basePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/");
  m_Controls->undoButton->setIcon(QmitkStyleManager::ThemeIcon(basePath + "edit-undo.svg"));
  m_Controls->redoButton->setIcon(QmitkStyleManager::ThemeIcon(basePath + "edit-redo.svg"));

  // Setup the model for the list view
  m_UndoRedoModel = new QStandardItemModel(this);
  m_UndoRedoModel->setColumnCount(1);
  m_UndoRedoModel->setHeaderData(0, Qt::Horizontal, "Description");

  // Set the model directly to the view without proxy
  m_Controls->undoRedoListView->setModel(m_UndoRedoModel);

  // Connect signals and slots
  connect(m_Controls->undoButton, SIGNAL(clicked()), this, SLOT(OnUndoButtonClicked()));
  connect(m_Controls->redoButton, SIGNAL(clicked()), this, SLOT(OnRedoButtonClicked()));
  connect(m_Controls->btnLimit, SIGNAL(clicked()), this, SLOT(OnChangeLimitClicked()));
  connect(m_Controls->checkLimit, &QAbstractButton::toggled, this, &QmitkUndoRedoView::OnCheckLimitChanged);

  // Use ITKEventObserver to listen for undo stack changes
  m_UndoStackObserverGuard = mitk::ITKEventObserverGuard(undoModel, mitk::UndoStackEvent(), [this](const itk::EventObject&) {this->OnUndoStackChanged(); });

  // Update the view initially
  UpdateUndoRedoList();
  UpdateButtonStatus();
}

void QmitkUndoRedoView::SetFocus()
{
  m_Controls->undoRedoListView->setFocus();
}

void QmitkUndoRedoView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& /*nodes*/)
{
  // Not reacting to selection changes
  // This view shows the global undo stack regardless of selected nodes
}

void QmitkUndoRedoView::NodeAdded(const mitk::DataNode* /*node*/)
{
  // Not needed, we're using the ITKEventObserver for updates
}

void QmitkUndoRedoView::NodeRemoved(const mitk::DataNode* /*node*/)
{
  // Not needed, we're using the ITKEventObserver for updates
}

void QmitkUndoRedoView::OnUndoButtonClicked()
{
  auto undoModel = this->GetUndoModel();
  if (nullptr != undoModel)
  {
    undoModel->Undo();
  }
}

void QmitkUndoRedoView::OnRedoButtonClicked()
{
  auto undoModel = this->GetUndoModel();
  if (nullptr != undoModel)
  {
    undoModel->Redo();
  }
}

void QmitkUndoRedoView::OnChangeLimitClicked()
{
  auto undoModel = this->GetUndoModel();
  if (nullptr != undoModel)
  {
    bool ok = false;
    auto newLimit = QInputDialog::getInt(m_Controls->undoRedoListView->parentWidget(), "Select the undo stack limit", "New max undo steps:",undoModel->GetUndoLimit(), 1, 9999, 1, &ok);
    if (ok)
    {
      undoModel->SetUndoLimit(newLimit);
      SetUndoLimitPreference(newLimit);
      this->UpdateUndoRedoList();
      this->UpdateButtonStatus();
    }
 }
}

void QmitkUndoRedoView::OnCheckLimitChanged(bool)
{
  auto undoModel = this->GetUndoModel();
  if (nullptr != undoModel)
  {
    if (m_Controls->checkLimit->isChecked() && undoModel->GetUndoLimit() == 0)
    {
      undoModel->SetUndoLimit(100);
    }
    else if (!m_Controls->checkLimit->isChecked())
    {
      undoModel->SetUndoLimit(0);
    }
  }
  this->UpdateButtonStatus();
}

void QmitkUndoRedoView::OnUndoStackChanged()
{
  // The undo stack has changed, update our view
  // This is called by the ITK event mechanism when the undo stack changes
  this->UpdateUndoRedoList();
  this->UpdateButtonStatus();
}

mitk::VerboseLimitedLinearUndo* QmitkUndoRedoView::GetUndoModel() const
{
  return dynamic_cast<mitk::VerboseLimitedLinearUndo*>(mitk::UndoController::GetCurrentUndoModel());
}

void QmitkUndoRedoView::UpdateUndoRedoList()
{
  auto undoModel = this->GetUndoModel();
  if (nullptr==undoModel || !m_UndoRedoModel)
    return;

  // Clear the current model
  m_UndoRedoModel->removeRows(0, m_UndoRedoModel->rowCount());

  // Get the undo/redo descriptions
  const auto undoDescriptions = undoModel->GetUndoDescriptions();
  const auto redoDescriptions = undoModel->GetRedoDescriptions();

  // Current operation position as separator
  QStandardItem* currentPositionItem = new QStandardItem("--- Current Position ---");
  currentPositionItem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
  currentPositionItem->setBackground(QBrush(m_Controls->undoRedoListView->palette().color(QPalette::Highlight)));
  currentPositionItem->setForeground(QBrush(m_Controls->undoRedoListView->palette().color(QPalette::Dark)));
  currentPositionItem->setFlags(Qt::ItemIsEnabled); // Make it non-selectable

  auto basePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/");

  // Add undo-able operations (in reverse order - most recent first)
  for (auto it = undoDescriptions.rbegin(); it != undoDescriptions.rend(); ++it)
  {
    const auto& [id, description] = *it;
    QStandardItem* item = new QStandardItem(QString::fromStdString("Undo: " + description));
    item->setIcon(QmitkStyleManager::ThemeIcon(basePath + "edit-undo.svg"));
    m_UndoRedoModel->appendRow(item);
  }

  // Add current position marker
  m_UndoRedoModel->appendRow(currentPositionItem);

  // Add redoable operations
  for (const auto& [id, description] : redoDescriptions)
  {
    QStandardItem* item = new QStandardItem(QString::fromStdString("Redo: " + description));
    item->setIcon(QmitkStyleManager::ThemeIcon(basePath + "edit-redo.svg"));
    m_UndoRedoModel->appendRow(item);
  }
}

void QmitkUndoRedoView::UpdateButtonStatus()
{
  auto undoModel = this->GetUndoModel();

  m_Controls->undoRedoListView->setEnabled(nullptr != undoModel);

  m_Controls->undoButton->setEnabled(nullptr != undoModel && !undoModel->UndoListEmpty());
  m_Controls->redoButton->setEnabled(nullptr != undoModel && !undoModel->RedoListEmpty());

  m_Controls->checkLimit->setEnabled(nullptr != undoModel);
  m_Controls->checkLimit->setChecked(nullptr != undoModel && undoModel->GetUndoLimit() > 0);
  m_Controls->labelLimit->setVisible(nullptr != undoModel && m_Controls->checkLimit->isChecked());
  m_Controls->btnLimit->setVisible(nullptr != undoModel && m_Controls->checkLimit->isChecked());
  QString limitText = "unknown";
  if (nullptr != undoModel)
  {
    if (undoModel->GetUndoLimit() > 0)
    {
      limitText = QString::number(undoModel->GetUndoLimit());
    }
    else
    {
      limitText = "unlimited";
    }
  }
  m_Controls->labelLimit->setText(limitText);
}
