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


#ifndef QMITKDATATREENODESELECTIONPROVIDER_H_
#define QMITKDATATREENODESELECTIONPROVIDER_H_

#include "mitkQtCommonDll.h"

#include <cherryQtSelectionProvider.h>
#include <mitkDataTreeNodeSelection.h>

class MITK_QT_COMMON QmitkDataTreeNodeSelectionProvider : public cherry::QtSelectionProvider
{
public:

  cherryObjectMacro(QmitkDataTreeNodeSelectionProvider)

  QmitkDataTreeNodeSelectionProvider();

  cherry::ISelection::ConstPointer GetSelection() const;
  mitk::DataTreeNodeSelection::ConstPointer GetDataTreeNodeSelection() const;

protected:

  virtual void FireSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};

#endif /* QMITKDATATREENODESELECTIONPROVIDER_H_ */
