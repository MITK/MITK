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
#ifndef __QmitkConvertSurfaceToLabelAction_H_
#define __QmitkConvertSurfaceToLabelAction_H_

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_multilabelsegmentation_Export.h"

#include "vector"
#include "mitkDataNode.h"
#include "mitkImage.h"

class MITK_QT_SEGMENTATION QmitkConvertSurfaceToLabelAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkConvertSurfaceToLabelAction();
  ~QmitkConvertSurfaceToLabelAction() override;

  //interface methods
  void Run( const QList<mitk::DataNode::Pointer>& selectedNodes ) override;
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart* functionality) override;

protected:

private:

  typedef QList<mitk::DataNode::Pointer> NodeList;

};

#endif // __QmitkConvertSurfaceToLabelAction_H_
