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

#include "mitkOverlay.h"

mitk::Overlay::Overlay() : m_LayoutedBy(NULL)
{
  m_PropertyList = mitk::PropertyList::New();
}


mitk::Overlay::~Overlay()
{

}

void mitk::Overlay::SetProperty(const std::string& propertyKey,
                                     const BaseProperty::Pointer& propertyValue,
                                     const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, propertyValue);
  this->Modified();
}

void mitk::Overlay::ReplaceProperty(const std::string& propertyKey,
                                         const BaseProperty::Pointer& propertyValue,
                                         const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->ReplaceProperty(propertyKey, propertyValue);
}

void mitk::Overlay::AddProperty(const std::string& propertyKey,
                                     const BaseProperty::Pointer& propertyValue,
                                     const mitk::BaseRenderer* renderer,
                                     bool overwrite)
{
  if((overwrite) || (GetProperty(propertyKey, renderer) == NULL))
  {
    SetProperty(propertyKey, propertyValue, renderer);
  }
}

void mitk::Overlay::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  m_PropertyList->ConcatenatePropertyList(pList, replace);
}

mitk::BaseProperty* mitk::Overlay::GetProperty(const std::string& propertyKey, const mitk::BaseRenderer* renderer) const
{
  //renderer specified?
  if (renderer)
  {
    std::map<const mitk::BaseRenderer*,mitk::PropertyList::Pointer>::const_iterator it;
    //check for the renderer specific property
    it=m_MapOfPropertyLists.find(renderer);
    if(it!=m_MapOfPropertyLists.end()) //found
    {
      mitk::BaseProperty::Pointer property;
      property=it->second->GetProperty(propertyKey);
      if(property.IsNotNull())//found an enabled property in the render specific list
        return property;
      else //found a renderer specific list, but not the desired property
        return m_PropertyList->GetProperty(propertyKey); //return renderer unspecific property
    }
    else //didn't find the property list of the given renderer
    {
      //return the renderer unspecific property if there is one
      return m_PropertyList->GetProperty(propertyKey);
    }
  }
  else //no specific renderer given; use the renderer independent one
  {
    mitk::BaseProperty::Pointer property;
    property=m_PropertyList->GetProperty(propertyKey);
    if(property.IsNotNull())
      return property;
  }

  //only to satisfy compiler!
  return NULL;
}

bool mitk::Overlay::GetBoolProperty(const std::string&  propertyKey, bool& boolValue, mitk::BaseRenderer* renderer) const
{
  mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty*>(GetProperty(propertyKey, renderer));
  if(boolprop.IsNull())
    return false;

  boolValue = boolprop->GetValue();
  return true;
}

bool mitk::Overlay::GetIntProperty(const std::string&  propertyKey, int &intValue, mitk::BaseRenderer* renderer) const
{
  mitk::IntProperty::Pointer intprop = dynamic_cast<mitk::IntProperty*>(GetProperty(propertyKey, renderer));
  if(intprop.IsNull())
    return false;

  intValue = intprop->GetValue();
  return true;
}

bool mitk::Overlay::GetFloatProperty(const std::string&  propertyKey, float &floatValue, mitk::BaseRenderer* renderer) const
{
  mitk::FloatProperty::Pointer floatprop = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey, renderer));
  if(floatprop.IsNull())
    return false;

  floatValue = floatprop->GetValue();
  return true;
}

bool mitk::Overlay::GetStringProperty(const std::string&  propertyKey, std::string& string, mitk::BaseRenderer* renderer) const
{
  mitk::StringProperty::Pointer stringProp = dynamic_cast<mitk::StringProperty*>(GetProperty(propertyKey, renderer));
  if(stringProp.IsNull())
  {
    return false;
  }
  else
  {
    //memcpy((void*)string, stringProp->GetValue(), strlen(stringProp->GetValue()) + 1 ); // looks dangerous
    string = stringProp->GetValue();
    return true;
  }
}

void mitk::Overlay::SetIntProperty(const std::string&  propertyKey, int intValue, mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::IntProperty::New(intValue));
  Modified();
}
void mitk::Overlay::SetBoolProperty( const std::string&  propertyKey, bool boolValue, mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
  Modified();
}

void mitk::Overlay::SetFloatProperty( const std::string&  propertyKey, float floatValue, mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
  Modified();
}

void mitk::Overlay::SetStringProperty( const std::string&  propertyKey, const std::string&  stringValue, mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
  Modified();
}

std::string mitk::Overlay::GetName() const
{
  mitk::StringProperty* sp = dynamic_cast<mitk::StringProperty*>(this->GetProperty("name"));
  if (sp == NULL)
    return "";
  return sp->GetValue();
}

void mitk::Overlay::SetName(const std::string& name)
{
  this->SetStringProperty("name", name);
}

bool mitk::Overlay::GetName(std::string& nodeName, mitk::BaseRenderer* renderer, const std::string& propertyKey) const
{
  return GetStringProperty(propertyKey, nodeName, renderer);
}

void mitk::Overlay::SetText(std::string text)
{
  SetStringProperty("Overlay.Text", text.c_str());
}

std::string mitk::Overlay::GetText() const
{
  std::string text;
  GetPropertyList()->GetStringProperty("Overlay.Text", text);
  return text;
}

void mitk::Overlay::SetFontSize(int fontSize)
{
  SetIntProperty("Overlay.FontSize",fontSize);
}

int mitk::Overlay::GetFontSize() const
{
  int fontSize = 1;
  GetPropertyList()->GetIntProperty("Overlay.FontSize", fontSize);
  return fontSize;
}

bool mitk::Overlay::GetVisibility(bool& visible, mitk::BaseRenderer* renderer, const std::string& propertyKey) const
{
  return GetBoolProperty(propertyKey, visible, renderer);
}

bool mitk::Overlay::IsVisible(mitk::BaseRenderer* renderer, const std::string& propertyKey, bool defaultIsOn) const
{
  return IsOn(propertyKey, renderer, defaultIsOn);
}

bool mitk::Overlay::GetColor(float rgb[], mitk::BaseRenderer* renderer, const std::string&  propertyKey) const
{
  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetProperty(propertyKey, renderer));
  if(colorprop.IsNull())
    return false;

  memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
  return true;
}

void mitk::Overlay::SetColor(const mitk::Color &color, mitk::BaseRenderer* renderer, const std::string&  propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(color);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::Overlay::SetColor(float red, float green, float blue, mitk::BaseRenderer* renderer, const std::string&  propertyKey)
{
  float color[3];
  color[0]=red;
  color[1]=green;
  color[2]=blue;
  SetColor(color, renderer, propertyKey);
}

void mitk::Overlay::SetColor(const float rgb[], mitk::BaseRenderer* renderer, const std::string&  propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(rgb);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

bool mitk::Overlay::GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const std::string&  propertyKey) const
{
  mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey, renderer));
  if(opacityprop.IsNull())
    return false;

  opacity=opacityprop->GetValue();
  return true;
}

void mitk::Overlay::SetOpacity(float opacity, mitk::BaseRenderer* renderer, const std::string&  propertyKey)
{
  mitk::FloatProperty::Pointer prop;
  prop = mitk::FloatProperty::New(opacity);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::Overlay::SetVisibility(bool visible, mitk::BaseRenderer *renderer, const std::string& propertyKey)
{
  mitk::BoolProperty::Pointer prop;
  prop = mitk::BoolProperty::New(visible);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

mitk::PropertyList* mitk::Overlay::GetPropertyList(const mitk::BaseRenderer* renderer) const
{
  if(renderer==NULL)
    return m_PropertyList;

  mitk::PropertyList::Pointer & propertyList = m_MapOfPropertyLists[renderer];

  if(propertyList.IsNull())
    propertyList = mitk::PropertyList::New();

  assert(m_MapOfPropertyLists[renderer].IsNotNull());

  return propertyList;
}

bool mitk::Overlay::BaseLocalStorage::IsGenerateDataRequired(mitk::BaseRenderer *renderer, mitk::Overlay *overlay)
{
  if( m_LastGenerateDataTime < overlay -> GetMTime () )
    return true;

  if( renderer && m_LastGenerateDataTime < renderer -> GetTimeStepUpdateTime ( ) )
    return true;

  return false;
}

mitk::Overlay::Bounds mitk::Overlay::GetBoundsOnDisplay(mitk::BaseRenderer*) const
{
  mitk::Overlay::Bounds bounds;
  bounds.Position[0] = bounds.Position[1] = bounds.Size[0] = bounds.Size[1] = 0;
  return bounds;
}

void mitk::Overlay::SetBoundsOnDisplay(mitk::BaseRenderer*, const mitk::Overlay::Bounds&)
{
}

void mitk::Overlay::SetForceInForeground(bool forceForeground)
{
  m_ForceInForeground = forceForeground;
}

bool mitk::Overlay::IsForceInForeground() const
{
  return m_ForceInForeground;
}

mitk::Point2D mitk::Overlay::TransformDisplayPointToViewport( mitk::Point2D point , mitk::BaseRenderer* renderer)
{
  double* iViewport = renderer->GetVtkRenderer()->GetViewport();

  const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();

  float displayGeometryWidth = displayGeometry->GetSizeInDisplayUnits()[0];
  float displayGeometryHeight = displayGeometry->GetSizeInDisplayUnits()[1];

  float viewportWidth = (iViewport[2]-iViewport[0]) * displayGeometryWidth;
  float viewportHeight = (iViewport[3]-iViewport[1]) * displayGeometryHeight; // seemingly right

  float zoom = (iViewport[3]-iViewport[1]);

  point[0] +=
    0.5 * (viewportWidth/viewportHeight-1.0)*displayGeometryHeight
    - 0.5 * (displayGeometryWidth - displayGeometryHeight);

  point[0] *= zoom;
  point[1] *= zoom;
  return point;
}
