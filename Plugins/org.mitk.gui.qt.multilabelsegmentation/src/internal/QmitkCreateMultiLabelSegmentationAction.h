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
#ifndef QMITK_CreateMultiLabelSegmentation_H
#define QMITK_CreateMultiLabelSegmentation_H

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_multilabelsegmentation_Export.h"

#include "vector"
#include "mitkDataNode.h"

class MITK_QT_SEGMENTATION QmitkCreateMultiLabelSegmentationAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkCreateMultiLabelSegmentationAction();
  virtual ~QmitkCreateMultiLabelSegmentationAction();

  //interface methods
  virtual void Run( const QList<mitk::DataNode::Pointer>& selectedNodes ) override;
  virtual void SetDataStorage(mitk::DataStorage* dataStorage) override;
  virtual void SetFunctionality(berry::QtViewPart* functionality) override;
  virtual void SetSmoothed(bool smoothed) override;
  virtual void SetDecimated(bool decimated) override;

private:

  typedef QList<mitk::DataNode::Pointer> NodeList;

  mitk::DataStorage::Pointer m_DataStorage;
};

#endif // QMITK_CreateMultiLabelSegmentation_H
