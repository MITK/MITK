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
#include "usGetModuleContext.h"

const std::string mitk::Overlay::US_INTERFACE_NAME = "org.mitk.services.Overlay";
const std::string mitk::Overlay::US_PROPKEY_OVERLAYNAME = US_INTERFACE_NAME + ".name";
const std::string mitk::Overlay::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string mitk::Overlay::US_PROPKEY_RENDERER_ID = US_INTERFACE_NAME + ".rendererId";
const std::string mitk::Overlay::US_PROPKEY_AR_ID = US_INTERFACE_NAME + ".arId";

mitk::Overlay::Overlay() : m_LayoutedBy(nullptr)
{
  m_PropertyList = mitk::PropertyList::New();
  this->SetName(this->GetNameOfClass());
  this->SetVisibility(true);
  this->SetOpacity(1.0);
}

mitk::Overlay::~Overlay()
{
  this->UnRegisterMicroservice();
}

void mitk::Overlay::SetUSProperty(const std::string & /*propertyKey*/, us::Any /*value*/)
{
  if (this->m_ServiceRegistration)
  {
    //        m_ServiceRegistration.SetProperties( //TODO19786
  }
}

void mitk::Overlay::SetProperty(const std::string &propertyKey, const BaseProperty::Pointer &propertyValue)
{
  this->m_PropertyList->SetProperty(propertyKey, propertyValue);
}

void mitk::Overlay::ReplaceProperty(const std::string &propertyKey, const BaseProperty::Pointer &propertyValue)
{
  this->m_PropertyList->ReplaceProperty(propertyKey, propertyValue);
}

void mitk::Overlay::AddProperty(const std::string &propertyKey,
                                const BaseProperty::Pointer &propertyValue,
                                bool overwrite)
{
  if ((overwrite) || (GetProperty(propertyKey) == NULL))
  {
    SetProperty(propertyKey, propertyValue);
  }
}

void mitk::Overlay::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  m_PropertyList->ConcatenatePropertyList(pList, replace);
}

mitk::BaseProperty *mitk::Overlay::GetProperty(const std::string &propertyKey) const
{
  mitk::BaseProperty::Pointer property = m_PropertyList->GetProperty(propertyKey);
  if (property.IsNotNull())
    return property;

  // only to satisfy compiler!
  return NULL;
}

bool mitk::Overlay::GetBoolProperty(const std::string &propertyKey, bool &boolValue) const
{
  mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty *>(GetProperty(propertyKey));
  if (boolprop.IsNull())
    return false;

  boolValue = boolprop->GetValue();
  return true;
}

bool mitk::Overlay::GetIntProperty(const std::string &propertyKey, int &intValue) const
{
  mitk::IntProperty::Pointer intprop = dynamic_cast<mitk::IntProperty *>(GetProperty(propertyKey));
  if (intprop.IsNull())
    return false;

  intValue = intprop->GetValue();
  return true;
}

bool mitk::Overlay::GetFloatProperty(const std::string &propertyKey, float &floatValue) const
{
  mitk::FloatProperty::Pointer floatprop = dynamic_cast<mitk::FloatProperty *>(GetProperty(propertyKey));
  if (floatprop.IsNull())
    return false;

  floatValue = floatprop->GetValue();
  return true;
}

bool mitk::Overlay::GetStringProperty(const std::string &propertyKey, std::string &string) const
{
  mitk::StringProperty::Pointer stringProp = dynamic_cast<mitk::StringProperty *>(GetProperty(propertyKey));
  if (stringProp.IsNull())
  {
    return false;
  }
  else
  {
    // memcpy((void*)string, stringProp->GetValue(), strlen(stringProp->GetValue()) + 1 ); // looks dangerous
    string = stringProp->GetValue();
    return true;
  }
}

void mitk::Overlay::SetIntProperty(const std::string &propertyKey, int intValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::IntProperty::New(intValue));
  Modified();
}
void mitk::Overlay::SetBoolProperty(const std::string &propertyKey, bool boolValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
  Modified();
}

void mitk::Overlay::SetFloatProperty(const std::string &propertyKey, float floatValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
  Modified();
}

void mitk::Overlay::SetStringProperty(const std::string &propertyKey, const std::string &stringValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
  Modified();
}

std::string mitk::Overlay::GetName() const
{
  mitk::StringProperty *sp = dynamic_cast<mitk::StringProperty *>(this->GetProperty("name"));
  if (sp == NULL)
    return "";
  return sp->GetValue();
}

void mitk::Overlay::SetName(const std::string &name)
{
  this->SetStringProperty("name", name);
}

bool mitk::Overlay::GetName(std::string &nodeName, const std::string &propertyKey) const
{
  return GetStringProperty(propertyKey, nodeName);
}

void mitk::Overlay::SetText(std::string text)
{
  SetStringProperty("Text", text.c_str());
}

std::string mitk::Overlay::GetText() const
{
  std::string text;
  GetStringProperty("Text", text);
  return text;
}

void mitk::Overlay::SetFontSize(int fontSize)
{
  SetIntProperty("FontSize", fontSize);
}

int mitk::Overlay::GetFontSize() const
{
  int fontSize = 1;
  GetIntProperty("FontSize", fontSize);
  return fontSize;
}

bool mitk::Overlay::GetVisibility(bool &visible, const std::string &propertyKey) const
{
  return GetBoolProperty(propertyKey, visible);
}

bool mitk::Overlay::IsVisible(const std::string &propertyKey, bool defaultIsOn) const
{
  return IsOn(propertyKey, defaultIsOn);
}

bool mitk::Overlay::GetColor(float rgb[], const std::string &propertyKey) const
{
  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty *>(GetProperty(propertyKey));
  if (colorprop.IsNull())
    return false;

  memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3 * sizeof(float));
  return true;
}

void mitk::Overlay::SetColor(const mitk::Color &color, const std::string &propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(color);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

void mitk::Overlay::SetColor(float red, float green, float blue, const std::string &propertyKey)
{
  float color[3];
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  SetColor(color, propertyKey);
}

void mitk::Overlay::SetColor(const float rgb[], const std::string &propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(rgb);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

bool mitk::Overlay::GetOpacity(float &opacity, const std::string &propertyKey) const
{
  mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty *>(GetProperty(propertyKey));
  if (opacityprop.IsNull())
    return false;

  opacity = opacityprop->GetValue();
  return true;
}

void mitk::Overlay::SetOpacity(float opacity, const std::string &propertyKey)
{
  mitk::FloatProperty::Pointer prop;
  prop = mitk::FloatProperty::New(opacity);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

void mitk::Overlay::SetVisibility(bool visible, const std::string &propertyKey)
{
  mitk::BoolProperty::Pointer prop;
  prop = mitk::BoolProperty::New(visible);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

bool mitk::Overlay::BaseLocalStorage::IsGenerateDataRequired(mitk::BaseRenderer *renderer, mitk::Overlay *overlay)
{
  if (m_LastGenerateDataTime < overlay->GetMTime())
    return true;

  if (m_LastGenerateDataTime < overlay->GetPropertyList()->GetMTime())
    return true;

  if (renderer && m_LastGenerateDataTime < renderer->GetTimeStepUpdateTime())
    return true;

  return false;
}

mitk::Overlay::Bounds mitk::Overlay::GetBoundsOnDisplay(mitk::BaseRenderer *) const
{
  mitk::Overlay::Bounds bounds;
  bounds.Position[0] = bounds.Position[1] = bounds.Size[0] = bounds.Size[1] = 0;
  return bounds;
}

void mitk::Overlay::SetBoundsOnDisplay(mitk::BaseRenderer *, const mitk::Overlay::Bounds &)
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

mitk::PropertyList *mitk::Overlay::GetPropertyList() const
{
  return m_PropertyList;
}

void mitk::Overlay::RegisterAsMicroservice(us::ServiceProperties props)
{
  if (m_ServiceRegistration != NULL)
    m_ServiceRegistration.Unregister();
  us::ModuleContext *context = us::GetModuleContext();
  // Define ServiceProps
  mitk::UIDGenerator uidGen = mitk::UIDGenerator("org.mitk.services.Overlay.id_", 16);
  props[US_PROPKEY_ID] = uidGen.GetUID();
  m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::Overlay::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != NULL)
    m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}
