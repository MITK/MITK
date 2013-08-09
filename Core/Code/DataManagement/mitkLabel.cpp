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


#include "mitkLabel.h"

#include "itkProcessObject.h"


mitk::Label::Label():
m_Locked(true),
m_Visible(true),
m_Filled(true),
m_Exterior(false),
m_Opacity(0.6),
m_Volume(0.0),
m_Index(0),
m_StickyBorders(false)
{

}

mitk::Label::Label(const Label& other)
  : itk::Object()
{
    this->m_Locked = other.GetLocked();
    this->m_Visible = other.GetVisible();
    this->m_Filled = other.GetFilled();
    this->m_Exterior = other.GetExterior();
    this->m_Opacity = other.GetOpacity();
    this->m_StickyBorders = other.GetStickyBorders();
    this->m_Selected = other.GetSelected();
    this->m_Name = other.GetName();
    this->m_LatinName = other.GetLatinName();
    this->m_Volume = other.GetVolume();
    this->m_LastModified = other.GetLastModified();
    this->m_Index = other.GetIndex();
    this->m_Color = other.GetColor();
}

mitk::Label::~Label()
{

}

void mitk::Label::SetColor(const mitk::Color &_color)
{
    this->m_Color = _color;
}

void mitk::Label::PrintSelf(std::ostream &os, itk::Indent indent) const
{
 // todo
}
