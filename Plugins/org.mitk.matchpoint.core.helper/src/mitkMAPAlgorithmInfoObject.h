/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_MAP_ALGORITHM_INFO_OBJECT_H
#define _MITK_MAP_ALGORITHM_INFO_OBJECT_H

#include <berryObject.h>
#include <berryMacros.h>

#include <mapDeploymentDLLInfo.h>

#include "org_mitk_matchpoint_core_helper_Export.h"

namespace mitk
{

/**
 * \brief berry wrapper for a MatchPoint algorithm deployment info
 *
 * Used by mitk::MAPAlgorithmInfoSelection.
 */
class MITK_MATCHPOINT_CORE_HELPER_EXPORT MAPAlgorithmInfoObject : public berry::Object
{

public:

  berryObjectMacro(mitk::MAPAlgorithmInfoObject);

  MAPAlgorithmInfoObject();
  MAPAlgorithmInfoObject(::map::deployment::DLLInfo::ConstPointer info);

  const ::map::deployment::DLLInfo* GetInfo() const;

  bool operator==(const berry::Object* obj) const override;

private:

  ::map::deployment::DLLInfo::ConstPointer m_Info;


};

}

#endif
