/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
  ~QmitkUploadToXNATAction() override;

  //interface methods
  void Run( const QList<mitk::DataNode::Pointer>& selectedNodes ) override;

  // Empty setters (not needed)
  void SetDataStorage(mitk::DataStorage* dataStorage) override;
  void SetSmoothed(bool smoothed) override;
  void SetDecimated(bool decimated) override;
  void SetFunctionality(berry::QtViewPart* functionality) override;

private:
  std::string ReplaceSpecialChars(const std::string& input) const;
  typedef QList<mitk::DataNode::Pointer> NodeList;

};

#endif // QMITK_UPLOADTOXNATACTION_H
