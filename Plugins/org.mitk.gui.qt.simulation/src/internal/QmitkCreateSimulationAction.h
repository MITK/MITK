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

#ifndef QmitkCreateSimulationAction_h
#define QmitkCreateSimulationAction_h

#include <QObject>
#include <mitkIContextMenuAction.h>

class QmitkCreateSimulationAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkCreateSimulationAction();
  ~QmitkCreateSimulationAction();

  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes);
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetFunctionality(berry::QtViewPart* functionality);
  void SetDecimated(bool decimated);
  void SetSmoothed(bool smoothed);

private:
  QmitkCreateSimulationAction(const QmitkCreateSimulationAction&);
  QmitkCreateSimulationAction& operator=(const QmitkCreateSimulationAction&);

  mitk::DataStorage::Pointer m_DataStorage;
};

#endif
