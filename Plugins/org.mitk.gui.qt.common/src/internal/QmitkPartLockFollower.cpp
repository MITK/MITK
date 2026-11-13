/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPartLockFollower.h"

#include <QmitkIconTheme.h>
#include <QmitkPartLock.h>

#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPart.h>
#include <berryIWorkbenchPartSite.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>

#include <internal/berryWorkbenchPartReference.h>

#include <QEvent>
#include <QFile>

namespace
{
  QIcon GetLockIcon()
  {
    static const QByteArray svg = []() {
      QFile file(":/Qmitk/lock.svg");
      return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }();

    return QmitkIconTheme::GetIcon(svg, QmitkIconTheme::GetErrorColor());
  }
}

QmitkPartLockFollower::QmitkPartLockFollower(const berry::IWorkbenchPart* part, QWidget* content, SetTitleFunction setTitle)
  : m_Part(part),
    m_PartId(part->GetSite()->GetId()),
    m_Content(content),
    m_SetTitle(std::move(setTitle)),
    m_Locked(false)
{
  connect(QmitkPartLock::GetInstance(), &QmitkPartLock::Changed, this, &QmitkPartLockFollower::Update);

  // The error color of the lock icon is the theme's.
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, [this]() {
    if (m_Locked)
      m_SetTitle(GetLockIcon(), QmitkPartLock::GetInstance()->GetReason());
  });

  if (m_Content != nullptr)
    m_Content->installEventFilter(this);

  // A part that opens while the lock holds starts locked.
  this->Update();
}

QmitkPartLockFollower::~QmitkPartLockFollower()
{
}

bool QmitkPartLockFollower::eventFilter(QObject* watched, QEvent* event)
{
  // The content of an editor is the control of its pane, which BlueBerry
  // enables on its own, for example when its tab is dropped into another
  // stack.
  if (watched == m_Content.data() && event->type() == QEvent::EnabledChange && m_Content->isEnabled()
      && QmitkPartLock::GetInstance()->IsLocked(m_PartId))
  {
    m_Content->setEnabled(false);
  }

  return QObject::eventFilter(watched, event);
}

void QmitkPartLockFollower::Update()
{
  const auto* lock = QmitkPartLock::GetInstance();
  const bool locked = lock->IsLocked(m_PartId);

  // Only at a transition: a part that was not locked keeps the state it has
  // for other reasons.
  if (m_Content != nullptr && locked != m_Locked)
    m_Content->setEnabled(!locked);

  if (locked)
  {
    if (!m_Locked)
    {
      m_TitleImage = m_Part->GetTitleImage();
      m_TitleToolTip = m_Part->GetTitleToolTip();
    }

    m_SetTitle(GetLockIcon(), lock->GetReason());
  }
  else if (m_Locked)
  {
    m_SetTitle(m_TitleImage, m_TitleToolTip);
  }

  m_Locked = locked;
}

QmitkUncreatedViewLockFollower::QmitkUncreatedViewLockFollower()
{
  connect(QmitkPartLock::GetInstance(), &QmitkPartLock::Changed, this, &QmitkUncreatedViewLockFollower::Update);
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, &QmitkUncreatedViewLockFollower::Update);
}

QmitkUncreatedViewLockFollower::~QmitkUncreatedViewLockFollower()
{
  this->Restore();
}

void QmitkUncreatedViewLockFollower::Update()
{
  // From scratch, as the exempt view and the colors of the lock may have changed.
  this->Restore();

  const auto* lock = QmitkPartLock::GetInstance();

  if (!lock->IsLocked() || !berry::PlatformUI::IsWorkbenchRunning())
    return;

  const auto icon = GetLockIcon();

  for (const auto& window : berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows())
  {
    const auto page = window->GetActivePage();

    if (page.IsNull())
      continue;

    for (const auto& view : page->GetViewReferences())
    {
      // A view that exists follows the lock through its own follower.
      if (view->GetPart(false).IsNotNull() || !lock->IsLocked(view->GetId()))
        continue;

      if (const auto reference = view.Cast<berry::WorkbenchPartReference>(); reference.IsNotNull())
      {
        m_Icons.emplace_back(view, view->GetTitleImage());
        reference->SetImageDescriptor(icon);
      }
    }
  }
}

void QmitkUncreatedViewLockFollower::Restore()
{
  for (const auto& [view, icon] : m_Icons)
  {
    // A view created meanwhile shows the icon of its part again through its own follower.
    const auto reference = view.Cast<berry::WorkbenchPartReference>();

    if (!reference->IsDisposed() && view->GetPart(false).IsNull())
      reference->SetImageDescriptor(icon);
  }

  m_Icons.clear();
}
