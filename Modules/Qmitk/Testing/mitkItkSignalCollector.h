/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __mitkItkSignalCollector_h
#define __mitkItkSignalCollector_h

#include <itkCommand.h>
#include <itkEventObject.h>

#include <mitkCommon.h>

#include <vector>

namespace mitk
{

/// \class ItkSignalListener
///
/// \brief Abstract class to be implemented by ITK signal listeners.
class ItkSignalListener
{
public:
  virtual void OnItkSignalReceived(const itk::Object* object, const itk::EventObject& event) = 0;
};


/// \class ItkSignalCollector
///
/// \brief Class for collecting ITK signals and sending notifications of them to registered listeners.
class ItkSignalCollector : public itk::Command
{
public:
  mitkClassMacro(ItkSignalCollector, itk::Command);
  itkNewMacro(ItkSignalCollector);

  typedef std::pair<const itk::Object*, itk::EventObject*> Signal;
  typedef std::vector<Signal> Signals;

  /// \brief Connects this object to the events of the given object.
  /// The current object will collect the given type of signals of that object.
  void Connect(itk::Object* object, const itk::EventObject& event);

  /// \brief Adds a listener that will get notified of the ITK signals that this object is connected to.
  void AddListener(ItkSignalListener* listener);

  /// \brief Removes a listener. The listener will not get notified about the ITK signals observed by
  /// the current object, any more.
  void RemoveListener(ItkSignalListener* listener);

  /// \brief Gets the signals collected by this object.
  const Signals& GetSignals() const;

  /// \brief Returns a set of the collected ITK signals that are of the given type or its subtype.
  Signals GetSignals(const itk::EventObject& event) const;

  /// \brief Returns a set of the collected ITK signals that are sent from the given object,
  /// and are of the given type or its subtype.
  Signals GetSignals(const itk::Object* object, const itk::EventObject& event = itk::AnyEvent()) const;

  /// \brief Clears all the signals collected by now.
  virtual void Clear();

  /// \brief Adds the object-event pair to the list of collected signals.
  virtual void ProcessEvent(const itk::Object* object, const itk::EventObject& event);

protected:

  /// \brief Constructs an ItkSignalCollector object.
  ItkSignalCollector();

  /// \brief Destructs an ItkSignalCollector object.
  virtual ~ItkSignalCollector();

  /// \brief Prints the collected signals to the given stream or to the standard output if no stream is given.
  virtual void PrintSelf(std::ostream & os, itk::Indent indent) const;

private:

  /// \brief Called when the event happens to the caller.
  virtual void Execute(itk::Object* caller, const itk::EventObject& event);

  /// \brief Called when the event happens to the caller.
  virtual void Execute(const itk::Object* object, const itk::EventObject& event);

  typedef std::multimap<itk::Object::Pointer, unsigned long> ObserverMap;
  ObserverMap m_ObserverTags;

  /// \brief The signals collected by this object.
  Signals m_Signals;

  /// \brief The listeners of the ITK signals.
  std::vector<ItkSignalListener*> m_Listeners;

};

}

#endif
