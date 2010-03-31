/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKSINGLENODESELECTION_H_
#define QMITKSINGLENODESELECTION_H_

// Own includes
#include "berryISelection.h"
/// Qmitk
#include "mitkQtDataManagerDll.h"

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
    virtual bool IsEmpty() const;

  protected:
    ///
    /// \brief Holds the node that is currently selected.
    ///
    mitk::DataNode* m_Node;
  };

}

#endif // QMITKSINGLENODESELECTION_H_