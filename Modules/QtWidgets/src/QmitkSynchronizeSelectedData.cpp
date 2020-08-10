#include <boost/thread/locks.hpp>

#include "../include/QmitkSynchronizeSelectedData.h"

boost::recursive_mutex QmitkSynchronizeSelectedData::m_mutex;
SlotType QmitkSynchronizeSelectedData::m_imageChange;
SlotType QmitkSynchronizeSelectedData::m_segmentationChange;
SlotListType QmitkSynchronizeSelectedData::m_imageListChange;

boost::signals2::connection QmitkSynchronizeSelectedData::addObserver(SynchronizeEventType type, const SlotType::slot_type& func)
{
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

  boost::signals2::connection currentConnect;
  
  if (type == SynchronizeEventType::IMAGE_CHANGE) {
    currentConnect = m_imageChange.connect(func);
  } else if (type == SynchronizeEventType::SEGMENTATION_CHANGE) {
    currentConnect = m_segmentationChange.connect(func);
  }
  
  return currentConnect;
}

boost::signals2::connection QmitkSynchronizeSelectedData::addImageListObserver(const SlotListType::slot_type& func)
{
  return m_imageListChange.connect(func);
}

void QmitkSynchronizeSelectedData::emitImageChange(mitk::DataNode* node)
{
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
  m_imageChange(node);
}

void QmitkSynchronizeSelectedData::emitSegmentationChange(mitk::DataNode* node)
{
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
  m_segmentationChange(node);
}

void QmitkSynchronizeSelectedData::emitImageListChange(std::vector<mitk::DataNode*>& nodes) {
  boost::unique_lock<boost::recursive_mutex> lock(m_mutex);
  m_imageListChange(nodes);
}
