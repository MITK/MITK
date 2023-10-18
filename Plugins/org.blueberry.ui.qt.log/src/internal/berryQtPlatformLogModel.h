/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTPLATFORMLOGMODEL_H_
#define BERRYQTPLATFORMLOGMODEL_H_

#include "berryLog.h"

#include "ctkPluginFrameworkEvent.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDateTime>

#include <ctime>
#include <sstream>

#include "berryLog.h"

#include <QMutex>


namespace berry {

/** Documentation
 *  @brief An object of this class represents a table of logging data.
 *         The table presentation can be modified by the methods
 *         SetShowAdvancedFiels() and SetShowCategory().
 */
class QtPlatformLogModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  QtPlatformLogModel(QObject* parent = nullptr);
  ~QtPlatformLogModel() override;

  void SetShowAdvancedFiels( bool showAdvancedFiels );
  void SetShowCategory( bool showCategory );
  int rowCount(const QModelIndex&) const override;
  int columnCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int) const override;

  /** Documentation
   *  @return Returns the complete table data as string representation.
   */
  QString GetDataAsString();

  QVariant headerData(int section, Qt::Orientation orientation, int) const override;

  void addLogEntry(const mitk::LogMessage &msg);

  Q_SLOT void addLogEntry(const ctkPluginFrameworkEvent& event);

private:
  bool m_ShowAdvancedFiels;
  bool m_ShowCategory;

  /** Documentation
   *  @brief An object of this struct internally represents a logging message.
   *         It offers methods to convert the logging data into QVaraint objects
   *         and also adds time and threadid as logging data. The struct is
   *         internally used to store logging data in the table data model.
   */
  struct ExtendedLogMessage {
    mitk::LogMessage message;
    clock_t time;
    int threadid;

    ExtendedLogMessage(const ExtendedLogMessage &src):message(src.message),time(src.time),threadid(src.threadid)
    {
    }

    ExtendedLogMessage(const mitk::LogMessage &msg):message(msg),time(std::clock()),threadid(0)
    {
    }

    ExtendedLogMessage operator = (const ExtendedLogMessage& src)
    {
      return ExtendedLogMessage(src);
    }

    QVariant getLevel() const
    {
    switch(this->message.Level)
        {
          default:
          case mitk::LogLevel::Info:
            return QVariant(Info);

          case mitk::LogLevel::Warn:
            return QVariant(Warn);

          case mitk::LogLevel::Error:
            return QVariant(Error);

          case mitk::LogLevel::Fatal:
            return QVariant(Fatal);

          case mitk::LogLevel::Debug:
            return QVariant(Debug);
        }
    }

    QVariant getMessage() const
    {
    return QVariant(QString::fromStdString(this->message.Message));
    }

    QVariant getCategory() const
    {
    return QVariant(QString::fromStdString(this->message.Category));
    }

    QVariant getModuleName() const
    {
    return QVariant(QString::fromStdString(this->message.ModuleName));
    }

    QVariant getFunctionName() const
    {
    return QVariant(QString::fromStdString(this->message.FunctionName));
    }

    QVariant getPath() const
    {
    return QVariant(QString::fromStdString(this->message.FilePath));
    }

    QVariant getLine() const
    {
    return QVariant(QString::number(this->message.LineNumber));
    }

    /** This method is implemented in the cpp file to save includes. */
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

  QList<ExtendedLogMessage> m_Entries;
  QList<ExtendedLogMessage> *m_Active,*m_Pending;

  static const QString Error;
  static const QString Warn;
  static const QString Fatal;
  static const QString Info;
  static const QString Debug;

  QMutex m_Mutex;

  signals:

    void signalFlushLogEntries();

  protected slots:

    void slotFlushLogEntries();
};

}

#endif /*BERRYQTPLATFORMLOGMODEL_H_*/
