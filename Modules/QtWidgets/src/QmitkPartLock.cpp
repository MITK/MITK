/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPartLock.h"

QmitkPartLock* QmitkPartLock::GetInstance()
{
  static QmitkPartLock instance;
  return &instance;
}

QmitkPartLock::QmitkPartLock()
  : m_Locked(false)
{
}

QmitkPartLock::~QmitkPartLock()
{
}

void QmitkPartLock::Lock(const QString& exemptPartId, const QString& reason)
{
  if (m_Locked && m_ExemptPartId == exemptPartId && m_Reason == reason)
    return;

  m_Locked = true;
  m_ExemptPartId = exemptPartId;
  m_Reason = reason;

  emit Changed();
}

void QmitkPartLock::Unlock()
{
  if (!m_Locked)
    return;

  m_Locked = false;
  m_ExemptPartId.clear();
  m_Reason.clear();

  emit Changed();
}

bool QmitkPartLock::IsLocked() const
{
  return m_Locked;
}

bool QmitkPartLock::IsLocked(const QString& partId) const
{
  return m_Locked && partId != m_ExemptPartId;
}

QString QmitkPartLock::GetExemptPartId() const
{
  return m_Locked ? m_ExemptPartId : QString();
}

QString QmitkPartLock::GetReason() const
{
  return m_Reason;
}
