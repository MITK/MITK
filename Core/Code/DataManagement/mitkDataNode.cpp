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

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkSmartPointerProperty.h"
//#include "mitkMaterialProperty.h"
#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkGeometry3D.h"
#include "mitkRenderingManager.h"
#include "mitkGlobalInteraction.h"
#include "mitkEventMapper.h"
#include "mitkGenericProperty.h"
#include "mitkImageSource.h"
#include "mitkCoreObjectFactory.h"



mitk::Mapper* mitk::DataNode::GetMapper(MapperSlotId id) const
{
  if( (id >= m_Mappers.size()) || (m_Mappers[id].IsNull()) )
  {
    if(id >= m_Mappers.capacity())
    {
//      int i, size=id-m_Mappers.capacity()+10;
      m_Mappers.resize(id+10);
    }
    m_Mappers[id] = CoreObjectFactory::GetInstance()->CreateMapper(const_cast<DataNode*>(this),id);
  }
  return m_Mappers[id];
}

mitk::BaseData* mitk::DataNode::GetData() const
{
  return m_Data;
}

mitk::Interactor* mitk::DataNode::GetInteractor() const
{
  return m_Interactor;
}

void mitk::DataNode::SetData(mitk::BaseData* baseData)
{
  if(m_Data!=baseData)
  {
    m_Data=baseData;

    m_Mappers.clear();
    m_Mappers.resize(10);

    mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties(this);

    m_DataReferenceChangedTime.Modified();
    Modified();

    //inform the interactor about the change
    if (m_Interactor.IsNotNull())
      m_Interactor->DataChanged();
  }
}

void mitk::DataNode::SetInteractor(mitk::Interactor* interactor)
{
  m_Interactor = interactor;
  if(m_Interactor.IsNotNull())
    m_Interactor->SetDataNode(this);
}

mitk::DataNode::DataNode() : m_Data(NULL), m_PropertyListModifiedObserverTag(0)
{
  m_Mappers.resize(10);

  m_PropertyList = PropertyList::New();

  // subscribe for modified event
  itk::MemberCommand<mitk::DataNode>::Pointer _PropertyListModifiedCommand =
    itk::MemberCommand<mitk::DataNode>::New();
  _PropertyListModifiedCommand->SetCallbackFunction(this, &mitk::DataNode::PropertyListModified);
  m_PropertyListModifiedObserverTag = m_PropertyList->AddObserver(itk::ModifiedEvent(), _PropertyListModifiedCommand);
}


mitk::DataNode::~DataNode()
{
  if(m_PropertyList.IsNotNull())
    // remove modified event listener
    m_PropertyList->RemoveObserver(m_PropertyListModifiedObserverTag);

  Interactor* interactor = this->GetInteractor();

  if ( interactor )
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( interactor );
  }
  m_Mappers.clear();
  m_Data = NULL;
}

mitk::DataNode& mitk::DataNode::operator=(const DataNode& right)
{
  mitk::DataNode* node=mitk::DataNode::New();
  node->SetData(right.GetData());
  return *node;
}

mitk::DataNode& mitk::DataNode::operator=(mitk::BaseData* right)
{
  mitk::DataNode* node=mitk::DataNode::New();
  node->SetData(right);
  return *node;
}

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::istream& mitk::operator>>( MBI_STD::istream& i, mitk::DataNode::Pointer& dtn )
#endif
#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MBI_STD::istream& operator>>( MBI_STD::istream& i, mitk::DataNode::Pointer& dtn )
#endif
{
  dtn = mitk::DataNode::New();
  //i >> av.get();
  return i;
}

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::ostream& mitk::operator<<( MBI_STD::ostream& o, mitk::DataNode::Pointer& dtn)
#endif
#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MBI_STD::ostream& operator<<( MBI_STD::ostream& o, mitk::DataNode::Pointer& dtn)
#endif
{
  if(dtn->GetData()!=NULL)
    o<<dtn->GetData()->GetNameOfClass();
  else
    o<<"empty data";
  return o;
}

void mitk::DataNode::SetMapper(MapperSlotId id, mitk::Mapper* mapper)
{
  m_Mappers[id] = mapper;

  if (mapper!=NULL)
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

void mitk::DataNode::SetRequestedRegion( const itk::DataObject * /*data*/)
{
}

void mitk::DataNode::CopyInformation(const itk::DataObject * /*data*/)
{
}
mitk::PropertyList* mitk::DataNode::GetPropertyList(const mitk::BaseRenderer* renderer) const
{
  if(renderer==NULL)
    return m_PropertyList;

  return this->GetPropertyList(renderer->GetName());
}

mitk::PropertyList* mitk::DataNode::GetPropertyList(const std::string& rendererName) const
{
  if (rendererName.empty())
    return m_PropertyList;

  mitk::PropertyList::Pointer & propertyList = m_MapOfPropertyLists[rendererName];

  if(propertyList.IsNull())
    propertyList = mitk::PropertyList::New();

  assert(m_MapOfPropertyLists[rendererName].IsNotNull());

  return propertyList;
}

void mitk::DataNode::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  m_PropertyList->ConcatenatePropertyList(pList, replace);
}

mitk::BaseProperty* mitk::DataNode::GetProperty(const char *propertyKey, const mitk::BaseRenderer* renderer) const
{
  if(propertyKey==NULL)
    return NULL;

  //renderer specified?
  if (renderer)
  {
    std::string rendererName = renderer->GetName();

    MapOfPropertyLists::const_iterator it;
    //check for the renderer specific property
    it=m_MapOfPropertyLists.find(rendererName);
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

mitk::DataNode::GroupTagList mitk::DataNode::GetGroupTags() const
{
  GroupTagList groups;
  const PropertyList::PropertyMap* propertyMap = m_PropertyList->GetMap();

  for ( PropertyList::PropertyMap::const_iterator groupIter = propertyMap->begin(); // m_PropertyList is created in the constructor, so we don't check it here
        groupIter != propertyMap->end();
        ++groupIter )
  {
    const BaseProperty* bp = groupIter->second;
    if ( dynamic_cast<const GroupTagProperty*>(bp) )
    {
      groups.insert( groupIter->first );
    }
  }

  return groups;
}

bool mitk::DataNode::GetBoolProperty(const char* propertyKey, bool& boolValue, const mitk::BaseRenderer* renderer) const
{
  mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty*>(GetProperty(propertyKey, renderer));
  if(boolprop.IsNull())
    return false;

  boolValue = boolprop->GetValue();
  return true;
}

bool mitk::DataNode::GetIntProperty(const char* propertyKey, int &intValue, const mitk::BaseRenderer* renderer) const
{
  mitk::IntProperty::Pointer intprop = dynamic_cast<mitk::IntProperty*>(GetProperty(propertyKey, renderer));
  if(intprop.IsNull())
    return false;

  intValue = intprop->GetValue();
  return true;
}

bool mitk::DataNode::GetFloatProperty(const char* propertyKey, float &floatValue, const mitk::BaseRenderer* renderer) const
{
  mitk::FloatProperty::Pointer floatprop = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey, renderer));
  if(floatprop.IsNull())
    return false;

  floatValue = floatprop->GetValue();
  return true;
}

bool mitk::DataNode::GetDoubleProperty(const char* propertyKey, double &doubleValue, const mitk::BaseRenderer* renderer) const
{
  mitk::DoubleProperty::Pointer doubleprop = dynamic_cast<mitk::DoubleProperty*>(GetProperty(propertyKey, renderer));
  if(doubleprop.IsNull())
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

bool mitk::DataNode::GetStringProperty(const char* propertyKey, std::string& string, const mitk::BaseRenderer* renderer) const
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

bool mitk::DataNode::GetColor(float rgb[3], const mitk::BaseRenderer* renderer, const char* propertyKey) const
{
  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetProperty(propertyKey, renderer));
  if(colorprop.IsNull())
    return false;

  memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
  return true;
}

bool mitk::DataNode::GetOpacity(float &opacity, const mitk::BaseRenderer* renderer, const char* propertyKey) const
{
  mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey, renderer));
  if(opacityprop.IsNull())
    return false;

  opacity=opacityprop->GetValue();
  return true;
}

bool mitk::DataNode::GetLevelWindow(mitk::LevelWindow &levelWindow, const mitk::BaseRenderer* renderer, const char* propertyKey) const
{
  mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(GetProperty(propertyKey, renderer));
  if(levWinProp.IsNull())
    return false;

  levelWindow=levWinProp->GetLevelWindow();
  return true;
}

void mitk::DataNode::SetColor(const mitk::Color &color, const mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(color);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetColor(float red, float green, float blue, const mitk::BaseRenderer* renderer, const char* propertyKey)
{
  float color[3];
  color[0]=red;
  color[1]=green;
  color[2]=blue;
  SetColor(color, renderer, propertyKey);
}

void mitk::DataNode::SetColor(const float rgb[3], const mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(rgb);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetVisibility(bool visible, const mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::BoolProperty::Pointer prop;
  prop = mitk::BoolProperty::New(visible);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetOpacity(float opacity, const mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::FloatProperty::Pointer prop;
  prop = mitk::FloatProperty::New(opacity);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetLevelWindow(mitk::LevelWindow levelWindow, const mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::LevelWindowProperty::Pointer prop;
  prop = mitk::LevelWindowProperty::New(levelWindow);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataNode::SetIntProperty(const char* propertyKey, int intValue, const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::IntProperty::New(intValue));
}
void mitk::DataNode::SetBoolProperty( const char* propertyKey, bool boolValue, const mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
}

void mitk::DataNode::SetFloatProperty( const char* propertyKey, float floatValue, const mitk::BaseRenderer* renderer/*=NULL*/ )
{
  if (dynamic_cast<DoubleProperty*>(this->GetProperty(propertyKey, renderer)) != NULL)
  {
    MITK_WARN << "Setting float property " << propertyKey << " although a double property with the same name already exists";
  }
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
}

void mitk::DataNode::SetDoubleProperty(const char *propertyKey, float doubleValue, const mitk::BaseRenderer *renderer)
{
  if (dynamic_cast<FloatProperty*>(this->GetProperty(propertyKey, renderer)) != NULL)
  {
    MITK_WARN << "Setting double property " << propertyKey << " although a float property with the same name already exists";
  }
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::DoubleProperty::New(doubleValue));
}

void mitk::DataNode::SetStringProperty( const char* propertyKey, const char* stringValue, const mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
}

void mitk::DataNode::SetProperty(const char *propertyKey,
                                     BaseProperty* propertyValue,
                                     const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, propertyValue);
}

void mitk::DataNode::ReplaceProperty(const char *propertyKey,
                                         BaseProperty* propertyValue,
                                         const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->ReplaceProperty(propertyKey, propertyValue);
}

void mitk::DataNode::AddProperty(const char *propertyKey,
                                     BaseProperty* propertyValue,
                                     const mitk::BaseRenderer* renderer,
                                     bool overwrite)
{
  if((overwrite) || (GetProperty(propertyKey, renderer) == NULL))
  {
    SetProperty(propertyKey, propertyValue, renderer);
  }
}


vtkLinearTransform* mitk::DataNode::GetVtkTransform(int t) const
{
  assert(m_Data.IsNotNull());

  mitk::BaseGeometry* geometry = m_Data->GetGeometry(t);

  if(geometry == NULL)
    return NULL;

  return geometry->GetVtkTransform();
}

unsigned long mitk::DataNode::GetMTime() const
{
  unsigned long time = Superclass::GetMTime();
  if(m_Data.IsNotNull())
  {
    if((time < m_Data->GetMTime()) ||
       ((m_Data->GetSource().IsNotNull()) && (time < m_Data->GetSource()->GetMTime()))
    )
    {
      Modified();
      return Superclass::GetMTime();
    }
  }
  return time;
}

void mitk::DataNode::SetSelected(bool selected, const mitk::BaseRenderer* renderer)
{
  mitk::BoolProperty::Pointer selectedProperty = dynamic_cast<mitk::BoolProperty*>(GetProperty("selected"));

  if ( selectedProperty.IsNull() )
  {
    selectedProperty = mitk::BoolProperty::New();
    selectedProperty->SetValue(false);
    SetProperty("selected", selectedProperty, renderer);
  }

  if( selectedProperty->GetValue() != selected )
  {
    selectedProperty->SetValue(selected);
    itk::ModifiedEvent event;
    InvokeEvent( event );
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

bool mitk::DataNode::IsSelected(const mitk::BaseRenderer* renderer)
{
  bool selected;

  if ( !GetBoolProperty("selected", selected, renderer) )
    return false;

  return selected;
}

void mitk::DataNode::SetInteractorEnabled( const bool& enabled )
{
  if ( m_Interactor.IsNull() )
  {
    itkWarningMacro("Interactor is NULL. Couldn't enable or disable interaction.");
    return;
  }
  if ( enabled )
    mitk::GlobalInteraction::GetInstance()->AddInteractor( m_Interactor.GetPointer() );
  else
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor.GetPointer() );
}

void mitk::DataNode::EnableInteractor()
{
  SetInteractorEnabled( true );
}

void mitk::DataNode::DisableInteractor()
{
  SetInteractorEnabled( false );
}

bool mitk::DataNode::IsInteractorEnabled() const
{
  return mitk::GlobalInteraction::GetInstance()->InteractorRegistered( m_Interactor.GetPointer() );
}

void mitk::DataNode::SetDataInteractor(const DataInteractor::Pointer& interactor)
{
  m_DataInteractor = interactor;
  Modified();

  // the interactor has changed, so we have ti invoke an InteractorChangedEvent
  const mitk::DataNode::InteractorChangedEvent changedEvent;
  InvokeEvent( changedEvent );
}

mitk::DataInteractor::Pointer mitk::DataNode::GetDataInteractor() const
{
  return m_DataInteractor;
}

void mitk::DataNode::PropertyListModified( const itk::Object* /*caller*/, const itk::EventObject& )
{
  Modified();
}

