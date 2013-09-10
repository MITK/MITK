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
#include <mitkContourModelSet.h>
#include <vtkMath.h>
#include <algorithm>

mitk::ContourModelSet::ContourModelSet()
{
  this->m_Contours;
}



mitk::ContourModelSet::ContourModelSet(const mitk::ContourModelSet &other) :
  m_Contours(other.m_Contours)
{
}



mitk::ContourModelSet::~ContourModelSet()
{
  this->m_Contours.clear();
}



void mitk::ContourModelSet::AddContourModel(mitk::ContourModel &contourModel)
{
  this->m_Contours.push_back(&contourModel);
}


void mitk::ContourModelSet::AddContourModel(mitk::ContourModel::Pointer contourModel)
{
  this->m_Contours.push_back(contourModel);
}



mitk::ContourModel* mitk::ContourModelSet::GetContourModelAt(int index)
{
  return this->m_Contours.at(index).GetPointer();
}

bool mitk::ContourModelSet::IsEmpty()
{
  return this->m_Contours.empty();
}


mitk::ContourModelSet::ContourModelListType* mitk::ContourModelSet::GetContourModelList()
{
  return &(this->m_Contours);
}


bool mitk::ContourModelSet::RemoveContourModel(mitk::ContourModel* contourModel)
{
  ContourModelSetIterator it = this->m_Contours.begin();

  ContourModelSetIterator end = this->m_Contours.end();

  //search for ContourModel and remove it if exists
  while(it != end)
  {
    if((*it) == contourModel)
    {
      this->m_Contours.erase(it);
      return true;
    }

    it++;
  }

  return false;
}


bool mitk::ContourModelSet::RemoveContourModelAt(int index)
{
  if( index >= 0 && index < this->m_Contours.size() )
  {
    this->m_Contours.erase(this->m_Contours.begin()+index);
    return true;
  }
  else
  {
    return false;
  }
}


void mitk::ContourModelSet::Clear()
{
  this->m_Contours.clear();
}
