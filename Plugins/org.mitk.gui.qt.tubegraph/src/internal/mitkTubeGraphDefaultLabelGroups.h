/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitk_TubeGraphDefaultLabelGroups_h
#define _mitk_TubeGraphDefaultLabelGroups_h

#include "mitkTubeGraphProperty.h"

#include <vector>

namespace mitk
{
  class TubeGraphDefaultLabelGroups
  {

    typedef TubeGraphProperty::LabelGroup  LabelGroupType;
    typedef LabelGroupType::Label          LabelType;

  public:

    TubeGraphDefaultLabelGroups();
    virtual ~TubeGraphDefaultLabelGroups();

    std::vector<LabelGroupType*> GetLabelGroupForLung();
    std::vector<LabelGroupType*> GetLabelGroupForLiver();

  private:
    std::vector<LabelGroupType*> m_labelGroupsLung;
    std::vector<LabelGroupType*> m_labelGroupsLiver;

  };
}//namespace mitk
#endif
