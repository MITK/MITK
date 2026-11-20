/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPulseTimers_h
#define QmitkPulseTimers_h

#include <MitkMeasurementRESTExports.h>

#include <mitkDataNode.h>
#include <mitkWeakPointer.h>

#include <QObject>
#include <QPointer>
#include <QTimer>

#include <chrono>
#include <map>

/**
 * \brief Ends pulses of figures after their duration, with one timer per figure.
 *
 * Lives in the GUI thread, as a child of the object that it was created with.
 * A pulse is the boolean property "animated.pulse" of the figure's node.
 */
class MITKMEASUREMENTREST_EXPORT QmitkPulseTimers : public QObject
{
public:
  explicit QmitkPulseTimers(QObject* parent);

  /** Ends the pulses that are still timed at once, as no timer would end them anymore. */
  ~QmitkPulseTimers() override;

  /** Switches the pulse of a figure off after the duration, instead of when an earlier call said. */
  void EndPulseAfter(mitk::DataNode* node, std::chrono::milliseconds duration);

  /** Keeps the pulse of a figure as it is, without an end that an earlier call set. */
  void Cancel(const mitk::DataNode* node);

private:
  struct TimedPulse
  {
    QPointer<QTimer> Timer;

    /** The figure, which may be deleted before its pulse ends; another node may get its address then. */
    mitk::WeakPointer<mitk::DataNode> Node;
  };

  std::map<const mitk::DataNode*, TimedPulse> m_Pulses;
};

#endif
