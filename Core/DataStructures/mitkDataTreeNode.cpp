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


#include "mitkDataTreeNode.h"
#include "mitkCoreObjectFactory.h"
#include <vtkTransform.h>
#include <itkSmartPointerForwardReference.txx>

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkSmartPointerProperty.h"
#include "mitkMaterialProperty.h"
#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkGeometry3D.h"
#include "mitkRenderingManager.h"
#include "mitkXMLWriter.h"
#include "mitkXMLReader.h"
#include "mitkGlobalInteraction.h"
#include "mitkEventMapper.h"
#include "mitkGenericProperty.h"

#include "mitkCoreObjectFactory.h"

const std::string mitk::DataTreeNode::XML_NODE_NAME = "dataTreeNode";

mitk::Mapper* mitk::DataTreeNode::GetMapper(MapperSlotId id) const
{
  if( (id >= m_Mappers.size()) || (m_Mappers[id].IsNull()) ) 
  {
    if(id >= m_Mappers.capacity())
    {
//      int i, size=id-m_Mappers.capacity()+10;
      m_Mappers.resize(id+10);
    }
    m_Mappers[id] = CoreObjectFactory::GetInstance()->CreateMapper(const_cast<DataTreeNode*>(this),id);
  }
  return m_Mappers[id];
}

mitk::BaseData* mitk::DataTreeNode::GetData() const
{
  return m_Data;
}

mitk::Interactor* mitk::DataTreeNode::GetInteractor() const
{
  return m_Interactor;
}

void mitk::DataTreeNode::SetData(mitk::BaseData* baseData)
{
  if(m_Data!=baseData)
  {
    m_Data=baseData;

    m_Mappers.clear();
    m_Mappers.resize(10);

    mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties(this);

    m_DataReferenceChangedTime.Modified();
    Modified();
  }
}

void mitk::DataTreeNode::SetInteractor(mitk::Interactor* interactor)
{
  m_Interactor = interactor;
  if(m_Interactor.IsNotNull())
  m_Interactor->SetDataTreeNode(this);
}

mitk::DataTreeNode::DataTreeNode() : m_Data(NULL)
{
  m_Mappers.resize(10);

  m_PropertyList = PropertyList::New();
}


mitk::DataTreeNode::~DataTreeNode()
{
  Interactor* interactor = this->GetInteractor();

  if ( ( interactor ) && ( mitk::GlobalInteraction::HasInstance() ) )
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( interactor );  
  }
  m_Mappers.clear();
  m_Data = NULL;
}

mitk::DataTreeNode& mitk::DataTreeNode::operator=(const DataTreeNode& right)
{
  mitk::DataTreeNode* node=mitk::DataTreeNode::New();
  node->SetData(right.GetData());
  return *node;
}

mitk::DataTreeNode& mitk::DataTreeNode::operator=(mitk::BaseData* right)
{
  mitk::DataTreeNode* node=mitk::DataTreeNode::New();
  node->SetData(right);
  return *node;
}

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::istream& mitk::operator>>( MBI_STD::istream& i, mitk::DataTreeNode::Pointer& dtn )
#endif
#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MBI_STD::istream& operator>>( MBI_STD::istream& i, mitk::DataTreeNode::Pointer& dtn ) 
#endif
{
  dtn = mitk::DataTreeNode::New();
  //i >> av.get();
  return i;
}

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::ostream& mitk::operator<<( MBI_STD::ostream& o, mitk::DataTreeNode::Pointer& dtn)
#endif
#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MBI_STD::ostream& operator<<( MBI_STD::ostream& o, mitk::DataTreeNode::Pointer& dtn)
#endif
{
  if(dtn->GetData()!=NULL)
    o<<dtn->GetData()->GetNameOfClass();
  else
    o<<"empty data";
  return o;
}

void mitk::DataTreeNode::SetMapper(MapperSlotId id, mitk::Mapper* mapper)
{
  m_Mappers[id] = mapper;

  if (mapper!=NULL)
    mapper->SetDataTreeNode(this);
}

void mitk::DataTreeNode::UpdateOutputInformation()
{
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }
}

void mitk::DataTreeNode::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::DataTreeNode::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::DataTreeNode::VerifyRequestedRegion()
{
    return true;
}

void mitk::DataTreeNode::SetRequestedRegion(itk::DataObject * /*data*/)
{
}

void mitk::DataTreeNode::CopyInformation(const itk::DataObject * /*data*/)
{
}
mitk::PropertyList* mitk::DataTreeNode::GetPropertyList(const mitk::BaseRenderer* renderer) const
{
  if(renderer==NULL)
    return m_PropertyList;

  mitk::PropertyList::Pointer & propertyList = m_MapOfPropertyLists[renderer];

  if(propertyList.IsNull())
    propertyList = mitk::PropertyList::New();

  assert(m_MapOfPropertyLists[renderer].IsNotNull());

  return propertyList;
}

void mitk::DataTreeNode::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  m_PropertyList->ConcatenatePropertyList(pList, replace);
}

mitk::BaseProperty* mitk::DataTreeNode::GetProperty(const char *propertyKey, const mitk::BaseRenderer* renderer) const
{
  if(propertyKey==NULL)
    return NULL;

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
  
mitk::DataTreeNode::GroupTagList mitk::DataTreeNode::GetGroupTags() const
{
  GroupTagList groups;
  const PropertyList::PropertyMap* propertyMap = m_PropertyList->GetMap();

  for ( PropertyList::PropertyMap::const_iterator groupIter = propertyMap->begin(); // m_PropertyList is created in the constructor, so we don't check it here
        groupIter != propertyMap->end();
        ++groupIter )
  {
    const BaseProperty* bp = groupIter->second.first;
    if ( dynamic_cast<const GroupTagProperty*>(bp) && groupIter->second.second )
    {
      groups.insert( groupIter->first );
    }
  }

  return groups;
}

bool mitk::DataTreeNode::GetBoolProperty(const char* propertyKey, bool& boolValue, mitk::BaseRenderer* renderer) const
{
  mitk::BoolProperty::Pointer boolprop = dynamic_cast<mitk::BoolProperty*>(GetProperty(propertyKey, renderer));
  if(boolprop.IsNull())
    return false;

  boolValue = boolprop->GetValue();
  return true;
}

bool mitk::DataTreeNode::GetIntProperty(const char* propertyKey, int &intValue, mitk::BaseRenderer* renderer) const
{
  mitk::IntProperty::Pointer intprop = dynamic_cast<mitk::IntProperty*>(GetProperty(propertyKey, renderer));
  if(intprop.IsNull())
    return false;

  intValue = intprop->GetValue();
  return true;
}

bool mitk::DataTreeNode::GetFloatProperty(const char* propertyKey, float &floatValue, mitk::BaseRenderer* renderer) const
{
  mitk::FloatProperty::Pointer floatprop = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey, renderer));
  if(floatprop.IsNull())
    return false;

  floatValue = floatprop->GetValue();
  return true;
}

bool mitk::DataTreeNode::GetStringProperty(const char* propertyKey, std::string& string, mitk::BaseRenderer* renderer) const
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

bool mitk::DataTreeNode::GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* propertyKey) const
{
  mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetProperty(propertyKey, renderer));
  if(colorprop.IsNull())
    return false;

  memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
  return true;
}

bool mitk::DataTreeNode::GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* propertyKey) const
{
  mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty*>(GetProperty(propertyKey, renderer));
  if(opacityprop.IsNull())
    return false;

  opacity=opacityprop->GetValue();
  return true;
}

bool mitk::DataTreeNode::GetLevelWindow(mitk::LevelWindow &levelWindow, mitk::BaseRenderer* renderer, const char* propertyKey) const
{
  mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(GetProperty(propertyKey, renderer));
  if(levWinProp.IsNull())
    return false;

  levelWindow=levWinProp->GetLevelWindow();
  return true;
}

void mitk::DataTreeNode::SetColor(const mitk::Color &color, mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(color);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataTreeNode::SetColor(float red, float green, float blue, mitk::BaseRenderer* renderer, const char* propertyKey)
{
  float color[3];
  color[0]=red;
  color[1]=green;
  color[2]=blue;
  SetColor(color, renderer, propertyKey);
}

void mitk::DataTreeNode::SetColor(const float rgb[3], mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::ColorProperty::Pointer prop;
  prop = mitk::ColorProperty::New(rgb);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataTreeNode::SetVisibility(bool visible, mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::BoolProperty::Pointer prop;
  prop = mitk::BoolProperty::New(visible);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataTreeNode::SetOpacity(float opacity, mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::FloatProperty::Pointer prop;
  prop = mitk::FloatProperty::New(opacity);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataTreeNode::SetLevelWindow(mitk::LevelWindow levelWindow, mitk::BaseRenderer* renderer, const char* propertyKey)
{
  mitk::LevelWindowProperty::Pointer prop;
  prop = mitk::LevelWindowProperty::New(levelWindow);
  GetPropertyList(renderer)->SetProperty(propertyKey, prop);
}

void mitk::DataTreeNode::SetIntProperty(const char* propertyKey, int intValue, mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::IntProperty::New(intValue));
}
void mitk::DataTreeNode::SetBoolProperty( const char* propertyKey, bool boolValue, mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
}

void mitk::DataTreeNode::SetFloatProperty( const char* propertyKey, float floatValue, mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
}

void mitk::DataTreeNode::SetStringProperty( const char* propertyKey, const char* stringValue, mitk::BaseRenderer* renderer/*=NULL*/ )
{
  GetPropertyList(renderer)->SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
}

void mitk::DataTreeNode::SetProperty(const char *propertyKey, 
                                     BaseProperty* propertyValue, 
                                     const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->SetProperty(propertyKey, propertyValue);
}

void mitk::DataTreeNode::ReplaceProperty(const char *propertyKey, 
                                         BaseProperty* propertyValue, 
                                         const mitk::BaseRenderer* renderer)
{
  GetPropertyList(renderer)->ReplaceProperty(propertyKey, propertyValue);
}

void mitk::DataTreeNode::AddProperty(const char *propertyKey, 
                                     BaseProperty* propertyValue, 
                                     const mitk::BaseRenderer* renderer,
                                     bool overwrite)
{
  if((overwrite) || (GetProperty(propertyKey, renderer) == NULL))
  {
    SetProperty(propertyKey, propertyValue, renderer);
  }
}


vtkLinearTransform* mitk::DataTreeNode::GetVtkTransform(int t) const
{
  assert(m_Data.IsNotNull());

  mitk::Geometry3D* geometry = m_Data->GetGeometry(t);

  if(geometry == NULL)
    return NULL;

  return geometry->GetVtkTransform();
}

unsigned long mitk::DataTreeNode::GetMTime() const
{
  unsigned long time = Superclass::GetMTime();
  if(m_Data.IsNotNull())
  {
    if((time < m_Data->GetMTime()) ||
      ((m_Data->GetSource() != NULL) && (time < m_Data->GetSource()->GetMTime()))
    )
    {
      Modified();
      return Superclass::GetMTime();
    }
  }
  return time;
}

bool mitk::DataTreeNode::WriteXMLData( XMLWriter& xmlWriter ) 
{
  xmlWriter.BeginNode( SmartPointerProperty::XML_SMARTPOINTER_TARGET_NODE );
  if ( SmartPointerProperty::GetReferenceCountFor(this) > 0 )
  {
    xmlWriter.WriteProperty( SmartPointerProperty::XML_SMARTPOINTER_TARGET_KEY, SmartPointerProperty::GetReferenceUIDFor(this) );
  }
  xmlWriter.EndNode();

  // PropertyLists
  MapOfPropertyLists::iterator i = m_MapOfPropertyLists.begin();
  const MapOfPropertyLists::iterator end = m_MapOfPropertyLists.end();

  while ( i != end )
  {
    mitk::PropertyList* propertyList = (*i).second;

    if ( propertyList != NULL && propertyList->GetMap()->size() > 0 )
    {
      xmlWriter.BeginNode("renderer");

      if ( (*i).first != NULL )
        xmlWriter.WriteProperty( "RENDERE_NAME", (*i).first->GetName() );
      else
        xmlWriter.WriteProperty( "rendererName", "" );

      if ( propertyList )
        propertyList->WriteXML( xmlWriter );

      xmlWriter.EndNode(); // Renderer
    }
    i++;
  }

  mitk::PropertyList* propertyList = GetPropertyList();

  if ( propertyList )
    propertyList->WriteXML( xmlWriter );

  // Data
  BaseData* data = GetData();

  if ( data )
    data->WriteXML( xmlWriter );

  // mapperList
  xmlWriter.BeginNode("mapperList");
  int mappercount = m_Mappers.size();

  for ( int i=0; i<mappercount; i++ )
  {
    mitk::Mapper* mapper = GetMapper( i );

    if ( mapper )
    {
      xmlWriter.BeginNode("mapperSlot");
      xmlWriter.WriteProperty( "id", i );
      mapper->WriteXML( xmlWriter );
      xmlWriter.EndNode(); // mapperSlot
    }
  }
  xmlWriter.EndNode(); // mapperList

  // Interactor
  Interactor::Pointer interactor = GetInteractor();

  if ( interactor.IsNotNull() )
    interactor->WriteXML( xmlWriter );
  
  return true;    
}

bool mitk::DataTreeNode::ReadXMLData( XMLReader& xmlReader ) 
{
  if ( xmlReader.Goto( BaseData::XML_NODE_NAME ) ) {
    m_Data = dynamic_cast<mitk::BaseData*>( xmlReader.CreateObject().GetPointer() );
    if ( m_Data.IsNotNull() ) m_Data->ReadXMLData( xmlReader );
    xmlReader.GotoParent();
  }
  if ( xmlReader.Goto( SmartPointerProperty::XML_SMARTPOINTER_TARGET_NODE ) ) {
    std::string uid;
    if ( xmlReader.GetAttribute( SmartPointerProperty::XML_SMARTPOINTER_TARGET_KEY, uid ) )
    {
      SmartPointerProperty::RegisterPointerTarget( this, uid );
    }
    xmlReader.GotoParent();
  }
  if ( xmlReader.Goto( Interactor::XML_NODE_NAME ) ) {
    m_Interactor = dynamic_cast<mitk::Interactor*>( xmlReader.CreateObject().GetPointer() );
    if ( m_Interactor.IsNotNull() ) 
    {
      m_Interactor->ReadXMLData( xmlReader );
      m_Interactor->SetDataTreeNode( this );
      mitk::GlobalInteraction::GetInstance()->AddInteractor( m_Interactor );
    }
    xmlReader.GotoParent();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  if ( xmlReader.Goto( PropertyList::XML_NODE_NAME ) ) {
    m_PropertyList = dynamic_cast<mitk::PropertyList*>( xmlReader.CreateObject().GetPointer() );
    if ( m_PropertyList.IsNotNull() ) m_PropertyList->ReadXMLData( xmlReader );
    xmlReader.GotoParent();
  }

  // additional property settings prevent update problems
  // the advantage to manipulate the property path here is that you get the SourceFilePath from relative - negative is that the XML file contains old information
  mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New( xmlReader.GetSourceFilePath() );
  this->SetProperty( StringProperty::PATH, pathProp );
  // fixes the update problem of colorProperty and materialProperty
  mitk::MaterialProperty::Pointer material = dynamic_cast<mitk::MaterialProperty*>(m_PropertyList->GetProperty("material"));
  if(material)
    material->SetDataTreeNode(this);

  if ( xmlReader.Goto( "mapperList" ) ) {

    if ( xmlReader.Goto( "mapperSlot" ) ) {
    
      do{
        int id = -1;
        xmlReader.GetAttribute( "id", id );

        if ( xmlReader.Goto( "mapper" ) ) {
          Mapper::Pointer mapper = dynamic_cast<mitk::Mapper*>( xmlReader.CreateObject().GetPointer() );
          if ( mapper.IsNotNull() ) {
            mapper->ReadXMLData( xmlReader );
            SetMapper( id, mapper );
          }
          xmlReader.GotoParent();
        }
      
      }while ( xmlReader.GotoNext() );
      xmlReader.GotoParent();
    }
    xmlReader.GotoParent();
  }
  return true;
}

const std::string& mitk::DataTreeNode::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}

void mitk::DataTreeNode::SetSelected(bool selected, mitk::BaseRenderer* renderer)
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

  SelectedEvent(DataTreeNode* dataTreeNode)
    { m_DataTreeNode = dataTreeNode; };
  DataTreeNode* GetDataTreeNode() 
    { return m_DataTreeNode; };
  virtual const char * GetEventName() const 
    { return "SelectedEvent"; } 
  virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { return dynamic_cast<const Self*>(e); } 
  virtual ::itk::EventObject* MakeObject() const 
    { return new Self(m_DataTreeNode); } 
private: 
  DataTreeNode* m_DataTreeNode;
  SelectedEvent(const Self& event)
    { m_DataTreeNode = event.m_DataTreeNode; }; 
  void operator=(const Self& event)
  { m_DataTreeNode = event.m_DataTreeNode; }
};
*/

bool mitk::DataTreeNode::IsSelected(mitk::BaseRenderer* renderer)
{
  bool selected;

  if ( !GetBoolProperty("selected", selected, renderer) )
    return false;

  return selected;
}

void mitk::DataTreeNode::SetInteractorEnabled( const bool& enabled )
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

void mitk::DataTreeNode::EnableInteractor()
{
  SetInteractorEnabled( true );
}

void mitk::DataTreeNode::DisableInteractor()
{
  SetInteractorEnabled( false );
}

bool mitk::DataTreeNode::IsInteractorEnabled() const
{
  return mitk::GlobalInteraction::GetInstance()->InteractorRegistered( m_Interactor.GetPointer() );
}


#ifndef _MSC_VER
template <typename T>
bool mitk::DataTreeNode::GetPropertyValue(const char* propertyKey, T & value, mitk::BaseRenderer* renderer) const
{
  GenericProperty<T>* gp= dynamic_cast<GenericProperty<T>*>(GetProperty(propertyKey, renderer) );
  if ( gp != NULL )
  {
    value = gp->GetValue();
    return true;
  }
  return false;
}

template bool mitk::DataTreeNode::GetPropertyValue<double>(char const*, double&, mitk::BaseRenderer*, bool*) const;
template bool mitk::DataTreeNode::GetPropertyValue<float>(char const*, float&, mitk::BaseRenderer*, bool*) const;
template bool mitk::DataTreeNode::GetPropertyValue<int>(char const*, int&, mitk::BaseRenderer*, bool*) const;
template bool mitk::DataTreeNode::GetPropertyValue<bool>(char const*, bool&, mitk::BaseRenderer*, bool*) const;

#endif

