/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkBaseRenderer.h"
#include "mitkProperties.h"

mitk::Mapper::Mapper()
  : m_VtkBased( true ), m_TimeStep( 0 )
{

}


mitk::Mapper::~Mapper()
{
}

mitk::BaseData* mitk::Mapper::GetData() const
{
  return m_DataTreeNode->GetData();
}

mitk::DataTreeNode* mitk::Mapper::GetDataTreeNode() const
{
  itkDebugMacro("returning DataTreeNode address " << this->m_DataTreeNode );
  return this->m_DataTreeNode.GetPointer();
}


bool mitk::Mapper::GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetColor(rgb, renderer, name);
}

bool mitk::Mapper::GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetVisibility(visible, renderer, name);
}

bool mitk::Mapper::GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetOpacity(opacity, renderer, name);
}

bool mitk::Mapper::GetLevelWindow(mitk::LevelWindow& levelWindow, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetLevelWindow(levelWindow, renderer, name);
}

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

void mitk::Mapper::CalculateTimeStep( mitk::BaseRenderer *renderer )
{
  if ( ( renderer != NULL ) && ( m_DataTreeNode.GetPointer() != NULL ) )
  {
    m_TimeStep = renderer->GetTimeStep(m_DataTreeNode->GetData());
  }
  else
  {
    m_TimeStep = 0;
  }
}

void mitk::Mapper::Update(mitk::BaseRenderer *renderer)
{
  const DataTreeNode* node = GetDataTreeNode();
  assert(node!=NULL);

  this->CalculateTimeStep( renderer );

  //safety cause there are datatreenodes that have no defined data (video-nodes and root)
  unsigned int dataMTime = 0;
  mitk::BaseData::Pointer data = static_cast<mitk::BaseData *>(node->GetData());
  if (data.IsNotNull())
  {
    dataMTime = data->GetMTime();
  }

  if(
      (m_LastUpdateTime < GetMTime()) ||
      (m_LastUpdateTime < node->GetDataReferenceChangedTime()) ||
      (m_LastUpdateTime < dataMTime) ||
      (renderer && (m_LastUpdateTime < renderer->GetTimeStepUpdateTime()))
    )
  {
    this->GenerateData();
    m_LastUpdateTime.Modified();
  }

  this->GenerateData(renderer);
}

void mitk::Mapper::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "visible", mitk::BoolProperty::New(true), renderer, overwrite );
  node->AddProperty( "layer", mitk::IntProperty::New(0), renderer, overwrite);
  node->AddProperty( "name", mitk::StringProperty::New("No Name!"), renderer, overwrite );
}
