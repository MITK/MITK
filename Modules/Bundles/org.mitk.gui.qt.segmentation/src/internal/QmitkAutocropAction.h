#ifndef QMITK_AUTOCROPACTION_H
#define QMITK_AUTOCROPACTION_H

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_segmentation_Export.h"

#include "vector"
#include "mitkDataNode.h"
#include "mitkImage.h"

class QmitkStdMultiWidget;

class MITK_QT_SEGMENTATION QmitkAutocropAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkAutocropAction();
  QmitkAutocropAction(const QmitkAutocropAction& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkAutocropAction();

  //interface methods
  void Run( const std::vector<mitk::DataNode*>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetStdMultiWidget(QmitkStdMultiWidget *stdMultiWidget);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart* functionality);

protected:

  mitk::Image::Pointer IncreaseCroppedImageSize( mitk::Image::Pointer image );

private:

  typedef std::vector<mitk::DataNode*> NodeList;

};

#endif // QMITK_AUTOCROPACTION_H
