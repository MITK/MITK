/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkPulseTimers.h>

#include <mitkRenderingManager.h>

namespace
{
  constexpr const char* PulseProperty = "animated.pulse";
}

QmitkPulseTimers::QmitkPulseTimers(QObject* parent)
  : QObject(parent)
{
}

QmitkPulseTimers::~QmitkPulseTimers()
{
  bool hasEnded = false;

  for (const auto& entry : m_Pulses)
  {
    if (auto lockedNode = entry.second.Node.Lock(); lockedNode.IsNotNull())
    {
      lockedNode->SetBoolProperty(PulseProperty, false);
      hasEnded = true;
    }
  }

  if (hasEnded)
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPulseTimers::EndPulseAfter(mitk::DataNode* node, std::chrono::milliseconds duration)
{
  this->Cancel(node);

  auto* timer = new QTimer(this);
  timer->setSingleShot(true);
  m_Pulses[node] = { timer, node };

  // A later call for the figure deletes the timer, so the pulse found here is the one it times.
  connect(timer, &QTimer::timeout, this, [this, node]() {
    const auto it = m_Pulses.find(node);

    if (it == m_Pulses.end())
      return;

    const auto lockedNode = it->second.Node.Lock();
    it->second.Timer->deleteLater();
    m_Pulses.erase(it);

    if (lockedNode.IsNotNull())
    {
      lockedNode->SetBoolProperty(PulseProperty, false);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  });

  timer->start(duration);
}

void QmitkPulseTimers::Cancel(const mitk::DataNode* node)
{
  if (const auto it = m_Pulses.find(node); it != m_Pulses.end())
  {
    delete it->second.Timer.data();
    m_Pulses.erase(it);
  }
}
