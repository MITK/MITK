/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtPlatformLogModel.h"
#include "berryQtLogPlugin.h"

#include <mitkFileSystem.h>
#include <mitkLogLevel.h>

#include <QDebug>
#include <QModelIndex>
#include <QMutexLocker>
#include <QRegularExpression>

namespace
{
  QString CollapseWhitespace(const std::string& text)
  {
    static const QRegularExpression whitespace(QStringLiteral("\\s+"));
    return QString::fromStdString(text).replace(whitespace, QStringLiteral(" ")).trimmed();
  }

  QString FileNameOf(const std::string& path)
  {
    return QString::fromStdString(fs::path(path).filename().string());
  }

  QString ShortenSourcePath(const std::string& path)
  {
    const auto relative = fs::path(path).lexically_relative(MITK_SOURCE_DIR);

    // Anything not below the source tree, ITK and VTK sources included, comes
    // back empty or reaching upwards and is better off shown as it is.
    return !relative.empty() && ".." != *relative.begin()
      ? QString::fromStdString(relative.string())
      : QString::fromStdString(path);
  }
}

namespace berry {

QtPlatformLogModel::ExtendedLogMessage::ExtendedLogMessage(const mitk::LogMessage& msg)
  : message(msg),
    time(QDateTime::currentDateTime()),
    singleLineMessage(CollapseWhitespace(msg.Message)),
    fileName(FileNameOf(msg.FilePath))
{
}

QVariant QtPlatformLogModel::ExtendedLogMessage::getPath() const
{
  return QVariant(ShortenSourcePath(this->message.FilePath));
}

QVariant QtPlatformLogModel::ExtendedLogMessage::getTime() const
{
  // Same wall clock as the log file, but with the resolution needed to tell
  // messages of the same second apart.
  return QVariant(this->time.toString(QStringLiteral("HH:mm:ss.zzz")));
}

void QtPlatformLogModel::slotFlushLogEntries()
{
  std::deque<ExtendedLogMessage>* pending = nullptr;

  {
    QMutexLocker locker(&m_Mutex);
    std::swap(m_Active, m_Pending);
    m_FlushPending = false;
    pending = m_Pending;
  }

  const int num = static_cast<int>(pending->size());

  if (num > 0)
  {
    const int row = static_cast<int>(m_Entries.size());
    this->beginInsertRows(QModelIndex(), row, row + num - 1);

    for (const auto& entry : *pending)
      m_Entries.push_back(entry);

    pending->clear();
    this->endInsertRows();
  }

  const int excess = static_cast<int>(m_Entries.size()) - MaxEntries;

  if (excess > 0)
  {
    this->beginRemoveRows(QModelIndex(), 0, excess - 1);

    // pop_front() rather than erase(): erase moves the remaining elements, and
    // ExtendedLogMessage is deliberately not assignable.
    for (int i = 0; i < excess; ++i)
      m_Entries.pop_front();

    this->endRemoveRows();
  }
}

void QtPlatformLogModel::addLogEntry(const mitk::LogMessage &msg)
{
  bool flushPending;

  {
    QMutexLocker locker(&m_Mutex);
    m_Active->push_back(ExtendedLogMessage(msg));
    flushPending = m_FlushPending;
    m_FlushPending = true;
  }

  if (!flushPending)
    emit signalFlushLogEntries();
}

void
QtPlatformLogModel::addLogEntry(const ctkPluginFrameworkEvent& event)
{
  auto level = mitk::LogLevel::Info;
  if (event.getType() == ctkPluginFrameworkEvent::PLUGIN_ERROR)
  {
    level = mitk::LogLevel::Error;
  }
  else if (event.getType() == ctkPluginFrameworkEvent::FRAMEWORK_WAIT_TIMEDOUT ||
           event.getType() == ctkPluginFrameworkEvent::PLUGIN_WARNING)
  {
    level = mitk::LogLevel::Warn;
  }

  mitk::LogMessage msg(level,"n/a",-1,"n/a");

  QString str;
  QDebug dbg(&str);
  dbg << event;
  msg.Message = str.toStdString();

  addLogEntry(msg);
}

QtPlatformLogModel::QtPlatformLogModel(QObject* parent)
  : QAbstractTableModel(parent),
    m_FlushPending(false)
{
  m_Active=new std::deque<ExtendedLogMessage>;
  m_Pending=new std::deque<ExtendedLogMessage>;
  connect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ), Qt::QueuedConnection );
  QtLogPlugin::GetInstance()->GetContext()->connectFrameworkListener(this, SLOT(addLogEntry(ctkPluginFrameworkEvent)));
  myBackend = new QtLogBackend(this);
}

QtPlatformLogModel::~QtPlatformLogModel()
{
  disconnect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ));
  QtLogPlugin::GetInstance()->GetContext()->disconnectFrameworkListener(this);

  // dont delete and unregister backend, only deactivate it to avoid thread synchronization issues cause mitk::UnregisterBackend is not threadsafe
  // will be fixed.
  //  delete myBackend;
  //  delete m_Active;
  //  delete m_Pending;
  m_Mutex.lock();
  myBackend->Deactivate();
  m_Mutex.unlock();
}

void QtPlatformLogModel::Clear()
{
  this->beginResetModel();

  {
    QMutexLocker locker(&m_Mutex);
    m_Active->clear();
    m_Pending->clear();
  }

  m_Entries.clear();

  this->endResetModel();
}

int QtPlatformLogModel::GetSeverityRank(mitk::LogLevel level)
{
  switch (level)
  {
    case mitk::LogLevel::Debug:
      return 0;

    case mitk::LogLevel::Info:
      return 1;

    case mitk::LogLevel::Warn:
      return 2;

    case mitk::LogLevel::Error:
      return 3;

    case mitk::LogLevel::Fatal:
      return 4;
  }

  return 0;
}

QString QtPlatformLogModel::GetLevelName(mitk::LogLevel level)
{
  switch (level)
  {
    case mitk::LogLevel::Debug:
      return QStringLiteral("Debug");

    case mitk::LogLevel::Info:
      return QStringLiteral("Info");

    case mitk::LogLevel::Warn:
      return QStringLiteral("Warning");

    case mitk::LogLevel::Error:
      return QStringLiteral("Error");

    case mitk::LogLevel::Fatal:
      return QStringLiteral("Fatal");
  }

  return QString();
}

const QIcon& QtPlatformLogModel::GetLevelIcon(mitk::LogLevel level)
{
  // Built on first use, as the resource system and the GUI application are not
  // necessarily up when this translation unit is initialized.
  static const QIcon debug(QStringLiteral(":/org_blueberry_ui_qt_log/debug.png"));
  static const QIcon info(QStringLiteral(":/org_blueberry_ui_qt_log/information.png"));
  static const QIcon warning(QStringLiteral(":/org_blueberry_ui_qt_log/warning.png"));
  static const QIcon error(QStringLiteral(":/org_blueberry_ui_qt_log/error.png"));
  static const QIcon fatal(QStringLiteral(":/org_blueberry_ui_qt_log/fatal.png"));

  switch (level)
  {
    case mitk::LogLevel::Debug:
      return debug;

    case mitk::LogLevel::Info:
      return info;

    case mitk::LogLevel::Warn:
      return warning;

    case mitk::LogLevel::Error:
      return error;

    case mitk::LogLevel::Fatal:
      return fatal;
  }

  return info;
}

bool QtPlatformLogModel::IsDetailColumn(Column column)
{
  switch (column)
  {
    case Column::Level:
    case Column::Message:
      return false;

    case Column::Time:
    case Column::Module:
    case Column::Function:
    case Column::File:
    case Column::Line:
      return true;
  }

  return true;
}

int QtPlatformLogModel::rowCount(const QModelIndex&) const
{
  return static_cast<int>(m_Entries.size());
}

int QtPlatformLogModel::columnCount(const QModelIndex&) const
{
  return ColumnCount;
}

QVariant QtPlatformLogModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() >= static_cast<int>(m_Entries.size()) ||
      index.column() < 0 || index.column() >= ColumnCount)
  {
    return QVariant();
  }

  const auto& entry = m_Entries[index.row()];

  if (LogLevelRole == role)
    return static_cast<int>(entry.message.Level);

  const auto column = static_cast<Column>(index.column());

  switch (role)
  {
    case Qt::DisplayRole:
      switch (column)
      {
        case Column::Time:
          return entry.getTime();

        case Column::Level:
          return GetLevelName(entry.message.Level);

        case Column::Message:
          return entry.singleLineMessage;

        case Column::Module:
          return entry.getModuleName();

        case Column::Function:
          return entry.getFunctionName();

        case Column::File:
          return entry.fileName;

        case Column::Line:
          return entry.getLine();
      }
      break;

    case Qt::DecorationRole:
      if (Column::Level == column)
        return GetLevelIcon(entry.message.Level);
      break;

    case Qt::ToolTipRole:
      // Whether a tool tip is worth showing depends on the column width, which
      // only the view knows, so the view decides.
      switch (column)
      {
        case Column::Message:
          return entry.getMessage();

        case Column::File:
          return entry.getPath();

        default:
          break;
      }
      break;

    case SortRole:
      switch (column)
      {
        case Column::Time:
          return entry.time;

        case Column::Level:
          return GetSeverityRank(entry.message.Level);

        case Column::Line:
          return entry.message.LineNumber;

        default:
          return this->data(index, Qt::DisplayRole);
      }
  }

  return QVariant();
}

QVariant QtPlatformLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::DisplayRole != role || Qt::Horizontal != orientation ||
      section < 0 || section >= ColumnCount)
  {
    return QVariant();
  }

  switch (static_cast<Column>(section))
  {
    case Column::Time:
      return QStringLiteral("Time");

    case Column::Level:
      return QStringLiteral("Level");

    case Column::Message:
      return QStringLiteral("Message");

    case Column::Module:
      return QStringLiteral("Module");

    case Column::Function:
      return QStringLiteral("Function");

    case Column::File:
      return QStringLiteral("File");

    case Column::Line:
      return QStringLiteral("Line");
  }

  return QVariant();
}

}
