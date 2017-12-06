#include "PluginLocker.h"

#include <QMessageBox>

PluginLocker::PluginLocker(QString viewId)
  : m_PartTabItems(berry::IFolderLayout::getPartTabItems())
  , m_TabLock(nullptr)
{
  if (m_PartTabItems.count(viewId))
  {
    m_TabLock = &(m_PartTabItems[viewId]);
    m_TabLock->m_Lock = true;
    m_TabLock->m_AbstractTab->SetLock(true);
  }
}

PluginLocker::PluginLocker(std::string viewId_str)
  : m_PartTabItems(berry::IFolderLayout::getPartTabItems())
  , m_TabLock(nullptr)
{
  QString viewId = viewId_str.c_str();
  if (m_PartTabItems.count(viewId))
  {
    m_TabLock = &(m_PartTabItems[viewId]);
    m_TabLock->m_Lock = true;
    m_TabLock->m_AbstractTab->SetLock(true);
  }
}

PluginLocker::~PluginLocker()
{
  if (m_TabLock)
  {
    m_TabLock->m_Lock = false;
    m_TabLock->m_AbstractTab->SetLock(false);
  }
}
