/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtLogView.h"
#include <ui_berryQtLogView.h>

#include "berryQtLogFilterProxyModel.h"
#include "berryQtLogPlugin.h"
#include "berryQtPlatformLogModel.h"

#include <berryPlatform.h>

#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QHelpEvent>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>
#include <QToolTip>

#include <algorithm>
#include <array>

namespace
{
  /** \brief The levels offered as a minimum, ordered by severity.
   *
   * Fatal is missing on purpose: nothing in MITK logs at that level, so the
   * entry could never match anything. A Fatal message still outranks Error and
   * so remains visible at every threshold offered here.
   */
  constexpr std::array SelectableLevels = {
    mitk::LogLevel::Debug,
    mitk::LogLevel::Info,
    mitk::LogLevel::Warn,
    mitk::LogLevel::Error
  };

  /** Long enough that typing does not refilter the whole table per keystroke,
   *  short enough not to feel like lag.
   */
  constexpr int FilterDelayInMs = 200;

  mitk::IPreferences* GetPreferences()
  {
    auto* prefService = berry::Platform::GetPreferencesService();
    return prefService->GetSystemPreferences()->Node("org_blueberry_ui_qt_log");
  }

  mitk::LogLevel FindLevelByName(const QString& name, mitk::LogLevel fallback)
  {
    for (auto level : SelectableLevels)
    {
      if (berry::QtPlatformLogModel::GetLevelName(level) == name)
        return level;
    }

    return fallback;
  }
}

namespace berry {

QtLogView::QtLogView(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::QtLogViewClass>()),
    m_Model(QtLogPlugin::GetInstance()->GetLogModel()),
    m_FilterModel(new QtLogFilterProxyModel(this)),
    m_FilterTimer(new QTimer(this)),
    m_FollowNewEntries(true),
    m_ScrolledToNewest(false)
{
  ui->setupUi(this);

  auto* prefs = GetPreferences();

  // Retired with the two checkboxes the details toggle replaced.
  prefs->Remove("ShowAdvancedFields");
  prefs->Remove("ShowCategory");

  const bool showDetails = prefs->GetBool("ShowDetails", false);
  const auto minimumLevel = FindLevelByName(
    QString::fromStdString(prefs->Get("MinimumLevel", "Info")), mitk::LogLevel::Info);

  // Connected before the proxy attaches to the same model, so that the position
  // is sampled while the view still reflects the rows it currently shows.
  connect(m_Model, &QAbstractItemModel::rowsAboutToBeInserted, this, &QtLogView::OnRowsAboutToBeInserted);

  m_FilterModel->setSourceModel(m_Model);
  m_FilterModel->setSortRole(QtPlatformLogModel::SortRole);
  m_FilterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_FilterModel->setDynamicSortFilter(true);

  ui->tableView->setModel(m_FilterModel);

  // Every section stays Interactive, including Message: a section in Stretch
  // mode has no working resize handle, which would leave the divider next to
  // Message dead. Message is instead given the remaining width explicitly.
  auto* header = ui->tableView->horizontalHeader();
  header->setStretchLastSection(false);

  // Every row is a single line, so the rows need no measuring.
  auto* verticalHeader = ui->tableView->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader->setDefaultSectionSize(
    std::max(this->fontMetrics().height(), this->style()->pixelMetric(QStyle::PM_SmallIconSize)) +
    2 * this->style()->pixelMetric(QStyle::PM_FocusFrameVMargin));

  // setSortingEnabled() sorts by the current sort indicator right away, and a
  // fresh header reports section 0, so the indicator has to be cleared
  // afterwards to leave the messages in arrival order.
  ui->tableView->setSortingEnabled(true);
  header->setSortIndicatorClearable(true);
  header->setSortIndicator(-1, Qt::AscendingOrder);

  ui->filterContent->setClearButtonEnabled(true);
  m_FilterTimer->setSingleShot(true);
  m_FilterTimer->setInterval(FilterDelayInMs);

  for (auto level : SelectableLevels)
  {
    ui->levelFilter->addItem(QtPlatformLogModel::GetLevelIcon(level),
      QtPlatformLogModel::GetLevelName(level) + " and above", static_cast<int>(level));
  }

  // Restore before connecting: both addItem() and setCurrentIndex() emit
  // currentIndexChanged(), which would write preferences during construction.
  ui->levelFilter->setCurrentIndex(ui->levelFilter->findData(static_cast<int>(minimumLevel)));
  ui->showDetails->setChecked(showDetails);

  m_FilterModel->SetMinimumLevel(minimumLevel);
  this->ApplyShowDetails(showDetails);

  connect(m_FilterTimer, &QTimer::timeout, this, &QtLogView::ApplyFilter);
  connect(ui->filterContent, &QLineEdit::textChanged, this, [this] { m_FilterTimer->start(); });
  connect(ui->levelFilter, &QComboBox::currentIndexChanged, this, &QtLogView::OnMinimumLevelChanged);
  connect(ui->showDetails, &QAbstractButton::toggled, this, &QtLogView::OnShowDetailsToggled);
  connect(ui->clear, &QAbstractButton::clicked, this, &QtLogView::OnClearClicked);
  connect(ui->copyToClipboard, &QAbstractButton::clicked, this, &QtLogView::OnCopyToClipboardClicked);
  connect(m_FilterModel, &QAbstractItemModel::rowsInserted, this, &QtLogView::OnRowsInserted);

  // Installed last, so it runs before the event filter the scroll area put on
  // its own viewport and can suppress the default tool tip handling.
  ui->tableView->viewport()->installEventFilter(this);
}

QtLogView::~QtLogView()
{
}

void QtLogView::showEvent(QShowEvent*)
{
  this->SizeColumnsToContents();

  // A log is read from its end. Only on the first show, though: afterwards the
  // scroll position is the user's.
  if (!m_ScrolledToNewest)
  {
    ui->tableView->scrollToBottom();
    m_ScrolledToNewest = true;
  }
}

bool QtLogView::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == ui->tableView->viewport() && QEvent::ToolTip == event->type())
  {
    auto* helpEvent = static_cast<QHelpEvent*>(event);
    const auto index = ui->tableView->indexAt(helpEvent->pos());
    const auto tooltip = index.data(Qt::ToolTipRole).toString();

    // The file column always has the path to add, which its cell never shows.
    const bool isFileColumn = index.isValid() &&
      QtPlatformLogModel::Column::File == static_cast<QtPlatformLogModel::Column>(index.column());

    // Otherwise a tool tip is only worth showing where the cell cannot hold the
    // whole value: it was logged across several lines, or it does not fit the
    // column. One repeating what is already fully readable just gets in the way.
    if (!tooltip.isEmpty() &&
        (isFileColumn || tooltip.contains(QLatin1Char('\n')) || this->IsElided(index)))
    {
      QToolTip::showText(helpEvent->globalPos(), tooltip, ui->tableView->viewport());
    }
    else
    {
      QToolTip::hideText();
    }

    return true;
  }

  return QWidget::eventFilter(watched, event);
}

bool QtLogView::IsElided(const QModelIndex& index) const
{
  const auto text = index.data(Qt::DisplayRole).toString();

  // The delegate insets the text by a focus frame on either side. Being a pixel
  // out only ever means a tool tip on a message that just fits, or none on one
  // clipped by a hair.
  const int margin = 2 * ui->tableView->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);

  return ui->tableView->fontMetrics().horizontalAdvance(text) + margin >
         ui->tableView->columnWidth(index.column());
}

void QtLogView::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  // Message absorbs the width the view gains or loses, the way a stretched last
  // section would, but without giving up its own resize handle.
  this->FillMessageColumn();
}

void QtLogView::SizeColumnsToContents()
{
  const auto& metrics = ui->tableView->fontMetrics();
  const int margin = 4 * metrics.horizontalAdvance(QLatin1Char(' '));
  const int iconWidth = ui->tableView->style()->pixelMetric(QStyle::PM_SmallIconSize);

  for (int column = 0; column < QtPlatformLogModel::ColumnCount; ++column)
  {
    const auto asColumn = static_cast<QtPlatformLogModel::Column>(column);

    // The message column takes whatever the others leave.
    if (asColumn == QtPlatformLogModel::Column::Message || ui->tableView->isColumnHidden(column))
      continue;

    ui->tableView->resizeColumnToContents(column);

    // Sizing to contents only measures the rows sampled so far, so a column
    // whose widest value has not been logged yet ends up truncated. Time and
    // Level have a known widest value; Module, Function and File get room for a
    // typical one, since theirs is far wider than their header.
    int minimumWidth = 0;

    switch (asColumn)
    {
      case QtPlatformLogModel::Column::Time:
        minimumWidth = metrics.horizontalAdvance(QStringLiteral("00:00:00.000"));
        break;

      case QtPlatformLogModel::Column::Level:
        minimumWidth = metrics.horizontalAdvance(QStringLiteral("Warning")) + iconWidth;
        break;

      case QtPlatformLogModel::Column::Module:
      case QtPlatformLogModel::Column::Function:
      case QtPlatformLogModel::Column::File:
        minimumWidth = 28 * metrics.averageCharWidth();
        break;

      default:
        break;
    }

    if (minimumWidth + margin > ui->tableView->columnWidth(column))
      ui->tableView->setColumnWidth(column, minimumWidth + margin);
  }

  this->FillMessageColumn();
}

void QtLogView::FillMessageColumn()
{
  const auto messageColumn = static_cast<int>(QtPlatformLogModel::Column::Message);
  int used = 0;

  for (int column = 0; column < QtPlatformLogModel::ColumnCount; ++column)
  {
    if (column != messageColumn && !ui->tableView->isColumnHidden(column))
      used += ui->tableView->columnWidth(column);
  }

  const int available = ui->tableView->viewport()->width() - used;

  if (available > 0)
    ui->tableView->setColumnWidth(messageColumn, available);
}

void QtLogView::ApplyShowDetails(bool showDetails)
{
  auto* header = ui->tableView->horizontalHeader();

  for (int column = 0; column < QtPlatformLogModel::ColumnCount; ++column)
  {
    const bool hidden = !showDetails &&
      QtPlatformLogModel::IsDetailColumn(static_cast<QtPlatformLogModel::Column>(column));

    ui->tableView->setColumnHidden(column, hidden);

    // Sorting by a column that is no longer visible leaves an order that
    // nothing on screen explains.
    if (hidden && column == m_FilterModel->sortColumn())
      header->setSortIndicator(-1, Qt::AscendingOrder);
  }

  m_FilterModel->SetShowDetails(showDetails);
  this->SizeColumnsToContents();
}

void QtLogView::ApplyFilter()
{
  QRegularExpression regExp(QRegularExpression::escape(ui->filterContent->text()),
    QRegularExpression::CaseInsensitiveOption);

  m_FilterModel->setFilterRegularExpression(regExp);
}

void QtLogView::OnMinimumLevelChanged(int index)
{
  const auto level = static_cast<mitk::LogLevel>(ui->levelFilter->itemData(index).toInt());

  m_FilterModel->SetMinimumLevel(level);

  auto* prefs = GetPreferences();
  prefs->Put("MinimumLevel", QtPlatformLogModel::GetLevelName(level).toStdString());
  prefs->Flush();
}

void QtLogView::OnShowDetailsToggled(bool checked)
{
  this->ApplyShowDetails(checked);

  auto* prefs = GetPreferences();
  prefs->PutBool("ShowDetails", checked);
  prefs->Flush();
}

void QtLogView::OnClearClicked()
{
  m_Model->Clear();
}

void QtLogView::OnRowsAboutToBeInserted()
{
  auto* scrollBar = ui->tableView->verticalScrollBar();
  m_FollowNewEntries = scrollBar->value() == scrollBar->maximum();
}

void QtLogView::OnRowsInserted(const QModelIndex&, int, int last)
{
  // Only follow an entry that actually lands at the end: under a user-chosen
  // sort order a new message can appear anywhere in the table.
  if (m_FollowNewEntries && last == m_FilterModel->rowCount() - 1)
    ui->tableView->scrollTo(m_FilterModel->index(last, 0), QAbstractItemView::PositionAtBottom);
}

void QtLogView::OnCopyToClipboardClicked()
{
  auto* model = ui->tableView->model();
  const int rowCount = model->rowCount();
  const int columnCount = model->columnCount();

  QStringList lines;
  QStringList fields;

  for (int column = 0; column < columnCount; ++column)
  {
    if (!ui->tableView->isColumnHidden(column))
      fields << model->headerData(column, Qt::Horizontal).toString();
  }

  lines << fields.join(QStringLiteral("\t"));

  for (int row = 0; row < rowCount; ++row)
  {
    fields.clear();

    for (int column = 0; column < columnCount; ++column)
    {
      if (!ui->tableView->isColumnHidden(column))
        fields << model->index(row, column).data().toString();
    }

    lines << fields.join(QStringLiteral("\t"));
  }

  QApplication::clipboard()->setText(lines.join(QStringLiteral("\n")));
}

}
