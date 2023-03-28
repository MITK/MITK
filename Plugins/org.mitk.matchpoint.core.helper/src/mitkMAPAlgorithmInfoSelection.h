/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMAPAlgorithmInfoSelection_h
#define mitkMAPAlgorithmInfoSelection_h

#include <berryIStructuredSelection.h>

#include <mapDeploymentDLLInfo.h>

#include "org_mitk_matchpoint_core_helper_Export.h"


namespace mitk {

/**
 * \brief Used by plugins to communicate selections of deployed algorithms
 *
 * For example used by the algorithm browser to inform about the currently selected algorithm.
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

  Object::Pointer GetFirstElement() const override;
  iterator Begin() const override;
  iterator End() const override;

  int Size() const override;

  ContainerType::Pointer ToVector() const override;

  AlgorithmInfoVectorType GetSelectedAlgorithmInfo() const;

  /**
   * @see berry::ISelection::IsEmpty()
   */
  bool IsEmpty() const override;

  bool operator==(const berry::Object* obj) const override;

protected:

  ContainerType::Pointer m_Selection;
};

}


#endif
