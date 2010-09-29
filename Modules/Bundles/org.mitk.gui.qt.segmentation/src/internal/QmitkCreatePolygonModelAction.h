#ifndef QMITK_CREATEPOLYGONMODELACTION_H
#define QMITK_CREATEPOLYGONMODELACTION_H

#include "mitkIContextMenuAction.h"
#include "berryQtViewPart.h"

#include "mitkQtSegmentationDll.h"

#include "mitkDataNode.h"

#include "mitkShowSegmentationAsSurface.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

class MITK_QT_SEGMENTATION QmitkCreatePolygonModelAction: public mitk::IContextMenuAction
{

public:
  QmitkCreatePolygonModelAction();
  virtual ~QmitkCreatePolygonModelAction();

  //interface methods
  void Run( const std::vector<mitk::DataNode*>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetSmoothed(bool smoothed);
  void SetFunctionality(berry::QtViewPart* functionality);

  // for receiving messages
  void OnSurfaceCalculationDone();

protected:

  typedef std::vector<mitk::DataNode*> NodeList;

  mitk::DataStorage::Pointer m_DataStorage;

  bool m_IsSmoothed;

};
#endif // QMITK_CREATEPOLYGONMODELACTION_H
