#ifndef QMITK_STATISTICSACTION_H
#define QMITK_STATISTICSACTION_H

#include <berryIWorkbenchPage.h>
#include "berryQtViewPart.h"

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_segmentation_Export.h"

class QmitkStdMultiWidget;

class MITK_QT_SEGMENTATION QmitkStatisticsAction: public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkStatisticsAction();
  QmitkStatisticsAction(const QmitkStatisticsAction& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkStatisticsAction();

  //interface methods
  void Run( const std::vector<mitk::DataNode*>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetStdMultiWidget(QmitkStdMultiWidget *stdMultiWidget);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart* functionality);

protected:

  //needs to be set over the IContextMenuAction (with this - QmitkDataManagerView - as parameter)
  berry::QtViewPart* m_BlueBerryView;
};
#endif // QMITK_STATISTICSACTION_H
