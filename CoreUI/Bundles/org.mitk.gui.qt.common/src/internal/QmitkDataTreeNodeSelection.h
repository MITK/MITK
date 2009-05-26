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

#include <mitkDataTreeNode.h>
#include <mitkDataTreeNodeSelection.h>
#include <cherryQtItemSelection.h>

#include <vector>

class QmitkDataTreeNodeSelection: public mitk::DataTreeNodeSelection, public cherry::QtItemSelection
{
public:

  cherryObjectMacro(QmitkDataTreeNodeSelection);

  QmitkDataTreeNodeSelection();
  QmitkDataTreeNodeSelection(const QItemSelection& sel);

  bool IsEmpty() const;

};
