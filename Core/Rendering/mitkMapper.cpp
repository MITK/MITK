/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkMapper.h"
#include "mitkDataTreeNode.h"

const std::string mitk::Mapper::XML_NODE_NAME = "mapper";

//##ModelId=3E3C337E0162
mitk::Mapper::Mapper()
{

}


//##ModelId=3E3C337E019E
mitk::Mapper::~Mapper()
{
}

//##ModelId=3E860B9A0378
mitk::BaseData* mitk::Mapper::GetData() const
{
  return m_DataTreeNode->GetData();
}

//##ModelId=3EF17276014B
bool mitk::Mapper::GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetColor(rgb, renderer, name);
}

//##ModelId=3EF17795006A
bool mitk::Mapper::GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetVisibility(visible, renderer, name);
}

//##ModelId=3EF1781F0285
bool mitk::Mapper::GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetOpacity(opacity, renderer, name);
}

//##ModelId=3EF179660018
bool mitk::Mapper::GetLevelWindow(mitk::LevelWindow& levelWindow, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetLevelWindow(levelWindow, renderer, name);
}

//##ModelId=3EF18B340008
bool mitk::Mapper::IsVisible(mitk::BaseRenderer* renderer, const char* name) const
{
    bool visible=true;
    GetVisibility(visible, renderer, name);
    return visible;
}

void mitk::Mapper::GenerateData()
{

}

void mitk::Mapper::GenerateData(mitk::BaseRenderer* /*renderer*/)
{

}

//##ModelId=3EF1A43C01D9
void mitk::Mapper::Update(mitk::BaseRenderer* renderer)
{
  const DataTreeNode* node = GetDataTreeNode();
  assert(node!=NULL);
  
  //safty cause there are datatreenodes, that have no defined data (video-nodes and root)
  unsigned int dataMTime = 0;
  mitk::BaseData::Pointer data = static_cast<mitk::BaseData *>(node->GetData());
  if (data.IsNotNull())
  {
    dataMTime = data->GetMTime();
  }

  if(
      (m_LastUpdateTime < GetMTime()) ||
      (m_LastUpdateTime < node->GetDataReferenceChangedTime()) ||
      (m_LastUpdateTime < dataMTime)
    )
  {
    GenerateData();
    m_LastUpdateTime.Modified();
  }

  GenerateData(renderer);
}

const std::string& mitk::Mapper::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}
