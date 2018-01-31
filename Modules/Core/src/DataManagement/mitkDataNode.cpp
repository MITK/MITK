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

#include "mitkDataNode.h"
#include "mitkCoreObjectFactory.h"
#include <vtkTransform.h>

#include "mitkGroupTagProperty.h"
#include "mitkProperties.h"
#include "mitkSmartPointerProperty.h"
#include "mitkStringProperty.h"
//#include "mitkMaterialProperty.h"
#include "mitkColorProperty.h"
#include "mitkCoreObjectFactory.h"
#include "mitkGenericProperty.h"
#include "mitkGeometry3D.h"
#include "mitkImageSource.h"
#include "mitkLevelWindowProperty.h"
#include "mitkRenderingManager.h"

mitk::Mapper *mitk::DataNode::GetMapper(MapperSlotId id) const
{
  if ((id >= m_Mappers.size()) || (m_Mappers[id].IsNull()))
  {
    if (id >= m_Mappers.capacity())
    {
      //      int i, size=id-m_Mappers.capacity()+10;
      m_Mappers.resize(id + 10);
    }
    m_Mappers[id] = CoreObjectFactory::GetInstance()->CreateMapper(const_cast<DataNode *>(this), id);
  }
  return m_Mappers[id];
}

mitk::BaseData *mitk::DataNode::GetData() const
{
  return m_Data;
}

void mitk::DataNode::SetData(mitk::BaseData *baseData)
{
  if (m_Data != baseData)
  {
    m_Mappers.clear();
    m_Mappers.resize(10);

    if (m_Data.IsNotNull() && baseData != nullptr)
    {
      // Do previous and new data have same type? Keep existing properties.
      if (0 == strcmp(m_Data->GetNameOfClass(), baseData->GetNameOfClass()))
      {
        m_Data = baseData;
      }
      else
      {
        m_Data = baseData;
        this->GetPropertyList()->Clear();
        mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties(this);
      }
    }
    else
    {
      m_Data = baseData;
      mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties(this);
    }

    m_DataReferenceChangedTime.Modified();
    Modified();
  }
}

mitk::DataNode::DataNode()
  : m_PropertyList(PropertyList::New()),
    m_PropertyListModifiedObserverTag(0)
{
  m_Mappers.resize(10);

  // subscribe for modified event
  itk::MemberCommand<mitk::DataNode>::Pointer _PropertyListModifiedCommand = itk::MemberCommand<mitk::DataNode>::New();
  _PropertyListModifiedCommand->SetCallbackFunction(this, &mitk::DataNode::PropertyListModified);
  m_PropertyListModifiedObserverTag = m_PropertyList->AddObserver(itk::ModifiedEvent(), _PropertyListModifiedCommand);
}

mitk::DataNode::~DataNode()
{
  if (m_PropertyList.IsNotNull())
    m_PropertyList->RemoveObserver(m_PropertyListModifiedObserverTag);

  m_Mappers.clear();
  m_Data = nullptr;
}

mitk::DataNode &mitk::DataNode::operator=(const DataNode &right)
{
  mitk::DataNode *node = mitk::DataNode::New();
  node->SetData(right.GetData());
  return *node;
}

mitk::DataNode &mitk::DataNode::operator=(mitk::BaseData *right)
{
  mitk::DataNode *node = mitk::DataNode::New();
  node->SetData(right);
  return *node;
}

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::istream &mitk::operator>>(MBI_STD::istream &i, mitk::DataNode::Pointer &dtn)
#endif
#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
  MBI_STD::istream &
  operator>>(MBI_STD::istream &i, mitk::DataNode::Pointer &dtn)
#endif
{
  dtn = mitk::DataNode::New();
  // i >> av.get();
  return i;
}

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::ostream &mitk::operator<<(MBI_STD::ostream &o, mitk::DataNode::Pointer &dtn)
#endif
#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
  MBI_STD::ostream &
  operator<<(MBI_STD::ostream &o, mitk::DataNode::Pointer &dtn)
#endif
{
  if (dtn->GetData() != nullptr)
    o << dtn->GetData()->GetNameOfClass();
  else
    o << "empty data";
  return o;
}

void mitk::DataNode::SetMapper(MapperSlotId id, mitk::Mapper *mapper)
{
  m_Mappers[id] = mapper;

  if (mapper != nullptr)
    mapper->SetDataNode(this);
}

void mitk::DataNode::UpdateOutputInformation()
{
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }
}

void mitk::DataNode::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::DataNode::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::DataNode::VerifyRequestedRegion()
{
  return true;
}

void mitk::DataNode::SetRequestedRegion(const itk::DataObject * /*data*/)
{
}

mitk::DataNode::PropertyListKeyNames mitk::DataNode::GetPropertyListNames() const
{
  PropertyListKeyNames result;

  for (auto entries : m_MapOfPropertyLists)
    result.push_back(entries.first);

  return result;
}

void mitk::DataNode::CopyInformation(const itk::DataObject * /*data*/)
{
}
mitk::PropertyList *mitk::DataNode::GetPropertyList(const mitk::BaseRenderer *renderer) const
{
  if (renderer == nullptr)
    return m_PropertyList;

  return this->GetPropertyList(renderer->GetName());
}

mitk::PropertyList *mitk::DataNode::GetPropertyList(const std::string &rendererName) const
{
  if (rendererName.empty())
    return m_PropertyList;

  mitk::PropertyList::Pointer &propertyList = m_MapOfPropertyLists[rendererName];

  if (propertyList.IsNull())
    propertyList = mitk::PropertyList::New();

  assert(m_MapOfPropertyLists[rendererName].IsNotNull());

  return propertyList;
}

void mitk::DataNode::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  m_PropertyList->ConcatenatePropertyList(pList, replace);
}

mitk::BaseProperty *mitk::DataNode::GetProperty(const char *propertyKey, const mitk::BaseRenderer *renderer, bool fallBackOnDataProperties) const
{
  if (nullptr == propertyKey)
    return nullptr;

  if (nullptr != renderer)
  {
    auto it = m_MapOfPropertyLists.find(renderer->GetName());

    if (m_MapOfPropertyLists.end() != it)
    {
      auto property = it->second->GetProperty(propertyKey);

      if (nullptr != property)
        return property;
    }
  }

  auto property = m_PropertyList->GetProperty(propertyKey);

  if (nullptr == property && fallBackOnDataProperties && m_Data.IsNotNull())
    property = m_Data->GetProperty(propertyKey);

  return property;
}

mitk::DataNode::GroupTagList mitk::DataNode::GetGroupTags() const
{
  GroupTagList groups;
  const PropertyList::PropertyMap *propertyMap = m_PropertyList->GetMap();

  for (auto groupIter =
         propertyMap->begin(); // m_PropertyList is created in the constructor, so we don't check it here
       groupIter != propertyMap->end();
       ++groupIter)
  {
    const BaseProperty *bp = groupIter->second;
    if (dynamic_cast<const GroupTagProperty *>(bp))
    {
      groups.insert(groupIter->first);
    }
  }

  return groups;
}

bool mitk::DataNode::GetBoolProperty(const char *propertyKey, bool &boolValue, const mitk::BaseRenderer *renderer) const
{
  mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty *>(GetProperty(propertyKey, renderer));
  if (boolprop.IsNull())
    return false;

  boolValue = boolprop->GetValue();
  return true;
}

bool mitk::DataNode::GetIntProperty(const char *propertyKey, int &intValue, const mitk::BaseRenderer *renderer) const
{
  mitk::IntProperty::Pointer intprop = dynamic_cast<mitk::IntProperty *>(GetProperty(propertyKey, renderer));
  if (intprop.IsNull())
    return false;

  intValue = intprop->GetValue();
  return true;
}

bool mitk::DataNode::GetFloatProperty(const char *propertyKey,
                                      float &floatValue,
                                      const mitk::BaseRenderer *renderer) const
{
  mitk::FloatProperty::Pointer floatprop = dynamic_cast<mitk::FloatProperty *>(GetProperty(propertyKey, renderer));
  if (floatprop.IsNull())
    return false;

  floatValue = floatprop->GetValue();
  return true;
}

bool mitk::DataNode::GetDoubleProperty(const char *propertyKey,
                                       double &doubleValue,
                                       const mitk::BaseRenderer *renderer) const
{
  mitk::DoubleProperty::Pointer doubleprop = dynamic_cast<mitk::DoubleProperty *>(GetProperty(propertyKey, renderer));
  if (doubleprop.IsNull())
  {
    // try float instead
    float floatValue = 0;
    if (this->GetFloatProperty(propertyKey, floatValue, renderer))
    {
      doubleValue = floatValue;
      return true;
    }
    return false;
  }

  doubleValue = doubleprop->GetValue();
  return true;
}

bool mitk::DataNode::GetStringProperty(const char *propertyKey,
                                       std::string &string,
                                       const mitk::BaseRenderer *renderer) const
{
  mitk::StringProperty::Pointer stringProp = dynamic_cast<mitk::StringProperty *>(GetProperty(propertyKey, renderer));
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

bool mitk::DataNode::GetColor(float rgb[3], const mitk::BaseRenderer *renderer, const char *propertyKey) const
{
  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty *>(GetProperty(propertyKey, renderer));
  if (colorprop.IsNull())
    return false;

  memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3 * sizeof(float));
  return true;
}

bool mitk::DataNode::GetOpacity(float &opacity, const mitk::BaseRenderer *renderer, const char *propertyKey) const
{
  mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty *>(GetProperty(propertyKey, renderer));
  if (opacityprop.IsNull())
    return false;

  opacity = opacityprop->GetValue();
  return true;
}

bool mitk::DataNode::GetLevelWindow(mitk::LevelWindow &levelWindow,
                                    const mitk::BaseRenderer *renderer,
                                    const char *propertyKey) const
{
  mitk::LevelWindowProperty::Pointer levWinProp =
    dynamic_cast<mitk::LevelWindowProperty *>(GetProperty(propertyKey, renderer));
  if (levWinProp.IsNull())
    return false;

  levelWindow = levWinProp->GetLevelWindow();
  return true;
}

void mitk::DataNode::SetColor(const mitk::Color &color, const mitk::BaseRenderer *renderer, const char *propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(color);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetColor(
  float red, float green, float blue, const mitk::BaseRenderer *renderer, const char *propertyKey)
{
  float color[3];
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  SetColor(color, renderer, propertyKey);
}

void mitk::DataNode::SetColor(const float rgb[3], const mitk::BaseRenderer *renderer, const char *propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(rgb);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetVisibility(bool visible, const mitk::BaseRenderer *renderer, const char *propertyKey)
{
  mitk::BoolProperty::Pointer prop;
  prop = mitk::BoolProperty::New(visible);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetOpacity(float opacity, const mitk::BaseRenderer *renderer, const char *propertyKey)
{
  mitk::FloatProperty::Pointer prop;
  prop = mitk::FloatProperty::New(opacity);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetLevelWindow(mitk::LevelWindow levelWindow,
                                    const mitk::BaseRenderer *renderer,
                                    const char *propertyKey)
{
  mitk::LevelWindowProperty::Pointer prop;
  prop = mitk::LevelWindowProperty::New(levelWindow);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetIntProperty(const char *propertyKey, int intValue, const mitk::BaseRenderer *renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::IntProperty::New(intValue));
}
void mitk::DataNode::SetBoolProperty(const char *propertyKey,
                                     bool boolValue,
                                     const mitk::BaseRenderer *renderer /*=nullptr*/)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
}

void mitk::DataNode::SetFloatProperty(const char *propertyKey,
                                      float floatValue,
                                      const mitk::BaseRenderer *renderer /*=nullptr*/)
{
  if (dynamic_cast<DoubleProperty *>(this->GetProperty(propertyKey, renderer)) != nullptr)
  {
    MITK_WARN << "Setting float property " << propertyKey
              << " although a double property with the same name already exists";
  }
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
}

void mitk::DataNode::SetDoubleProperty(const char *propertyKey, double doubleValue, const mitk::BaseRenderer *renderer)
{
  if (dynamic_cast<FloatProperty *>(this->GetProperty(propertyKey, renderer)) != nullptr)
  {
    MITK_WARN << "Setting double property " << propertyKey
              << " although a float property with the same name already exists";
  }
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::DoubleProperty::New(doubleValue));
}

void mitk::DataNode::SetStringProperty(const char *propertyKey,
                                       const char *stringValue,
                                       const mitk::BaseRenderer *renderer /*=nullptr*/)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
}

void mitk::DataNode::SetProperty(const char *propertyKey,
                                 BaseProperty *propertyValue,
                                 const mitk::BaseRenderer *renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, propertyValue);
}

void mitk::DataNode::ReplaceProperty(const char *propertyKey,
                                     BaseProperty *propertyValue,
                                     const mitk::BaseRenderer *renderer)
{
  GetPropertyList(renderer)->ReplaceProperty(propertyKey, propertyValue);
}

void mitk::DataNode::AddProperty(const char *propertyKey,
                                 BaseProperty *propertyValue,
                                 const mitk::BaseRenderer *renderer,
                                 bool overwrite)
{
  if ((overwrite) || (GetProperty(propertyKey, renderer) == nullptr))
  {
    SetProperty(propertyKey, propertyValue, renderer);
  }
}

vtkLinearTransform *mitk::DataNode::GetVtkTransform(int t) const
{
  assert(m_Data.IsNotNull());

  mitk::BaseGeometry *geometry = m_Data->GetGeometry(t);

  if (geometry == nullptr)
    return nullptr;

  return geometry->GetVtkTransform();
}

unsigned long mitk::DataNode::GetMTime() const
{
  unsigned long time = Superclass::GetMTime();
  if (m_Data.IsNotNull())
  {
    if ((time < m_Data->GetMTime()) || ((m_Data->GetSource().IsNotNull()) && (time < m_Data->GetSource()->GetMTime())))
    {
      Modified();
      return Superclass::GetMTime();
    }
  }
  return time;
}

void mitk::DataNode::SetSelected(bool selected, const mitk::BaseRenderer *renderer)
{
  mitk::BoolProperty::Pointer selectedProperty = dynamic_cast<mitk::BoolProperty *>(GetProperty("selected"));

  if (selectedProperty.IsNull())
  {
    selectedProperty = mitk::BoolProperty::New();
    selectedProperty->SetValue(false);
    SetProperty("selected", selectedProperty, renderer);
  }

  if (selectedProperty->GetValue() != selected)
  {
    selectedProperty->SetValue(selected);
    itk::ModifiedEvent event;
    InvokeEvent(event);
  }
}

/*
class SelectedEvent : public itk::ModifiedEvent
{
public:
  typedef SelectedEvent Self;
  typedef itk::ModifiedEvent Superclass;

  SelectedEvent(DataNode* dataNode)
    { m_DataNode = dataNode; };
  DataNode* GetDataNode()
    { return m_DataNode; };
  virtual const char * GetEventName() const
    { return "SelectedEvent"; }
  virtual bool CheckEvent(const ::itk::EventObject* e) const
    { return dynamic_cast<const Self*>(e); }
  virtual ::itk::EventObject* MakeObject() const
    { return new Self(m_DataNode); }
private:
  DataNode* m_DataNode;
  SelectedEvent(const Self& event)
    { m_DataNode = event.m_DataNode; };
  void operator=(const Self& event)
  { m_DataNode = event.m_DataNode; }
};
*/

bool mitk::DataNode::IsSelected(const mitk::BaseRenderer *renderer)
{
  bool selected;

  if (!GetBoolProperty("selected", selected, renderer))
    return false;

  return selected;
}

void mitk::DataNode::SetDataInteractor(const DataInteractor::Pointer interactor)
{
  if (m_DataInteractor == interactor)
    return;

  m_DataInteractor = interactor;
  this->Modified();

  mitk::DataNode::InteractorChangedEvent changedEvent;
  this->InvokeEvent(changedEvent);
}

mitk::DataInteractor::Pointer mitk::DataNode::GetDataInteractor() const
{
  return m_DataInteractor;
}

void mitk::DataNode::PropertyListModified(const itk::Object * /*caller*/, const itk::EventObject &)
{
  Modified();
}

mitk::BaseProperty::ConstPointer mitk::DataNode::GetConstProperty(const std::string &propertyKey, const std::string &contextName, bool fallBackOnDefaultContext) const
{
  if (propertyKey.empty())
    return nullptr;

  if (!contextName.empty())
  {
    auto propertyListIter = m_MapOfPropertyLists.find(contextName);

    if (m_MapOfPropertyLists.end() != propertyListIter)
    {
      BaseProperty::ConstPointer property = propertyListIter->second->GetProperty(propertyKey);

      if (property.IsNotNull())
        return property;
    }
  }

  if (contextName.empty() || fallBackOnDefaultContext)
  {
    BaseProperty::ConstPointer property = m_PropertyList->GetProperty(propertyKey);

    if (property.IsNull() && m_Data.IsNotNull())
      property = m_Data->GetProperty(propertyKey.c_str());

    return property;
  }

  return nullptr;
}

mitk::BaseProperty * mitk::DataNode::GetNonConstProperty(const std::string &propertyKey, const std::string &contextName, bool fallBackOnDefaultContext)
{
  if (propertyKey.empty())
    return nullptr;

  if (!contextName.empty())
  {
    auto propertyListIter = m_MapOfPropertyLists.find(contextName);

    if (m_MapOfPropertyLists.end() != propertyListIter)
    {
      auto property = propertyListIter->second->GetProperty(propertyKey);

      if (nullptr != property)
        return property;
    }
  }

  if (contextName.empty() || fallBackOnDefaultContext)
  {
    auto property = m_PropertyList->GetProperty(propertyKey);

    if (nullptr == property && m_Data.IsNotNull())
      property = m_Data->GetProperty(propertyKey.c_str());

    return property;
  }

  return nullptr;
}

void mitk::DataNode::SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName, bool fallBackOnDefaultContext)
{
  if (propertyKey.empty())
    mitkThrow() << "Property key is empty.";

  if (!contextName.empty())
  {
    auto propertyListIter = m_MapOfPropertyLists.find(contextName);

    if (m_MapOfPropertyLists.end() != propertyListIter)
    {
      propertyListIter->second->SetProperty(propertyKey, property);
      return;
    }
  }

  if (contextName.empty() || fallBackOnDefaultContext)
  {
    m_PropertyList->SetProperty(propertyKey, property);
    return;
  }

  mitkThrow() << "Unknown property context.";
}

void mitk::DataNode::RemoveProperty(const std::string &propertyKey, const std::string &contextName, bool fallBackOnDefaultContext)
{
  if (propertyKey.empty())
    mitkThrow() << "Property key is empty.";

  if (!contextName.empty())
  {
    auto propertyListIter = m_MapOfPropertyLists.find(contextName);

    if (m_MapOfPropertyLists.end() != propertyListIter)
    {
      propertyListIter->second->RemoveProperty(propertyKey);
      return;
    }
  }

  if (contextName.empty() || fallBackOnDefaultContext)
  {
    m_PropertyList->RemoveProperty(propertyKey);
    return;
  }

  mitkThrow() << "Unknown property context.";
}

std::vector<std::string> mitk::DataNode::GetPropertyKeys(const std::string &contextName, bool includeDefaultContext) const
{
  std::vector<std::string> propertyKeys;

  if (contextName.empty())
  {
    for (auto property : *m_PropertyList->GetMap())
      propertyKeys.push_back(property.first);

    return propertyKeys;
  }

  auto propertyListIter = m_MapOfPropertyLists.find(contextName);

  if (m_MapOfPropertyLists.end() != propertyListIter)
  {
    for (auto property : *propertyListIter->second->GetMap())
      propertyKeys.push_back(property.first);
  }

  if (includeDefaultContext)
  {
    for (auto property : *m_PropertyList->GetMap())
    {
      auto propertyKeyIter = std::find(propertyKeys.begin(), propertyKeys.end(), property.first);

      if (propertyKeys.end() == propertyKeyIter)
        propertyKeys.push_back(property.first);
    }
  }

  return propertyKeys;
}

std::vector<std::string> mitk::DataNode::GetPropertyContextNames() const
{
  return this->GetPropertyListNames();
}
