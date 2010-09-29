#ifndef QMITK_STATISTICSACTION_H
#define QMITK_STATISTICSACTION_H

#include "mitkIContextMenuAction.h"

#include "mitkQtSegmentationDll.h"

#include "berryQtViewPart.h"

class MITK_QT_SEGMENTATION QmitkStatisticsAction: public mitk::IContextMenuAction
{
public:

  QmitkStatisticsAction();
  virtual ~QmitkStatisticsAction();

  //interface methods
  void Run( const std::vector<mitk::DataNode*>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetSmoothed(bool smoothed);
  void SetFunctionality(berry::QtViewPart* functionality);

protected:

  //needs to be set over the IContextMenuAction (with this - QmitkDataManagerView - as parameter)
  berry::QtViewPart* m_BlueBerryView;
};
#endif // QMITK_STATISTICSACTION_H
