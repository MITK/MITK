/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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
  virtual ~QmitkAutocropAction();

  //interface methods
  void Run( const QList<mitk::DataNode::Pointer>& selectedNodes ) override;
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart* functionality) override;

protected:

  mitk::Image::Pointer IncreaseCroppedImageSize( mitk::Image::Pointer image );

private:

  typedef QList<mitk::DataNode::Pointer> NodeList;

};

#endif // QMITK_AUTOCROPACTION_H
