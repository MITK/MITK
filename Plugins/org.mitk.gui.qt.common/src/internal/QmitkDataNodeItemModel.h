/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKDATANODEITEMMODEL_H
#define QMITKDATANODEITEMMODEL_H

#include <QStandardItemModel>

#include <mitkDataNode.h>

class QmitkDataNodeItemModel : public QStandardItemModel
{

public:

  QmitkDataNodeItemModel(QObject* parent = 0);

  void AddDataNode(mitk::DataNode::Pointer node);
  void AddDataNodes(const QList<mitk::DataNode::Pointer>& nodes);
};

#endif // QMITKDATANODEITEMMODEL_H
