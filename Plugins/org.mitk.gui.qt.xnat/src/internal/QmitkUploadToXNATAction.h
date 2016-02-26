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
#ifndef QMITK_UPLOADTOXNATACTION_H
#define QMITK_UPLOADTOXNATACTION_H

#include "mitkIContextMenuAction.h"

#include "org_mitk_gui_qt_xnat_Export.h"

#include "mitkDataNode.h"

class XNAT_EXPORT QmitkUploadToXNATAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkUploadToXNATAction();
  virtual ~QmitkUploadToXNATAction();

  //interface methods
  void Run( const QList<mitk::DataNode::Pointer>& selectedNodes ) override;

  // Setters
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart* functionality) override;

private:

  typedef QList<mitk::DataNode::Pointer> NodeList;

};

#endif // QMITK_UPLOADTOXNATACTION_H
