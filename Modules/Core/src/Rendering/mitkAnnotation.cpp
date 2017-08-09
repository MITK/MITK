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

#include "mitkAnnotation.h"
#include "usGetModuleContext.h"

const std::string mitk::Annotation::US_INTERFACE_NAME = "org.mitk.services.Annotation";
const std::string mitk::Annotation::US_PROPKEY_AnnotationNAME = US_INTERFACE_NAME + ".name";
const std::string mitk::Annotation::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string mitk::Annotation::US_PROPKEY_MODIFIED = US_INTERFACE_NAME + ".modified";
const std::string mitk::Annotation::US_PROPKEY_RENDERER_ID = US_INTERFACE_NAME + ".rendererId";
const std::string mitk::Annotation::US_PROPKEY_AR_ID = US_INTERFACE_NAME + ".arId";

mitk::Annotation::Annotation() : m_PropertyListModifiedObserverTag(0)
{
  m_PropertyList = mitk::PropertyList::New();
  itk::MemberCommand<mitk::Annotation>::Pointer _PropertyListModifiedCommand =
    itk::MemberCommand<mitk::Annotation>::New();
  _PropertyListModifiedCommand->SetCallbackFunction(this, &mitk::Annotation::PropertyListModified);
  m_PropertyListModifiedObserverTag = m_PropertyList->AddObserver(itk::ModifiedEvent(), _PropertyListModifiedCommand);
  this->SetName(this->GetNameOfClass());
  this->SetVisibility(true);
  this->SetOpacity(1.0);
}

void mitk::Annotation::PropertyListModified(const itk::Object * /*caller*/, const itk::EventObject &)
{
  AnnotationModified();
}

mitk::Annotation::~Annotation()
{
  this->UnRegisterMicroservice();
}

void mitk::Annotation::SetUSProperty(const std::string &propertyKey, us::Any value)
{
  if (this->m_ServiceRegistration)
  {
    us::ServiceProperties props;
    std::vector<std::string> propertyKeys;
    m_ServiceRegistration.GetReference().GetPropertyKeys(propertyKeys);
    for (std::string key : propertyKeys)
    {
      props[key] = m_ServiceRegistration.GetReference().GetProperty(key);
    }
    props[propertyKey] = value;
    m_ServiceRegistration.SetProperties(props);
  }
}

void mitk::Annotation::SetProperty(const std::string &propertyKey, const BaseProperty::Pointer &propertyValue)
{
  this->m_PropertyList->SetProperty(propertyKey, propertyValue);
}

void mitk::Annotation::ReplaceProperty(const std::string &propertyKey, const BaseProperty::Pointer &propertyValue)
{
  this->m_PropertyList->ReplaceProperty(propertyKey, propertyValue);
}

void mitk::Annotation::AddProperty(const std::string &propertyKey,
                                   const BaseProperty::Pointer &propertyValue,
                                   bool overwrite)
{
  if ((overwrite) || (GetProperty(propertyKey) == nullptr))
  {
    SetProperty(propertyKey, propertyValue);
  }
}

void mitk::Annotation::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  m_PropertyList->ConcatenatePropertyList(pList, replace);
}

mitk::BaseProperty *mitk::Annotation::GetProperty(const std::string &propertyKey) const
{
  mitk::BaseProperty::Pointer property = m_PropertyList->GetProperty(propertyKey);
  if (property.IsNotNull())
    return property;

  // only to satisfy compiler!
  return nullptr;
}

bool mitk::Annotation::GetBoolProperty(const std::string &propertyKey, bool &boolValue) const
{
  mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty *>(GetProperty(propertyKey));
  if (boolprop.IsNull())
    return false;

  boolValue = boolprop->GetValue();
  return true;
}

bool mitk::Annotation::GetIntProperty(const std::string &propertyKey, int &intValue) const
{
  mitk::IntProperty::Pointer intprop = dynamic_cast<mitk::IntProperty *>(GetProperty(propertyKey));
  if (intprop.IsNull())
    return false;

  intValue = intprop->GetValue();
  return true;
}

bool mitk::Annotation::GetFloatProperty(const std::string &propertyKey, float &floatValue) const
{
  mitk::FloatProperty::Pointer floatprop = dynamic_cast<mitk::FloatProperty *>(GetProperty(propertyKey));
  if (floatprop.IsNull())
    return false;

  floatValue = floatprop->GetValue();
  return true;
}

bool mitk::Annotation::GetStringProperty(const std::string &propertyKey, std::string &string) const
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

void mitk::Annotation::SetIntProperty(const std::string &propertyKey, int intValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::IntProperty::New(intValue));
  Modified();
}
void mitk::Annotation::SetBoolProperty(const std::string &propertyKey, bool boolValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
  Modified();
}

void mitk::Annotation::SetFloatProperty(const std::string &propertyKey, float floatValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
  Modified();
}

void mitk::Annotation::SetDoubleProperty(const std::string &propertyKey, double doubleValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::DoubleProperty::New(doubleValue));
  Modified();
}

void mitk::Annotation::SetStringProperty(const std::string &propertyKey, const std::string &stringValue)
{
  this->m_PropertyList->SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
  Modified();
}

std::string mitk::Annotation::GetName() const
{
  mitk::StringProperty *sp = dynamic_cast<mitk::StringProperty *>(this->GetProperty("name"));
  if (sp == nullptr)
    return "";
  return sp->GetValue();
}

void mitk::Annotation::SetName(const std::string &name)
{
  this->SetStringProperty("name", name);
}

bool mitk::Annotation::GetName(std::string &nodeName, const std::string &propertyKey) const
{
  return GetStringProperty(propertyKey, nodeName);
}

void mitk::Annotation::SetText(std::string text)
{
  SetStringProperty("Text", text.c_str());
}

std::string mitk::Annotation::GetText() const
{
  std::string text;
  GetStringProperty("Text", text);
  return text;
}

void mitk::Annotation::SetFontSize(int fontSize)
{
  SetIntProperty("FontSize", fontSize);
}

int mitk::Annotation::GetFontSize() const
{
  int fontSize = 1;
  GetIntProperty("FontSize", fontSize);
  return fontSize;
}

bool mitk::Annotation::GetVisibility(bool &visible, const std::string &propertyKey) const
{
  return GetBoolProperty(propertyKey, visible);
}

bool mitk::Annotation::IsVisible(const std::string &propertyKey, bool defaultIsOn) const
{
  return IsOn(propertyKey, defaultIsOn);
}

bool mitk::Annotation::GetColor(float rgb[], const std::string &propertyKey) const
{
  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty *>(GetProperty(propertyKey));
  if (colorprop.IsNull())
    return false;

  memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3 * sizeof(float));
  return true;
}

void mitk::Annotation::SetColor(const mitk::Color &color, const std::string &propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(color);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

void mitk::Annotation::SetColor(float red, float green, float blue, const std::string &propertyKey)
{
  float color[3];
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  SetColor(color, propertyKey);
}

void mitk::Annotation::SetColor(const float rgb[], const std::string &propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(rgb);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

bool mitk::Annotation::GetOpacity(float &opacity, const std::string &propertyKey) const
{
  mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty *>(GetProperty(propertyKey));
  if (opacityprop.IsNull())
    return false;

  opacity = opacityprop->GetValue();
  return true;
}

void mitk::Annotation::SetOpacity(float opacity, const std::string &propertyKey)
{
  mitk::FloatProperty::Pointer prop;
  prop = mitk::FloatProperty::New(opacity);
  this->m_PropertyList->SetProperty(propertyKey, prop);
}

void mitk::Annotation::SetVisibility(bool visible, const std::string &propertyKey)
{
  mitk::BoolProperty::Pointer prop;
  prop = mitk::BoolProperty::New(visible);
  this->m_PropertyList->SetProperty(propertyKey, prop);
  Modified();
}

bool mitk::Annotation::BaseLocalStorage::IsGenerateDataRequired(mitk::BaseRenderer *renderer,
                                                                mitk::Annotation *Annotation)
{
  if (m_LastGenerateDataTime < Annotation->GetMTime())
    return true;

  if (m_LastGenerateDataTime < Annotation->GetPropertyList()->GetMTime())
    return true;

  if (renderer && m_LastGenerateDataTime < renderer->GetTimeStepUpdateTime())
    return true;

  return false;
}

mitk::Annotation::Bounds mitk::Annotation::GetBoundsOnDisplay(mitk::BaseRenderer *) const
{
  mitk::Annotation::Bounds bounds;
  bounds.Position[0] = bounds.Position[1] = bounds.Size[0] = bounds.Size[1] = 0;
  return bounds;
}

void mitk::Annotation::SetBoundsOnDisplay(mitk::BaseRenderer *, const mitk::Annotation::Bounds &)
{
}

void mitk::Annotation::SetForceInForeground(bool forceForeground)
{
  m_ForceInForeground = forceForeground;
}

bool mitk::Annotation::IsForceInForeground() const
{
  return m_ForceInForeground;
}

mitk::PropertyList *mitk::Annotation::GetPropertyList() const
{
  return m_PropertyList;
}

std::string mitk::Annotation::GetMicroserviceID()
{
  return this->m_ServiceRegistration.GetReference().GetProperty(US_PROPKEY_ID).ToString();
}

void mitk::Annotation::RegisterAsMicroservice(us::ServiceProperties props)
{
  if (m_ServiceRegistration != nullptr)
    m_ServiceRegistration.Unregister();
  us::ModuleContext *context = us::GetModuleContext();
  // Define ServiceProps
  mitk::UIDGenerator uidGen = mitk::UIDGenerator("org.mitk.services.Annotation.id_", 16);
  props[US_PROPKEY_ID] = uidGen.GetUID();
  m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::Annotation::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != nullptr)
    m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

void mitk::Annotation::AnnotationModified()
{
  Modified();
  this->SetUSProperty(US_PROPKEY_MODIFIED, this->GetMTime());
}
