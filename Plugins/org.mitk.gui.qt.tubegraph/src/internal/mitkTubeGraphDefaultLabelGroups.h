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
