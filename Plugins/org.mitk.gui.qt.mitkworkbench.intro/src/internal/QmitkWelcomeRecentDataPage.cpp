/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomeRecentDataPage.h"

#include <mitkWorkbenchUtil.h>

#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPromise>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QThreadPool>
#include <QVBoxLayout>

#include <memory>

namespace
{
  constexpr int PATH_ROLE = Qt::UserRole;
  constexpr int FOLDER_ROLE = Qt::UserRole + 1;
  constexpr int MARGIN = 6;

  QFont GetFolderFont(const QFont& font)
  {
    QFont folderFont(font);

    if (font.pixelSize() > 0)
    {
      folderFont.setPixelSize(font.pixelSize() * 7 / 8);
    }
    else
    {
      folderFont.setPointSizeF(font.pointSizeF() * 7 / 8);
    }

    return folderFont;
  }

  // Draws the file name above its folder, which is elided in the middle so
  // that both the drive or root and the innermost folders stay visible.
  class RecentDataItemDelegate : public QStyledItemDelegate
  {
  public:
    RecentDataItemDelegate(const QmitkWelcomePalette& palette, QObject* parent)
      : QStyledItemDelegate(parent),
        m_Palette(palette)
    {
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      const bool isAvailable = index.flags().testFlag(Qt::ItemIsEnabled);
      const QFontMetrics nameMetrics(option.font);
      const QFont folderFont = GetFolderFont(option.font);
      const QFontMetrics folderMetrics(folderFont);
      const QRect rect = option.rect.adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN);

      QString name = index.data(Qt::DisplayRole).toString();

      if (!isAvailable)
        name += " (not found)";

      painter->save();

      if (isAvailable && option.state.testFlag(QStyle::State_MouseOver))
        painter->fillRect(option.rect, m_Palette.Cell);

      painter->setFont(option.font);
      painter->setPen(isAvailable ? m_Palette.Text : m_Palette.MutedText);
      painter->drawText(QRect(rect.left(), rect.top(), rect.width(), nameMetrics.height()),
        Qt::AlignLeft | Qt::AlignVCenter, nameMetrics.elidedText(name, Qt::ElideRight, rect.width()));

      painter->setFont(folderFont);
      painter->setPen(m_Palette.MutedText);
      painter->drawText(QRect(rect.left(), rect.top() + nameMetrics.height(), rect.width(), folderMetrics.height()),
        Qt::AlignLeft | Qt::AlignVCenter, folderMetrics.elidedText(index.data(FOLDER_ROLE).toString(), Qt::ElideMiddle, rect.width()));

      painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex&) const override
    {
      const QFontMetrics nameMetrics(option.font);
      const QFontMetrics folderMetrics(GetFolderFont(option.font));

      return QSize(0, nameMetrics.height() + folderMetrics.height() + 2 * MARGIN);
    }

  private:
    const QmitkWelcomePalette& m_Palette;
  };
}

QmitkWelcomeRecentDataPage::QmitkWelcomeRecentDataPage(berry::IWorkbenchWindow::Pointer window, const QmitkWelcomePalette& palette, QWidget* parent)
  : QWidget(parent),
    m_Window(window),
    m_Palette(palette),
    m_Projects{ mitk::RecentData::Kind::Project, nullptr, nullptr, nullptr },
    m_Files{ mitk::RecentData::Kind::File, nullptr, nullptr, nullptr },
    m_IsAvailabilityCheckPending(false)
{
  auto* cards = new QHBoxLayout;
  cards->setSpacing(16);
  cards->addWidget(this->CreateCard("Recent projects", "Projects you open or save appear here.", m_Projects), 1);
  cards->addWidget(this->CreateCard("Recent files", "Files you open appear here.", m_Files), 1);

  // The tabs are as high as their highest page, which the cards would fill.
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addLayout(cards);
  layout->addStretch(1);

  connect(&m_AvailabilityCheck, &QFutureWatcherBase::finished, this, &QmitkWelcomeRecentDataPage::OnAvailabilityChecked);

  mitk::RecentData::OnChanged().AddListener(
    mitk::MessageDelegate<QmitkWelcomeRecentDataPage>(this, &QmitkWelcomeRecentDataPage::OnRecentDataChanged));
}

QmitkWelcomeRecentDataPage::~QmitkWelcomeRecentDataPage()
{
  mitk::RecentData::OnChanged().RemoveListener(
    mitk::MessageDelegate<QmitkWelcomeRecentDataPage>(this, &QmitkWelcomeRecentDataPage::OnRecentDataChanged));
}

void QmitkWelcomeRecentDataPage::SetPalette(const QmitkWelcomePalette& palette)
{
  m_Palette = palette;

  m_Projects.List->viewport()->update();
  m_Files.List->viewport()->update();
}

void QmitkWelcomeRecentDataPage::showEvent(QShowEvent* event)
{
  // Catch up on recent data that changed while the page was hidden, and on
  // files that appeared or vanished in the meantime.
  this->UpdateLists();
  QWidget::showEvent(event);
}

QWidget* QmitkWelcomeRecentDataPage::CreateCard(const QString& title, const QString& hint, RecentList& recentList)
{
  auto* card = new QWidget;
  card->setObjectName("welcomeCard");
  card->setAttribute(Qt::WA_StyledBackground, true);

  auto* titleLabel = new QLabel(title);
  titleLabel->setObjectName("cardTitle");

  recentList.ClearButton = new QPushButton("Clear");
  recentList.ClearButton->setObjectName("clearButton");
  recentList.ClearButton->setCursor(Qt::PointingHandCursor);
  recentList.ClearButton->setToolTip("Clear the list");

  const auto kind = recentList.Kind;

  connect(recentList.ClearButton, &QPushButton::clicked, this, [kind]()
  {
    mitk::RecentData::Clear(kind);
  });

  auto* titleRow = new QHBoxLayout;
  titleRow->addWidget(titleLabel, 1);
  titleRow->addWidget(recentList.ClearButton);

  recentList.List = new QListWidget;
  recentList.List->setItemDelegate(new RecentDataItemDelegate(m_Palette, recentList.List));
  recentList.List->setSelectionMode(QAbstractItemView::NoSelection);
  recentList.List->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  recentList.List->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  recentList.List->setFrameShape(QFrame::NoFrame);
  recentList.List->setMouseTracking(true);
  recentList.List->viewport()->setAttribute(Qt::WA_Hover);
  recentList.List->setCursor(Qt::PointingHandCursor);
  recentList.List->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(recentList.List, &QListWidget::itemClicked, this, &QmitkWelcomeRecentDataPage::OnItemClicked);

  auto* list = recentList.List;

  connect(list, &QListWidget::customContextMenuRequested, this, [this, list](const QPoint& pos)
  {
    this->OnContextMenuRequested(list, pos);
  });

  recentList.Hint = new QLabel(hint);
  recentList.Hint->setObjectName("mutedLabel");
  recentList.Hint->setWordWrap(true);

  auto* layout = new QVBoxLayout(card);
  layout->setContentsMargins(16, 12, 16, 16);
  layout->setSpacing(8);
  layout->addLayout(titleRow);
  layout->addWidget(recentList.List);
  layout->addWidget(recentList.Hint);
  layout->addStretch(1);

  return card;
}

void QmitkWelcomeRecentDataPage::UpdateList(const RecentList& recentList)
{
  const auto paths = mitk::RecentData::Get(recentList.Kind);

  recentList.List->clear();

  for (const auto& path : paths)
  {
    const QFileInfo fileInfo(path);

    auto* item = new QListWidgetItem(fileInfo.fileName());
    item->setData(PATH_ROLE, path);
    item->setData(FOLDER_ROLE, QDir::toNativeSeparators(fileInfo.absolutePath()));
    item->setToolTip(QDir::toNativeSeparators(path));

    recentList.List->addItem(item);
  }

  this->UpdateAvailability(recentList);

  recentList.List->setVisible(!paths.isEmpty());
  recentList.Hint->setVisible(paths.isEmpty());
  recentList.ClearButton->setEnabled(!paths.isEmpty());
}

void QmitkWelcomeRecentDataPage::UpdateLists()
{
  this->UpdateList(m_Projects);
  this->UpdateList(m_Files);
  this->CheckAvailability();
}

void QmitkWelcomeRecentDataPage::UpdateAvailability(const RecentList& recentList)
{
  // Entries count as available until a check finds them missing.
  for (int i = 0; i < recentList.List->count(); ++i)
  {
    auto* item = recentList.List->item(i);
    const bool isAvailable = !m_MissingPaths.contains(item->data(PATH_ROLE).toString());
    item->setFlags(isAvailable ? Qt::ItemIsEnabled : Qt::NoItemFlags);
  }
}

void QmitkWelcomeRecentDataPage::CheckAvailability()
{
  // A check of an unreachable network path may block for a long time, so
  // further checks wait for it instead of tying up more pool threads.
  if (m_AvailabilityCheck.isRunning())
  {
    m_IsAvailabilityCheckPending = true;
    return;
  }

  const auto paths = mitk::RecentData::Get(mitk::RecentData::Kind::Project) + mitk::RecentData::Get(mitk::RecentData::Kind::File);

  // Started here, so that the check counts as running before the thread pool
  // picks it up.
  auto promise = std::make_shared<QPromise<QSet<QString>>>();
  promise->start();
  m_AvailabilityCheck.setFuture(promise->future());

  QThreadPool::globalInstance()->start([promise, paths]()
  {
    QSet<QString> missingPaths;

    for (const auto& path : paths)
    {
      if (!QFileInfo::exists(path))
        missingPaths.insert(path);
    }

    promise->addResult(missingPaths);
    promise->finish();
  });
}

void QmitkWelcomeRecentDataPage::OnItemClicked(const QListWidgetItem* item)
{
  if (item == nullptr || !item->flags().testFlag(Qt::ItemIsEnabled))
    return;

  auto window = m_Window.Lock();

  if (window.IsNull())
    return;

  mitk::WorkbenchUtil::LoadFiles({ item->data(PATH_ROLE).toString() }, window);
}

void QmitkWelcomeRecentDataPage::OnContextMenuRequested(QListWidget* list, const QPoint& pos)
{
  const auto* item = list->itemAt(pos);

  if (item == nullptr)
    return;

  const QString path = item->data(PATH_ROLE).toString();

  QMenu menu(this);

  connect(menu.addAction("Remove from list"), &QAction::triggered, this, [path]()
  {
    mitk::RecentData::Remove(path);
  });

  menu.exec(list->viewport()->mapToGlobal(pos));
}

void QmitkWelcomeRecentDataPage::OnRecentDataChanged()
{
  // Hidden pages catch up in showEvent(). The update is queued, since clicking
  // an entry changes the recent data while its list still handles the click.
  if (this->isVisible())
    QMetaObject::invokeMethod(this, &QmitkWelcomeRecentDataPage::UpdateLists, Qt::QueuedConnection);
}

void QmitkWelcomeRecentDataPage::OnAvailabilityChecked()
{
  if (m_AvailabilityCheck.future().resultCount() > 0)
  {
    m_MissingPaths = m_AvailabilityCheck.result();

    this->UpdateAvailability(m_Projects);
    this->UpdateAvailability(m_Files);
  }

  if (m_IsAvailabilityCheckPending)
  {
    m_IsAvailabilityCheckPending = false;
    this->CheckAvailability();
  }
}
