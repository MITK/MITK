#ifndef QMITK_AUTOCROPACTION_H
#define QMITK_AUTOCROPACTION_H

#include "mitkQtSegmentationDll.h"

#include "mitkIContextMenuAction.h"

#include "vector"
#include "mitkDataNode.h"
#include "mitkImage.h"

#include "mitkAutoCropImageFilter.h"
#include "mitkImageCast.h"

#include "mitkProgressBar.h"

#include <itkConstantPadImageFilter.h>

//needed for qApp 
#include <qcoreapplication.h>

class MITK_QT_SEGMENTATION QmitkAutocropAction : public mitk::IContextMenuAction
{
public:

  QmitkAutocropAction();
  virtual ~QmitkAutocropAction();

  //interface methods
  void Run( const std::vector<mitk::DataNode*>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetSmoothed(bool smoothed);
  void SetFunctionality(berry::QtViewPart* functionality);

protected:

  mitk::Image::Pointer IncreaseCroppedImageSize( mitk::Image::Pointer image );

private:

  typedef std::vector<mitk::DataNode*> NodeList;

};

#endif // QMITK_AUTOCROPACTION_H
