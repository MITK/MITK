/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphDefaultLabelGroups.h"

#include "mitkColorProperty.h"

mitk::TubeGraphDefaultLabelGroups::TubeGraphDefaultLabelGroups()
{
  mitk::Color color;

  auto   newLabelGroup = new LabelGroupType();
  newLabelGroup->labelGroupName = "Vessel Type";

  auto   label = new LabelType();
  label->labelName = "Undefined";
  color[0] = 170; color[1] = 170; color[2] = 169;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "Artery";
  color[0] = 255; color[1] = 0; color[2] = 0;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "Hepatic Vein";
  color[0] = 255; color[1] = 0; color[2] = 0;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "Portal Vein";
  color[0] = 0; color[1] = 0; color[2] = 125;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "Bile Duct";
  color[0] = 0; color[1] = 255; color[2] = 0;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  m_labelGroupsLiver.push_back(newLabelGroup);
  /////////////////////////////////////////////////////////////////
  newLabelGroup = new LabelGroupType();
  newLabelGroup->labelGroupName = "Resection Status";

  label = new LabelType();
  label->labelName = "Undefined";
  color[0] = 170; color[1] = 170; color[2] = 169;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "to be resected";
  color[0] = 255; color[1] = 0; color[2] = 0;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "not to be resected";
  color[0] = 0; color[1] = 255; color[2] = 0;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "area at risk";
  color[0] = 255; color[1] = 255; color[2] = 0;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  m_labelGroupsLiver.push_back(newLabelGroup);
  ///////////////////////////////////////////////////////////
  newLabelGroup = new LabelGroupType();
  newLabelGroup->labelGroupName = "Segments (Portal)";

  label = new LabelType();
  label->labelName = "Undefined";
  color[0] = 170; color[1] = 170; color[2] = 169;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "I";
  color[0] = 95; color[1] = 158; color[2] = 160;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "II";
  color[0] = 122; color[1] = 197; color[2] = 205;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "III";
  color[0] = 0; color[1] = 154; color[2] =205;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "IVa";
  color[0] = 154; color[1] = 192; color[2] = 205;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "IVb";
  color[0] = 104; color[1] = 131; color[2] = 139;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "V";
  color[0] = 118; color[1] = 238; color[2] = 198;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "VI";
  color[0] = 102; color[1] = 205; color[2] = 170;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "VII";
  color[0] = 69; color[1] = 139; color[2] = 116;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  label = new LabelType();
  label->labelName = "VIII";
  color[0] = 46; color[1] = 139; color[2] = 87;
  label->labelColor = color;
  label->isVisible = true;
  newLabelGroup->labels.push_back(label);

  m_labelGroupsLiver.push_back(newLabelGroup);
}
mitk::TubeGraphDefaultLabelGroups::~TubeGraphDefaultLabelGroups()
{
}

std::vector<mitk::TubeGraphProperty::LabelGroup*> mitk::TubeGraphDefaultLabelGroups::GetLabelGroupForLung()
{
  return m_labelGroupsLung;
}

std::vector<mitk::TubeGraphProperty::LabelGroup*> mitk::TubeGraphDefaultLabelGroups::GetLabelGroupForLiver()
{
  return m_labelGroupsLiver;
}
