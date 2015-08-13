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

#include <mitkAbstractClassifier.h>


void mitk::AbstractClassifier::SetNthItems(const char * val, unsigned int idx)
{
  std::stringstream ss;
  ss << "itemlist." << idx;
  this->GetPropertyList()->SetStringProperty(ss.str().c_str(),val);
}

std::string mitk::AbstractClassifier::GetNthItems(unsigned int idx) const
{
  std::stringstream ss;
  ss << "itemlist." << idx;
  std::string val;
  this->GetPropertyList()->GetStringProperty(ss.str().c_str(),val);
  return val;
}

void mitk::AbstractClassifier::SetItemList(std::vector<std::string> list)
{
  for(unsigned int i = 0 ; i < list.size(); ++i)
    this->SetNthItems(list[i].c_str(),i);
}

std::vector<std::string> mitk::AbstractClassifier::GetItemList() const
{
  std::vector<std::string> result;
  for(unsigned int idx = 0 ;; idx++)
  {
    std::stringstream ss;
    ss << "itemlist." << idx;
    if(this->GetPropertyList()->GetProperty(ss.str().c_str()))
    {
      std::string s;
      this->GetPropertyList()->GetStringProperty(ss.str().c_str(),s);
      result.push_back(s);
    }else
      break;
  }
  return result;
}
