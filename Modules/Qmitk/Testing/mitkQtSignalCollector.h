/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __mitkQtSignalCollector_h
#define __mitkQtSignalCollector_h

#include <QObject>

#include <mitkCommon.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

#include <vector>

namespace mitk
{

/// \class QtSignalListener
///
/// \brief Abstract class to be implemented by Qt signal listeners.
class QtSignalListener
{
public:
  virtual void OnQtSignalReceived(const QObject* object, const char* signal) = 0;
};


/// \class QtSignalNotifier
///
/// \brief Helper class that implements a call back mechanism for Qt signals.
class QtSignalNotifier : public QObject
{
  Q_OBJECT

public:

  QtSignalNotifier(QtSignalListener* signalListener, const QObject* object, const char* signal);

  virtual ~QtSignalNotifier();

private slots:

  virtual void OnQtSignalReceived();

private:

  QtSignalListener* m_QtSignalListener;
  const QObject* m_Object;
  QByteArray m_Signal;
};


/// \class QtSignalCollector
///
/// \brief Class for collecting Qt signals and sending notifications of them to registered listeners.
class QtSignalCollector : public QObject, public itk::Object, private QtSignalListener
{
  Q_OBJECT

public:
  mitkClassMacro(QtSignalCollector, QObject);
  itkNewMacro(QtSignalCollector);

  typedef std::pair<const QObject*, QByteArray> Signal;
  typedef std::vector<Signal> Signals;

  /// \brief Connects this object to the signals of the given object.
  /// The current object will collect the given signals of that object.
  void Connect(const QObject* object, const char* signal = 0);

  /// \brief Adds a listener that will get notified of the Qt signals that this object is connected to.
  void AddListener(QtSignalListener* listener);

  /// \brief Removes a listener. The listener will not get notified about the Qt signals observed by
  /// the current object, any more.
  void RemoveListener(QtSignalListener* listener);

  /// \brief Gets the signals collected by this object.
  const Signals& GetSignals() const;

  /// \brief Returns a set of the collected Qt signals that are of the given type.
  Signals GetSignals(const char* signal) const;

  /// \brief Returns a set of the collected Qt signals that are sent from the given object,
  /// and are of the given type.
  Signals GetSignals(const QObject* object, const char* signal = 0) const;

  /// \brief Clears all the signals collected by now.
  virtual void Clear();

protected:

  /// \brief Constructs an QtSignalCollector object.
  QtSignalCollector();

  /// \brief Destructs an QtSignalCollector object.
  virtual ~QtSignalCollector();

  /// \brief Prints the collected signals to the given stream or to the standard output if no stream is given.
  virtual void PrintSelf(std::ostream & os, itk::Indent indent) const;

  /// \brief Adds the object-signal pair to the list of collected signals.
  virtual void OnQtSignalReceived(const QObject* object, const char* signal);

private:

  /// \brief The signals collected by this object.
  Signals m_Signals;

  /// \brief The signals collected by this object.
  std::vector<QtSignalNotifier*> m_SignalNotifiers;

  /// \brief The listeners of the ITK signals.
  std::vector<QtSignalListener*> m_Listeners;

};

}

#endif
