/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkTotalSegmentatorToolGUI.h>
#include <ui_QmitkTotalSegmentatorToolGUI.h>

#include <mitkTotalSegmentatorInstall.h>
#include <mitkTotalSegmentatorTool.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkProgressTask.h>

#include <QmitkPipInstallDialog.h>
#include <QmitkIconTheme.h>
#include <QmitkInfoCard.h>

#include <QComboBox>
#include <QMouseEvent>
#include <QPointer>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QTimer>

#include <algorithm>

MITK_TOOL_GUI_MACRO(MITKPYTHONSEGMENTATIONUI_EXPORT, QmitkTotalSegmentatorToolGUI, "")

namespace
{
  const QString PREFERENCES_NODE = "org.mitk.views.segmentation";
  const QString PREFERENCE_PAGE_ID = "org.mitk.gui.qt.application.TotalSegmentatorPreferencePage";

  // Combo-box item-data roles.
  constexpr int TaskRole = Qt::UserRole;         // QString: the bare task id
  constexpr int SpeedRole = Qt::UserRole + 1;    // int: TotalSegmentatorTool::Speed
  constexpr int LicensedRole = Qt::UserRole + 2; // bool: task needs a license

  QString GetSpeedLabel(const mitk::TotalSegmentatorTool::SpeedInfo& speed)
  {
    const QString name = speed.Value == mitk::TotalSegmentatorTool::Speed::Fast ? "fast" : "fastest";

    return speed.Resolution > 0
      ? QString("%1 (%2 mm)").arg(name).arg(speed.Resolution)
      : name;
  }

  // Toggles a checkable item on a click anywhere on it, not only on its check
  // box. The view emits clicked() for a click on the box as well, after the
  // base delegate toggled it, so toggling in a clicked() slot would toggle a
  // click on the box twice.
  class ToggleOnClickDelegate : public QStyledItemDelegate
  {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override
    {
      if (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton &&
          (index.flags() & Qt::ItemIsUserCheckable) && (index.flags() & Qt::ItemIsEnabled))
      {
        const auto state = index.data(Qt::CheckStateRole).value<Qt::CheckState>();
        return model->setData(index, state == Qt::Checked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
      }

      return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
  };
}

QmitkTotalSegmentatorToolGUI::QmitkTotalSegmentatorToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(),
    m_Ui(std::make_unique<Ui::QmitkTotalSegmentatorToolGUI>())
{
  // Only allow confirming once a non-empty preview exists, so a cancelled or
  // empty run leaves the Confirm button disabled.
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  {
    auto tool = this->GetTool();
    if (tool == nullptr)
      return false;

    auto preview = tool->GetPreviewSegmentation();
    if (preview == nullptr || preview->GetAllLabelValues().empty())
      return false;

    return enabled;
  };
}

QmitkTotalSegmentatorToolGUI::~QmitkTotalSegmentatorToolGUI()
{
  if (auto* tool = this->GetTool(); tool != nullptr)
  {
    tool->InitializedEvent -= mitk::MessageDelegate<QmitkTotalSegmentatorToolGUI>(this, &QmitkTotalSegmentatorToolGUI::OnToolInitialized);
    tool->OptionsChangedEvent -= mitk::MessageDelegate<QmitkTotalSegmentatorToolGUI>(this, &QmitkTotalSegmentatorToolGUI::OnToolOptionsChanged);
  }

  if (m_Preferences != nullptr)
  {
    m_Preferences->OnPropertyChanged -=
      mitk::MessageDelegate1<QmitkTotalSegmentatorToolGUI, const mitk::IPreferences::ChangeEvent&>(
        this, &QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent);
  }
}

void QmitkTotalSegmentatorToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  // Keep the task combo the same height as the Settings button they share the top
  // row with; a QComboBox's default height is otherwise slightly shorter.
  m_Ui->taskComboBox->setMinimumHeight(m_Ui->settingsButton->sizeHint().height());

  m_Ui->runButton->setIcon(QmitkIconTheme::GetIcon(
    QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg")));

  m_Ui->structuresList->setItemDelegate(new ToggleOnClickDelegate(m_Ui->structuresList));

  // How many names of the checked structures the summary shows depends on the
  // width of its label, see ShowStructuresSummary().
  m_Ui->structuresLabel->installEventFilter(this);

  // Hidden until the install state below sets the first message.
  this->SetStatus(QString());

  connect(m_Ui->installButton, &QPushButton::clicked, this, &Self::OnInstallButtonClicked);
  connect(m_Ui->settingsButton, &QPushButton::clicked, this, &Self::OnSettingsButtonClicked);
  connect(m_Ui->runButton, &QPushButton::clicked, this, &Self::OnRunButtonClicked);
  connect(m_Ui->taskComboBox, &QComboBox::currentIndexChanged, this, &Self::OnTaskSelected);
  connect(m_Ui->structuresList, &QListWidget::itemChanged, this, &Self::OnStructureChanged);
  connect(m_Ui->clearStructuresButton, &QPushButton::clicked, this, &Self::OnClearStructuresClicked);
  connect(m_Ui->structuresFilter, &QLineEdit::textChanged, this, &Self::OnStructuresFilterChanged);

  auto prefService = mitk::CoreServices::GetPreferencesService();
  m_Preferences = prefService->GetSystemPreferences()->Node(PREFERENCES_NODE.toStdString());
  if (m_Preferences != nullptr)
  {
    m_Preferences->OnPropertyChanged +=
      mitk::MessageDelegate1<QmitkTotalSegmentatorToolGUI, const mitk::IPreferences::ChangeEvent&>(
        this, &QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent);
  }

  Superclass::InitializeUI(mainLayout);
}

void QmitkTotalSegmentatorToolGUI::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  if (auto* tool = dynamic_cast<mitk::TotalSegmentatorTool*>(newTool); tool != nullptr)
  {
    tool->InitializedEvent += mitk::MessageDelegate<QmitkTotalSegmentatorToolGUI>(this, &QmitkTotalSegmentatorToolGUI::OnToolInitialized);
    tool->OptionsChangedEvent += mitk::MessageDelegate<QmitkTotalSegmentatorToolGUI>(this, &QmitkTotalSegmentatorToolGUI::OnToolOptionsChanged);
  }

  this->RefreshInstallState();
}

void QmitkTotalSegmentatorToolGUI::DisconnectOldTool(mitk::SegWithPreviewTool* oldTool)
{
  if (auto* tool = dynamic_cast<mitk::TotalSegmentatorTool*>(oldTool); tool != nullptr)
  {
    tool->InitializedEvent -= mitk::MessageDelegate<QmitkTotalSegmentatorToolGUI>(this, &QmitkTotalSegmentatorToolGUI::OnToolInitialized);
    tool->OptionsChangedEvent -= mitk::MessageDelegate<QmitkTotalSegmentatorToolGUI>(this, &QmitkTotalSegmentatorToolGUI::OnToolOptionsChanged);
  }

  Superclass::DisconnectOldTool(oldTool);
}

mitk::TotalSegmentatorTool* QmitkTotalSegmentatorToolGUI::GetTool()
{
  return this->GetConnectedToolAs<mitk::TotalSegmentatorTool>();
}

void QmitkTotalSegmentatorToolGUI::RefreshInstallState()
{
  const bool installed = mitk::TotalSegmentatorTool::IsInstalled();
  const auto* tool = this->GetTool();
  const bool initialized = tool != nullptr && tool->IsInitialized();

  m_Ui->installButton->setVisible(!installed);
  m_Ui->taskComboBox->setVisible(installed);
  m_Ui->runButton->setVisible(installed);
  this->SetStructuresVisible(installed && initialized);

  if (!installed)
  {
    this->SetStatus("TotalSegmentator is not installed. Click 'Install TotalSegmentator'.");
    return;
  }

  if (initialized)
  {
    this->PopulateTasks();
    return;
  }

  m_Ui->taskComboBox->setEnabled(false);
  m_Ui->runButton->setEnabled(false);
  this->SetStatus("Starting TotalSegmentator...");

  // Not while the tool is being activated, which this may be called from.
  QTimer::singleShot(0, this, &Self::InitializeTool);
}

void QmitkTotalSegmentatorToolGUI::InitializeTool()
{
  auto* tool = this->GetTool();

  if (tool == nullptr || tool->IsInitialized())
    return;

  // Initialize() processes events, during which a tool-manager change can
  // delete this GUI. OnToolInitialized() shows the tasks.
  QPointer<QmitkTotalSegmentatorToolGUI> self(this);
  mitk::ProgressTask task("Starting TotalSegmentator", mitk::ProgressTask::Indeterminate, true);

  try
  {
    tool->Initialize(&task);
  }
  catch (const mitk::Exception& e)
  {
    if (self.isNull())
      return;

    this->SetStatus(task.IsCancelRequested()
      ? QString("Starting TotalSegmentator was cancelled. Open the settings and close them to try again.")
      : QString::fromStdString(e.GetDescription()), true);
  }
}

void QmitkTotalSegmentatorToolGUI::OnToolInitialized()
{
  this->SetStructuresVisible(true);
  this->PopulateTasks();
}

void QmitkTotalSegmentatorToolGUI::OnToolOptionsChanged()
{
  this->ShowToolOptions();
  this->UpdateRunButtonState();
}

void QmitkTotalSegmentatorToolGUI::PopulateTasks()
{
  const auto* tool = this->GetTool();

  if (tool == nullptr)
    return;

  const bool hasLicense = mitk::TotalSegmentatorTool::HasLicense();

  // Rebuilt without emitting currentIndexChanged for each intermediate state,
  // which would choose those tasks for the tool.
  {
    QSignalBlocker blocker(m_Ui->taskComboBox);
    m_Ui->taskComboBox->clear();

    for (const auto& info : tool->GetTasks())
    {
      const auto task = QString::fromStdString(info.Name);

      // Display-only annotations; the stored task id (TaskRole) stays the bare name
      // that TotalSegmentator expects.
      const QString modality = info.MR ? QStringLiteral(" (MR)") : QString();
      const QString prefix = info.Licensed ? QStringLiteral("License required: ") : QString();

      for (const auto& speed : info.Speeds)
      {
        const auto label = speed.Value == mitk::TotalSegmentatorTool::Speed::Standard
          ? prefix + task + modality
          : prefix + task + " - " + GetSpeedLabel(speed) + modality;

        m_Ui->taskComboBox->addItem(label);
        const int index = m_Ui->taskComboBox->count() - 1;
        m_Ui->taskComboBox->setItemData(index, task, TaskRole);
        m_Ui->taskComboBox->setItemData(index, static_cast<int>(speed.Value), SpeedRole);
        m_Ui->taskComboBox->setItemData(index, info.Licensed, LicensedRole);

        // Licensed tasks stay selectable. Disabling them made the drop-down misbehave
        // (clicking a disabled entry jumped to an arbitrary enabled one); instead they
        // get a "License required: " prefix and the Run button is gated in
        // UpdateRunButtonState.
        if (info.Licensed && !hasLicense)
          m_Ui->taskComboBox->setItemData(index, QStringLiteral("Requires a TotalSegmentator license. Set it in Settings."), Qt::ToolTipRole);
      }
    }
  }

  m_Ui->taskComboBox->setEnabled(!m_Running);
  m_ListedTask.clear();

  this->ShowToolOptions();
  this->UpdateRunButtonState();
}

void QmitkTotalSegmentatorToolGUI::ShowToolOptions()
{
  const auto* tool = this->GetTool();

  if (tool == nullptr || !tool->IsInitialized())
    return;

  const auto task = QString::fromStdString(tool->GetTask());
  const auto speed = static_cast<int>(tool->GetSpeed());

  for (int i = 0; i < m_Ui->taskComboBox->count(); ++i)
  {
    if (m_Ui->taskComboBox->itemData(i, TaskRole).toString() == task && m_Ui->taskComboBox->itemData(i, SpeedRole).toInt() == speed)
    {
      QSignalBlocker blocker(m_Ui->taskComboBox);
      m_Ui->taskComboBox->setCurrentIndex(i);
      break;
    }
  }

  this->ShowStructures();
}

void QmitkTotalSegmentatorToolGUI::ShowStructures()
{
  const auto* tool = this->GetTool();
  const auto* task = tool != nullptr ? tool->FindTask(tool->GetTask()) : nullptr;

  if (task == nullptr || !task->SupportsRoiSubset)
  {
    this->SetStructuresVisible(false);
    m_ListedTask.clear();
    return;
  }

  this->SetStructuresVisible(true);

  const auto& subset = tool->GetRoiSubset();

  // The checks only change, unless the task does.
  {
    QSignalBlocker blocker(m_Ui->structuresList);

    if (m_ListedTask != task->Name)
    {
      m_Ui->structuresList->clear();

      for (const auto& [value, name] : task->Classes)
      {
        auto* item = new QListWidgetItem(QString::fromStdString(name), m_Ui->structuresList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      }

      m_ListedTask = task->Name;
      this->OnStructuresFilterChanged(m_Ui->structuresFilter->text());
    }

    for (int i = 0; i < m_Ui->structuresList->count(); ++i)
    {
      auto* item = m_Ui->structuresList->item(i);
      const bool checked = std::find(subset.begin(), subset.end(), item->text().toStdString()) != subset.end();
      item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
  }

  m_CheckedStructures.clear();

  for (const auto& name : subset)
    m_CheckedStructures << QString::fromStdString(name);

  this->ShowStructuresSummary();
  m_Ui->clearStructuresButton->setEnabled(!subset.empty() && !m_Running);
}

void QmitkTotalSegmentatorToolGUI::ShowStructuresSummary()
{
  auto* label = m_Ui->structuresLabel;
  const auto count = m_CheckedStructures.size();

  if (count == 0)
  {
    label->setText("Structures: all");
    label->setToolTip(QString());
    return;
  }

  const auto metrics = label->fontMetrics();
  const int width = label->contentsRect().width();
  auto text = QString("Structures: %1 selected").arg(count);

  for (auto shown = count; shown > 0; --shown)
  {
    auto candidate = "Structures: " + m_CheckedStructures.first(shown).join(", ");

    if (shown < count)
      candidate += QString(" and %1 more").arg(count - shown);

    if (metrics.horizontalAdvance(candidate) <= width)
    {
      text = candidate;
      break;
    }
  }

  // Only an extremely narrow panel cuts the fallback.
  label->setText(metrics.elidedText(text, Qt::ElideRight, width));

  // Rich text, which tool tips wrap.
  label->setToolTip(QString("<p>%1 of %2: %3</p>")
    .arg(count)
    .arg(m_Ui->structuresList->count())
    .arg(m_CheckedStructures.join(", ").toHtmlEscaped()));
}

bool QmitkTotalSegmentatorToolGUI::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Ui->structuresLabel && event->type() == QEvent::Resize)
    this->ShowStructuresSummary();

  return Superclass::eventFilter(watched, event);
}

void QmitkTotalSegmentatorToolGUI::SetStructuresVisible(bool visible)
{
  m_Ui->structuresLabel->setVisible(visible);
  m_Ui->clearStructuresButton->setVisible(visible);
  m_Ui->structuresFilter->setVisible(visible);
  m_Ui->structuresList->setVisible(visible);
}

void QmitkTotalSegmentatorToolGUI::OnTaskSelected()
{
  auto* tool = this->GetTool();
  const int index = m_Ui->taskComboBox->currentIndex();

  if (tool != nullptr && index >= 0)
  {
    // The combo box offers only tasks and speeds that the tool accepts.
    tool->SetTask(m_Ui->taskComboBox->itemData(index, TaskRole).toString().toStdString(),
      static_cast<mitk::TotalSegmentatorTool::Speed>(m_Ui->taskComboBox->itemData(index, SpeedRole).toInt()));
  }

  this->UpdateRunButtonState();
}

void QmitkTotalSegmentatorToolGUI::OnStructureChanged(QListWidgetItem*)
{
  auto* tool = this->GetTool();

  if (tool == nullptr)
    return;

  std::vector<std::string> subset;

  for (int i = 0; i < m_Ui->structuresList->count(); ++i)
  {
    const auto* item = m_Ui->structuresList->item(i);

    if (item->checkState() == Qt::Checked)
      subset.push_back(item->text().toStdString());
  }

  // The list offers only the structures of the tool's task.
  tool->SetRoiSubset(subset);
}

void QmitkTotalSegmentatorToolGUI::OnClearStructuresClicked()
{
  if (auto* tool = this->GetTool(); tool != nullptr)
    tool->SetRoiSubset({});
}

void QmitkTotalSegmentatorToolGUI::OnStructuresFilterChanged(const QString& text)
{
  for (int i = 0; i < m_Ui->structuresList->count(); ++i)
  {
    auto* item = m_Ui->structuresList->item(i);
    item->setHidden(!text.isEmpty() && !item->text().contains(text, Qt::CaseInsensitive));
  }
}

void QmitkTotalSegmentatorToolGUI::UpdateRunButtonState()
{
  // A run keeps the button disabled and the status on itself.
  if (m_Running)
    return;

  const auto* tool = this->GetTool();
  const int index = m_Ui->taskComboBox->currentIndex();

  if (tool == nullptr || !tool->IsInitialized() || index < 0)
  {
    m_Ui->runButton->setEnabled(false);
    return;
  }

  const bool licensed = m_Ui->taskComboBox->itemData(index, LicensedRole).toBool();

  if (licensed && !mitk::TotalSegmentatorTool::HasLicense())
  {
    m_Ui->runButton->setEnabled(false);
    this->SetStatus("This task requires a TotalSegmentator license. Set it in Settings.", true);
  }
  else
  {
    m_Ui->runButton->setEnabled(true);
    this->SetStatus("TotalSegmentator is ready. Select a task and run.");
  }
}

void QmitkTotalSegmentatorToolGUI::OnInstallButtonClicked()
{
  auto spec = mitk::TotalSegmentator::BuildInstallSpec(mitk::TotalSegmentator::VENV_NAME);

  QmitkPipInstallDialog dialog(spec, this->window());

  if (dialog.exec() == QDialog::Accepted)
    this->RefreshInstallState();
}

void QmitkTotalSegmentatorToolGUI::OnSettingsButtonClicked()
{
  mitk::CoreServices::GetPreferencesService()->OpenPreferencesDialog(PREFERENCE_PAGE_ID.toStdString());

  // The preferences dialog is modal; on return the user may have uninstalled,
  // cleared weights, or changed the license, so re-evaluate what the tool shows.
  this->RefreshInstallState();
}

void QmitkTotalSegmentatorToolGUI::OnRunButtonClicked()
{
  auto tool = this->GetTool();
  if (tool == nullptr)
    return;

  // UpdatePreview() processes events, during which a tool-manager change can
  // delete this GUI. BusyStateChanged() shows the run and its result.
  QPointer<QmitkTotalSegmentatorToolGUI> self(this);

  try
  {
    tool->UpdatePreview();
  }
  catch (const std::exception& e)
  {
    if (!self.isNull())
      this->SetStatus(QString("Error: %1").arg(e.what()), true);
  }
}

void QmitkTotalSegmentatorToolGUI::BusyStateChanged(bool isBusy)
{
  Superclass::BusyStateChanged(isBusy);

  // The task and the settings must not change while a run uses them.
  m_Running = isBusy;
  m_Ui->taskComboBox->setEnabled(!isBusy);
  m_Ui->settingsButton->setEnabled(!isBusy);
  m_Ui->structuresList->setEnabled(!isBusy);
  m_Ui->clearStructuresButton->setEnabled(!isBusy && this->GetTool() != nullptr && !this->GetTool()->GetRoiSubset().empty());

  if (isBusy)
  {
    m_Ui->runButton->setEnabled(false);
    this->SetStatus("Running TotalSegmentator. This can take a while...");
    return;
  }

  this->UpdateRunButtonState();
  this->ShowRunResult();
}

void QmitkTotalSegmentatorToolGUI::ShowRunResult()
{
  auto* tool = this->GetTool();
  if (tool == nullptr)
    return;

  auto preview = tool->GetPreviewSegmentation();
  if (preview != nullptr && !preview->GetAllLabelValues().empty())
  {
    this->SetLabelSetPreview(preview);
    this->ActualizePreviewLabelVisibility();
    this->SetStatus("Segmentation finished.");
  }
  else if (tool->LastRunSucceeded())
  {
    this->SetStatus("TotalSegmentator found none of the structures in this image.");
  }
  else
  {
    // Distinguish a genuine failure (which the tool records) from a user
    // cancellation, which leaves no error message.
    const auto& error = tool->GetLastErrorMessage();
    if (!error.empty())
      this->SetStatus(QString::fromStdString(error), true);
    else
      this->SetStatus("Segmentation was cancelled.", true);
  }
}

void QmitkTotalSegmentatorToolGUI::SetStatus(const QString& message, bool isError)
{
  // Errors of TotalSegmentator can contain angle brackets, and the lines of its
  // last output.
  m_Ui->statusCard->SetMessage(message.toHtmlEscaped().replace('\n', "<br>"), isError ? QmitkInfoCard::Severity::Error : QmitkInfoCard::Severity::Info);
  m_Ui->statusCard->setVisible(!message.isEmpty());
}

void QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event)
{
  // Re-enable licensed tasks as soon as a license is applied via the settings.
  if (event.GetProperty() == "TotalSeg/hasLicense")
    this->RefreshInstallState();
}
