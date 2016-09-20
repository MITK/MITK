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

#ifndef _MITK_MAP_ALGORITHM_INFO_SELECTION_H
#define _MITK_MAP_ALGORITHM_INFO_SELECTION_H

#include <berryIStructuredSelection.h>

#include <mapDeploymentDLLInfo.h>

#include "org_mitk_matchpoint_core_helper_Export.h"


namespace mitk {

/**
 * @class Used by plugins to communicate selections of deployed algorithms
 * E.G. used by the algorithm browser to inform about the currently selected algorithm.
 */
class MITK_MATCHPOINT_CORE_HELPER_EXPORT MAPAlgorithmInfoSelection : public virtual berry::IStructuredSelection
{

public:

  berryObjectMacro(MAPAlgorithmInfoSelection);

  typedef ::map::deployment::DLLInfo AlgorithmInfoType;
  typedef std::vector<AlgorithmInfoType::ConstPointer> AlgorithmInfoVectorType;

  MAPAlgorithmInfoSelection();
  MAPAlgorithmInfoSelection(AlgorithmInfoType::ConstPointer info);
  MAPAlgorithmInfoSelection(const AlgorithmInfoVectorType& infos);

  virtual Object::Pointer GetFirstElement() const;
  virtual iterator Begin() const;
  virtual iterator End() const;

  virtual int Size() const;

  virtual ContainerType::Pointer ToVector() const;

  AlgorithmInfoVectorType GetSelectedAlgorithmInfo() const;

  /**
   * @see berry::ISelection::IsEmpty()
   */
  bool IsEmpty() const;

  bool operator==(const berry::Object* obj) const;

protected:

  ContainerType::Pointer m_Selection;
};

}


#endif