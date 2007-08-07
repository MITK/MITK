/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 9841 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <sstream>
#include "mitkPicHeaderProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

mitk::PicHeaderProperty::PicHeaderProperty(ipPicTSV_t* tag)
: m_StoredTag(NULL)
{
  if (tag)
  {
    m_StoredTag = _ipPicCloneTag( tag );
  }
}

mitk::PicHeaderProperty::~PicHeaderProperty()
{
  if (m_StoredTag)
    ipPicFreeTag( m_StoredTag );
}

bool mitk::PicHeaderProperty::Assignable(const mitk::BaseProperty& other) const
{
  try
  {
    dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away!
    return true;
  }
  catch (std::bad_cast)
  {
  }
  return false;
}

mitk::BaseProperty& mitk::PicHeaderProperty::operator=(const mitk::BaseProperty& other)
{
  try
  {
    const Self& otherProp( dynamic_cast<const Self&>(other) );

    if (this->m_StoredTag)
    {
      ipPicFreeTag( this->m_StoredTag );
    }

    this->m_StoredTag = _ipPicCloneTag( otherProp.m_StoredTag );
    this->Modified();
  }
  catch (std::bad_cast)
  {
    // nothing to do then
  }

  return *this;
 }


bool mitk::PicHeaderProperty::operator==(const BaseProperty& property) const
{
  const Self *other = dynamic_cast<const Self*>(&property);

  if(other == NULL) return false;

  return (
      other->m_StoredTag->tag == m_StoredTag->tag &&
      other->m_StoredTag->type == m_StoredTag->type &&
      other->m_StoredTag->bpe == m_StoredTag->bpe &&
      other->m_StoredTag->dim == m_StoredTag->dim &&
      other->m_StoredTag->value == m_StoredTag->value
      );
}

ipPicTSV_t const* mitk::PicHeaderProperty::GetValue() const
{
  return m_StoredTag;
}

std::string mitk::PicHeaderProperty::GetValueAsString() const 
{
  std::stringstream myStr;

  if (m_StoredTag)
  {
    // TODO for valid text output, we would need to consider the bpe field and cast to the correct type
    // BUT we don't need to display these values so urgently
    if (m_StoredTag->type == ipPicASCII)
    {
      myStr << (char*)(m_StoredTag->value);
    }
    else if (m_StoredTag->type == ipPicBool)
    {
      myStr << *static_cast<bool*>(m_StoredTag->value);
    }
    else if (m_StoredTag->type == ipPicInt)
    {
      myStr << *static_cast<int*>(m_StoredTag->value);
    }
    else if (m_StoredTag->type == ipPicUInt)
    {
      myStr << *static_cast<unsigned int*>(m_StoredTag->value);
    }
    else if (m_StoredTag->type == ipPicFloat)
    {
      myStr << *static_cast<float*>(m_StoredTag->value);
    }
    else
    {
      myStr << "(no text representation possible)";
    }
  }
  else
  {
    myStr << "no value";
  }

  return myStr.str(); 
}

