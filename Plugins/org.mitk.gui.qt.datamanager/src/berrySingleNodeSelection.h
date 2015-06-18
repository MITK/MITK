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

#ifndef QMITKSINGLENODESELECTION_H_
#define QMITKSINGLENODESELECTION_H_

// Own includes
#include "berryISelection.h"
/// Qmitk
#include <org_mitk_gui_qt_datamanager_Export.h>

//# forward declarations
namespace mitk
{
  class DataNode;
}

namespace berry
{

  ///
  /// \struct SingleNodeSelection
  /// \brief Represents a selection object that encapsulates the selection of a single node.
  ///
  struct MITK_QT_DATAMANAGER SingleNodeSelection: public berry::ISelection
  {
    ///
    /// \brief Make typdefs for pointer types.
    ///
    berryObjectMacro(SingleNodeSelection);
    ///
    /// \brief Node setter.
    ///
    void SetNode(mitk::DataNode* _SelectedNode);
    ///
    /// \brief Node getter.
    ///
    mitk::DataNode* GetNode() const;
    ///
    /// \brief Checks if node is 0.
    ///
    virtual bool IsEmpty() const override;

  protected:
    ///
    /// \brief Holds the node that is currently selected.
    ///
    mitk::DataNode* m_Node;
  };

}

#endif // QMITKSINGLENODESELECTION_H_
