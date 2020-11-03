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
   *  @return Retruns the complete table data as string representation.
   */
  QString GetDataAsString();

  QVariant headerData(int section, Qt::Orientation orientation, int) const override;

  void addLogEntry(const mbilog::LogMessage &msg);

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
    mbilog::LogMessage message;
    clock_t time;
    int threadid;

    ExtendedLogMessage(const ExtendedLogMessage &src):message(src.message),time(src.time),threadid(src.threadid)
    {
    }

    ExtendedLogMessage(const mbilog::LogMessage &msg):message(msg),time(std::clock()),threadid(0)
    {
    }

    ExtendedLogMessage operator = (const ExtendedLogMessage& src)
    {
      return ExtendedLogMessage(src);
    }

    QVariant getLevel() const
    {
    switch(this->message.level)
        {
          default:
          case mbilog::Info:
            return QVariant(Info);

          case mbilog::Warn:
            return QVariant(Warn);

          case mbilog::Error:
            return QVariant(Error);

          case mbilog::Fatal:
            return QVariant(Fatal);

          case mbilog::Debug:
            return QVariant(Debug);
        }
    }

    QVariant getMessage() const
    {
    return QVariant(QString(this->message.message.c_str()));
    }

    QVariant getCategory() const
    {
    return QVariant(QString(this->message.category.c_str()));
    }

    QVariant getModuleName() const
    {
    return QVariant(QString(this->message.moduleName));
    }

    QVariant getFunctionName() const
    {
    return QVariant(QString(this->message.functionName));
    }

    QVariant getPath() const
    {
    return QVariant(QString(this->message.filePath));
    }

    QVariant getLine() const
    {
    std::stringstream out;
    std::locale C("C");
    out.imbue(C);
    out << this->message.lineNumber;
    return QVariant(QString(out.str().c_str()));
    }

    /** This method is implemented in the cpp file to save includes. */
    QVariant getTime() const;

  };

  class QtLogBackend : public mbilog::BackendBase
  {
    public:

      QtLogBackend(QtPlatformLogModel *_myModel)
      {
        myModel=_myModel;
        deactivated = false;
        mbilog::RegisterBackend(this);
        BERRY_INFO << "BlueBerry mbilog backend registered";
      }

      ~QtLogBackend() override
      {
        mbilog::UnregisterBackend(this);
      }

      void ProcessMessage(const mbilog::LogMessage &l ) override
      {
        if(!deactivated)
          myModel->addLogEntry(l);
      }

      mbilog::OutputType GetOutputType() const override
      {
        return mbilog::Other;
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
