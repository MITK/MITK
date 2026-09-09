/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTPLATFORMLOGMODEL_H_
#define BERRYQTPLATFORMLOGMODEL_H_

#include <berryLog.h>

#include <ctkPluginFrameworkEvent.h>

#include <QAbstractTableModel>
#include <QDateTime>
#include <QIcon>
#include <QMutex>

#include <deque>

namespace berry {

/** \brief A table of the log messages distributed to the MITK log backends.
 *
 * The model always exposes all columns of Column. Which of them are shown is up
 * to the view: a single model instance is shared by every log view, so column
 * visibility cannot be model state.
 */
class QtPlatformLogModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  /** \brief The fields of a log message, in display order.
   *
   * There is deliberately no Count enumerator so that switches over this enum
   * can omit the default label and adding a field is caught at compile time.
   * ColumnCount takes its place, which is why Line has to stay last.
   */
  enum class Column
  {
    Time,
    Level,
    Message,
    Module,
    Function,
    File,
    Line
  };

  static constexpr int ColumnCount = static_cast<int>(Column::Line) + 1;

  /** \brief The number of messages kept before the oldest ones are dropped.
   *
   * Without a limit the table grows for as long as the application runs.
   */
  static constexpr int MaxEntries = 10000;

  /** \brief Custom item data roles.
   *
   * LogLevelRole is answered by every column so that filtering by level does not
   * depend on where the level column sits. SortRole yields naturally ordered
   * values for the columns whose display string sorts wrongly.
   */
  enum Role
  {
    LogLevelRole = Qt::UserRole,
    SortRole
  };

  QtPlatformLogModel(QObject* parent = nullptr);
  ~QtPlatformLogModel() override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /** \brief Discard all messages recorded so far. */
  void Clear();

  void addLogEntry(const mitk::LogMessage& msg);

  Q_SLOT void addLogEntry(const ctkPluginFrameworkEvent& event);

  /** \brief The severity rank of a log level, ascending.
   *
   * Debug < Info < Warn < Error < Fatal. The declaration order of mitk::LogLevel
   * is not its severity order, so the levels cannot be compared directly.
   */
  static int GetSeverityRank(mitk::LogLevel level);

  static QString GetLevelName(mitk::LogLevel level);
  static const QIcon& GetLevelIcon(mitk::LogLevel level);

  /** \brief Whether a column carries information worth showing only on demand.
   *
   * The view hides these columns and the filter proxy excludes them from the
   * text search, so both follow one definition.
   */
  static bool IsDetailColumn(Column column);

private:

  /** \brief A log message plus the arrival data the log mechanism does not carry.
   */
  struct ExtendedLogMessage
  {
    mitk::LogMessage message;
    QDateTime time;

    /** \brief The message with every run of whitespace collapsed to a space.
     *
     * Messages forwarded from ITK or VTK and exception descriptions span several
     * lines, which a single table row cannot show. Cached rather than derived on
     * demand because it is read on every repaint and on every keystroke in the
     * filter.
     */
    QString singleLineMessage;

    /** \brief The file the message came from, without its directories.
     *
     * The full path is far wider than the column and the same for every message
     * from one file, so the file column shows only this and offers the path as a
     * tool tip.
     */
    QString fileName;

    /** Implemented in the cpp file to save includes. */
    ExtendedLogMessage(const mitk::LogMessage& msg);

    /** mitk::LogMessage has const members, so it is copyable but not assignable.
     *  Spelling that out keeps a silently discarded assignment from compiling;
     *  it is also why the entries are kept in a std::deque rather than a QList,
     *  as QList requires an assignable element type.
     */
    ExtendedLogMessage& operator=(const ExtendedLogMessage& src) = delete;

    /** Declaring the assignment operator suppresses the implicit move
     *  constructor, and every entry is moved once, out of the queue the log
     *  backend fills and into the table. Declaring the move constructor in turn
     *  suppresses the implicit copy constructor, hence both.
     */
    ExtendedLogMessage(const ExtendedLogMessage& src) = default;
    ExtendedLogMessage(ExtendedLogMessage&& src) = default;

    QVariant getMessage() const
    {
      return QVariant(QString::fromStdString(this->message.Message));
    }

    QVariant getModuleName() const
    {
      return QVariant(QString::fromStdString(this->message.ModuleName));
    }

    QVariant getFunctionName() const
    {
      return QVariant(QString::fromStdString(this->message.FunctionName));
    }

    /** \brief The origin of the message, relative to the MITK source tree.
     *
     * Implemented in the cpp file to save includes.
     */
    QVariant getPath() const;

    QVariant getLine() const
    {
      return QVariant(QString::number(this->message.LineNumber));
    }

    QVariant getTime() const;
  };

  class QtLogBackend : public mitk::LogBackendBase
  {
    public:

      QtLogBackend(QtPlatformLogModel *_myModel)
      {
        myModel=_myModel;
        deactivated = false;
        mitk::RegisterBackend(this);
        BERRY_INFO << "BlueBerry log backend registered";
      }

      ~QtLogBackend() override
      {
        mitk::UnregisterBackend(this);
      }

      void ProcessMessage(const mitk::LogMessage &l ) override
      {
        if(!deactivated)
          myModel->addLogEntry(l);
      }

      mitk::LogBackendBase::OutputType GetOutputType() const override
      {
        return mitk::LogBackendBase::OutputType::Other;
      }

      void Deactivate()
      {
        deactivated=true;
      }

    private:

      QtPlatformLogModel *myModel;
      bool deactivated;

  } *myBackend;

  std::deque<ExtendedLogMessage> m_Entries;
  std::deque<ExtendedLogMessage> *m_Active,*m_Pending;

  /** Whether a flush is already on the event queue. Messages arriving while one
   *  is pending are picked up by it rather than queueing an event of their own.
   */
  bool m_FlushPending;

  QMutex m_Mutex;

  signals:

    void signalFlushLogEntries();

  protected slots:

    void slotFlushLogEntries();
};

}

#endif /*BERRYQTPLATFORMLOGMODEL_H_*/
