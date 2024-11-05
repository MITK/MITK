/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkINodeSelectionListener_h
#define mitkINodeSelectionListener_h

#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Interface for listening to node selections sent via the INodeSelectionService.
   *
   * To receive node selections, listeners must first subscribe via INodeSelectionService::AddListener().
   *
   * \sa INodeSelectionService
   */
  class MITKCORE_EXPORT INodeSelectionListener
  {
  public:
    /** \brief Callback method for received selections.
     *
     * \param context An arbitrary string that sender and receiver agreed upon to establish a connection.
     * \param selection The sent selection of potentially multiple data nodes.
     */
    virtual void OnSelectionReceived(const std::string& context, const std::vector<mitk::DataNode::Pointer>& selection) = 0;
  };
}

#endif
