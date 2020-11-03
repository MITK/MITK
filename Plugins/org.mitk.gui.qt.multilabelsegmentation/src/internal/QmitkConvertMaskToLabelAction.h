/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __QmitkConvertMaskToLabelAction_H_
#define __QmitkConvertMaskToLabelAction_H_

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_multilabelsegmentation_Export.h"

#include "vector"
#include "mitkDataNode.h"
#include "mitkImage.h"

class MITK_QT_SEGMENTATION QmitkConvertMaskToLabelAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkConvertMaskToLabelAction();
  ~QmitkConvertMaskToLabelAction() override;

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

#endif // __QmitkConvertMaskToLabelAction_H_
