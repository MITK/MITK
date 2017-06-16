#pragma once

#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>
#include <boost/signals2.hpp>

#include <mitkDataNode.h>

#include <MitkQtWidgetsExports.h>

enum class SynchronizeEventType
{
  IMAGE_CHANGE,
  SEGMENTATION_CHANGE
};

typedef boost::signals2::signal<void(const mitk::DataNode*)> SlotType;

class MITKQTWIDGETS_EXPORT QmitkSynchronizeSelectedData
{

public:

  ~QmitkSynchronizeSelectedData();

  static boost::signals2::connection addObserver(SynchronizeEventType type, const SlotType::slot_type& func);

  static void emitImageChange(mitk::DataNode* node);
  static void emitSegmentationChange(mitk::DataNode* node);

  static QmitkSynchronizeSelectedData* getInstance();

private:

  static QmitkSynchronizeSelectedData* m_instance;

  static boost::recursive_mutex m_mutex;
  
  static SlotType m_imageChange;
  static SlotType m_segmentationChange;

  QmitkSynchronizeSelectedData();
  QmitkSynchronizeSelectedData(const QmitkSynchronizeSelectedData&);
  QmitkSynchronizeSelectedData& operator = (const QmitkSynchronizeSelectedData&);
};
