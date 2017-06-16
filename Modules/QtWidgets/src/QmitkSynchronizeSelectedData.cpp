#include <boost/thread/locks.hpp>

#include "../include/QmitkSynchronizeSelectedData.h"

QmitkSynchronizeSelectedData* QmitkSynchronizeSelectedData::m_instance = nullptr;
boost::recursive_mutex QmitkSynchronizeSelectedData::m_mutex;
SlotType QmitkSynchronizeSelectedData::m_imageChange;
SlotType QmitkSynchronizeSelectedData::m_segmentationChange;

QmitkSynchronizeSelectedData::QmitkSynchronizeSelectedData()
{
}

QmitkSynchronizeSelectedData::~QmitkSynchronizeSelectedData()
{
  m_imageChange.disconnect_all_slots();
  m_segmentationChange.disconnect_all_slots();
}

QmitkSynchronizeSelectedData* QmitkSynchronizeSelectedData::getInstance()
{
  if (m_instance == nullptr)
  {
    m_instance = new QmitkSynchronizeSelectedData();
  }

  return m_instance;
}

boost::signals2::connection QmitkSynchronizeSelectedData::addObserver(SynchronizeEventType type, const SlotType::slot_type& func)
{
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

  boost::signals2::connection currentConnect;
  
  if (type == SynchronizeEventType::IMAGE_CHANGE)
  {
    currentConnect = getInstance()->m_imageChange.connect(func);
  }
  else if (type == SynchronizeEventType::SEGMENTATION_CHANGE)
  {
    currentConnect = getInstance()->m_segmentationChange.connect(func);
  }
  
  return currentConnect;
}

void QmitkSynchronizeSelectedData::emitImageChange(mitk::DataNode* node)
{
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
  getInstance()->m_imageChange(node);
}

void QmitkSynchronizeSelectedData::emitSegmentationChange(mitk::DataNode* node)
{
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
  getInstance()->m_segmentationChange(node);
}
