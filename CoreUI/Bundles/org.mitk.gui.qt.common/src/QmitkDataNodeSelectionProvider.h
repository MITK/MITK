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

#include <org_mitk_gui_qt_common_Export.h>

#include <berryQtSelectionProvider.h>
#include <mitkDataNodeSelection.h>

/**
 * \ingroup org_mitk_gui_qt_common
 *
 * \brief A BlueBerry selection provider for mitk::DataNode selections.
 *
 * This class works together with a Qt item selection model which holds the actual
 * selection. The underlying Qt model must support the data role \e QmitkDataNodeRole
 * and return a mitk::DataNode::Pointer object for each QModelIndex in the selection.
 *
 * You can set a Qt item selection model using QtSelectionProvider::SetItemSelectionModel().
 */
class MITK_QT_COMMON QmitkDataNodeSelectionProvider : public berry::QtSelectionProvider
{
public:

  berryObjectMacro(QmitkDataNodeSelectionProvider)

  QmitkDataNodeSelectionProvider();

  /**
   * This method will always return a mitk::DataNodeSelection object.
   */
  berry::ISelection::ConstPointer GetSelection() const;

protected:

  mitk::DataNodeSelection::ConstPointer GetDataNodeSelection() const;

  virtual void FireSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};

#endif /* QMITKDATATREENODESELECTIONPROVIDER_H_ */
