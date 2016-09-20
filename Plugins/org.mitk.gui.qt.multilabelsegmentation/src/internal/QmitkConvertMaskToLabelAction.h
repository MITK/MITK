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
#ifndef __QmitkConvertMaskToLabelAction_H_
#define __QmitkConvertMaskToLabelAction_H_

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_multilabelsegmentation_Export.h"

#include "vector"
#include "mitkDataNode.h"
#include "mitkImage.h"

class QmitkStdMultiWidget;

class MITK_QT_SEGMENTATION QmitkConvertMaskToLabelAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkConvertMaskToLabelAction();
  virtual ~QmitkConvertMaskToLabelAction();

  //interface methods
  void Run( const QList<mitk::DataNode::Pointer>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart* functionality);

protected:

private:

  typedef QList<mitk::DataNode::Pointer> NodeList;

};

#endif // __QmitkConvertMaskToLabelAction_H_
