/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-01 08:56:22 +0200 (Di, 01 Apr 2008) $
Version:   $Revision: 13931 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkParentChild.h"

//CHILI
#include <chili/plugin.h>
//MITK
#include "mitkDataStorage.h"
#include "mitkImageToPicDescriptor.h"
//XML
#include <tinyxml.h>

#ifdef CHILI_PLUGIN_VERSION_CODE

mitk::ParentChild::ParentChild()
{
  m_currentXML.xmlStudyOID = "";
  m_currentXML.xmlStudyInstanceUID = "";
  m_currentXML.xmlPatientOID = "";
  m_currentXML.xmlSeriesOID = "";
  m_currentXML.xmlTextOID = "";
  m_currentXML.xmlDocument = NULL;
  m_currentXML.elementCount = 0;
}

mitk::ParentChild::~ParentChild()
{
}

void mitk::ParentChild::InitParentChild( QcPlugin* instance, const std::string& studyOID, const std::string& instanceUID, const std::string& patientOID, const std::string& tmpDirectory )
{
  if( studyOID == "" )
  {
    std::cout << "ParentChild (InitParentChild): Empty StudyOID set. Its not possible to load/save volumes or relations." << std::endl;
    m_currentXML.xmlStudyOID = "";
    m_currentXML.xmlStudyInstanceUID = "";
    m_currentXML.xmlPatientOID = "";
    m_currentXML.xmlSeriesOID = "";
    m_currentXML.xmlTextOID = "";
    m_currentXML.xmlDocument = NULL;
    m_currentXML.elementCount = 0;
    m_SavedNodeList.clear();
  }

  if( m_currentXML.xmlStudyOID != studyOID )
  {
    m_Instance = instance;
    m_tmpDirectory = tmpDirectory;
    m_currentXML.xmlStudyOID = studyOID;
    m_currentXML.xmlStudyInstanceUID = instanceUID;
    m_currentXML.xmlPatientOID = patientOID;
    m_currentXML.xmlSeriesOID = "";
    m_currentXML.xmlTextOID = "";
    m_currentXML.xmlDocument = NULL;
    m_currentXML.elementCount = 0;
    m_SavedNodeList.clear();
    //search for xml-file
    pIterateSeries( instance, (char*)studyOID.c_str(), NULL, &ParentChild::GlobalIterateSearchStepOne, this );

    if( m_currentXML.xmlSeriesOID != "" )
    {
      if( m_currentXML.xmlTextOID == "" )  //no file found
      {
        //CHILI-Version >= 3.12 ???
        #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE

        //create new file on harddisk
        m_currentXML.xmlDocument = new TiXmlDocument();
        TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
        TiXmlElement * volumes = new TiXmlElement( "volumes" );
        TiXmlElement * relations = new TiXmlElement( "relations" );
        m_currentXML.xmlDocument->LinkEndChild( decl );
        m_currentXML.xmlDocument->LinkEndChild( volumes );
        m_currentXML.xmlDocument->LinkEndChild( relations );
        m_currentXML.xmlTextOID = pGetNewOID();  //create new text-oid
        std::string pathAndFile = m_tmpDirectory + "ParentChild.xml";
        m_currentXML.xmlDocument->SaveFile( pathAndFile.c_str() );

        #endif
      }
      else  //file exist
      {
        //load xml-file from harddisk
        std::string pathAndFile = tmpDirectory + "ParentChild.xml";
        m_currentXML.xmlDocument = new TiXmlDocument( pathAndFile.c_str() );
        if( !m_currentXML.xmlDocument->LoadFile() )
        {
          std::cout << "ParentChild (InitParentChild): File for parent-child-relationship exist, but not able to load. Abort." << std::endl;
          m_currentXML.xmlSeriesOID = "";
          m_currentXML.xmlTextOID = "";
          m_currentXML.xmlDocument = NULL;
        }
        else
        {
          TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
           if( volume )
             for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
               m_currentXML.elementCount++;
        }
      }
    }
  }
}

ipBool_t mitk::ParentChild::GlobalIterateSearchStepOne( int /*rows*/, int /*row*/, series_t* series, void* user_data )
{
  ParentChild* callingObject = static_cast<ParentChild*>(user_data);
  if( series != NULL )
  {
    //if we find no xml-file, we have to create a new one and save it to the first found series
    if( callingObject->m_currentXML.xmlSeriesOID == "" )
      callingObject->m_currentXML.xmlSeriesOID = series->oid;

    //search for file
    pIterateTexts( callingObject->m_Instance, series->oid, NULL, &ParentChild::GlobalIterateSearchStepTwo, callingObject );
    if( callingObject->m_currentXML.xmlTextOID != "" )
    {
      callingObject->m_currentXML.xmlSeriesOID = series->oid; //file found, save series-oid
      return ipFalse; //stop iterate
    }
  }
  return ipTrue;
}

ipBool_t mitk::ParentChild::GlobalIterateSearchStepTwo( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  ParentChild* callingObject = static_cast<ParentChild*>(user_data);
  if( text != NULL )
  {
    std::string chiliDataBaseName = text->chiliText;
    std::string fileName = chiliDataBaseName.substr( chiliDataBaseName.find_last_of("-") + 1 );
    if( fileName == "ParentChild.xml" )
    {
      //save xml-file to harddisk
      std::string pathAndFile = callingObject->m_tmpDirectory + fileName;
      ipInt4_t error;
      pFetchDataToFile( chiliDataBaseName.c_str(), pathAndFile.c_str(), &error );
      if( error != 0 )
        std::cout << "ParentChild (GlobalIterateSearchStepTwo): ChiliError: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
      else
        callingObject->m_currentXML.xmlTextOID = text->oid;
      return ipFalse;  //stop iterate
    }
  }
  return ipTrue;
}

void mitk::ParentChild::AddEntry( DataTreeNode::Pointer node, std::list< std::string > CurrentImageInstanceUIDs, const std::string& seriesOID )
{
  if( m_currentXML.xmlDocument && node )
  {
    TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
    if( volume )
    {
      BaseData* data = node->GetData();
      if( data )
      {
        //create a new volume
        BaseProperty::Pointer name = node->GetProperty( "name" );
        std::string volumeDescription;
        if( name )
          volumeDescription = name->GetValueAsString();
        else
          volumeDescription = "no Description";
        std::ostringstream stringHelper;
        stringHelper << "volume" << m_currentXML.elementCount + 1;
        std::string volumeLabel = stringHelper.str();
        TiXmlElement * element = new TiXmlElement( "volume" );
        element->SetAttribute( "label", volumeLabel.c_str() );
        node->SetProperty( "VolumeLabel", new StringProperty( volumeLabel ) );
        element->SetAttribute( "id", volumeDescription.c_str() );
        element->SetAttribute( "seriesOID", seriesOID.c_str() );

        std::string UIDDescription;
        Image* image = dynamic_cast<Image*>( data );
        if( image )
          UIDDescription = "ImageInstanceUID";
        else
          UIDDescription = "TextOID";

        //add UIDs
        while( !CurrentImageInstanceUIDs.empty() )
        {
          TiXmlElement * uid;
          uid = new TiXmlElement( UIDDescription.c_str() );
          uid->LinkEndChild( new TiXmlText( CurrentImageInstanceUIDs.front().c_str() ) );
          element->LinkEndChild( uid );
          CurrentImageInstanceUIDs.pop_front();
        }

        //save new volume
        volume->LinkEndChild( element );
        std::string pathAndFile = m_tmpDirectory + "ParentChild.xml";
        m_currentXML.xmlDocument->SaveFile( pathAndFile.c_str() );
        m_currentXML.elementCount++;
        //save the used node for save relationships
        m_SavedNodeList.push_back( node );
      }
    }
  }
}

void mitk::ParentChild::SaveRelationShip()
{
  //CHILI-Version >= 3.12 ???
  #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE

  if( m_currentXML.xmlDocument )
  {
    InitCircleCheckStructure();
    while( !m_SavedNodeList.empty() )
    {
      //child-node
      DataTreeNode::Pointer currentNode = m_SavedNodeList.front();
      BaseProperty::Pointer childVolumeLabel = currentNode->GetProperty( "VolumeLabel" );
      //get relation
      DataStorage::SetOfObjects::ConstPointer currentRelations = DataStorage::GetInstance()->GetSources( currentNode );
      for( DataStorage::SetOfObjects::const_iterator relationIter = currentRelations->begin(); relationIter != currentRelations->end(); relationIter++ )
      {
        if( (*relationIter) )
        {
          //check Parent-Node
          BaseProperty::Pointer parentSeriesOID = (*relationIter)->GetProperty( "SeriesOID" );
          BaseProperty::Pointer parentVolumeLabel = (*relationIter)->GetProperty( "VolumeLabel" );

          if( !parentSeriesOID ) continue;  //the parent dont save at CHILI, so we dont need to save the relation
          else
          {
            if( !parentVolumeLabel )  //the node save to CHILI but not known as parent-child-volume
            {
              std::list< std::string > currentUIDs;
              currentUIDs.clear();
              BaseProperty::Pointer parentTextOID = (*relationIter)->GetProperty( "TextOID" );
              if( parentTextOID )  //text
                currentUIDs.push_back( parentTextOID->GetValueAsString() );
              else  //image
              {
                BaseData* data = (*relationIter)->GetData();
                if ( data )
                {
                  Image* image = dynamic_cast<Image*>( data );
                  if( image )
                  {
                    //use the class ImageToPicDescriptor to get the single ImageInstanceUIDS
                    ImageToPicDescriptor::Pointer converterToDescriptor = ImageToPicDescriptor::New();
                    converterToDescriptor->SetImage( image );
                    ImageToPicDescriptor::TagInformationList empty;
                    empty.clear();
                    converterToDescriptor->SetTagList( empty, true );
                    converterToDescriptor->Update();
                    currentUIDs = converterToDescriptor->GetSaveImageInstanceUIDs();
                  }
                }
              }
              AddEntry( (*relationIter), currentUIDs, parentSeriesOID->GetValueAsString() );  //add parent-volume to xml-file
              parentVolumeLabel = (*relationIter)->GetProperty( "VolumeLabel" );  //get the volume-label
            }

            if( !SaveSingleRelation( childVolumeLabel->GetValueAsString(), parentVolumeLabel->GetValueAsString() ) )
              std::cout << "ChiliPlugin (SaveRelationShip): Relation " << parentVolumeLabel->GetValueAsString() << " - " << childVolumeLabel->GetValueAsString() << " create a circle. Relation not added." << std::endl;
          }
        }
      }
      m_SavedNodeList.pop_front();
    }
    //save Text-file
    std::string pathAndFile = m_tmpDirectory + "ParentChild.xml";
    if( !pStoreDataFromFile( pathAndFile.c_str(), "ParentChild.xml", "application/MITK.xml", NULL, m_currentXML.xmlStudyInstanceUID.c_str(), m_currentXML.xmlPatientOID.c_str(), m_currentXML.xmlStudyOID.c_str(), m_currentXML.xmlSeriesOID.c_str(), m_currentXML.xmlTextOID.c_str() ) )
      std::cout << "ParentChild (SaveRelationShip): Error while saving parent-child-relationship." << std::endl;
   else
      if( remove(  pathAndFile.c_str() ) != 0 )
        std::cout << "ParentChild (SaveRelationShip): Not able to  delete file: "<< pathAndFile << std::endl;
  }
  #endif
}

void mitk::ParentChild::InitCircleCheckStructure()
{
  //for further information look at http://en.wikipedia.org/wiki/Topological_sorting

  TiXmlElement* relation = m_currentXML.xmlDocument->FirstChildElement("relations");
  TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
  if( !volume || !relation ) return;

  //use all volumes to create the list
  m_CircleCheckStructure.clear();
  for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
  {
    CircleCheckStruct newElement;
    newElement.VolumeLabel = "";

    for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
    {
      std::string reference = search->Name();
      if( reference == "label" )
        newElement.VolumeLabel = search->Value();
    }

    newElement.ParentCount = 0;
    newElement.ChildList.clear();
    m_CircleCheckStructure.push_back( newElement );
  }

  //now set the count and parentList
  for( TiXmlElement* singleRelation = relation->FirstChildElement(); singleRelation; singleRelation = singleRelation->NextSiblingElement() )
  {
    std::string volumeLabel = singleRelation->GetText();
    std::string parent = volumeLabel.substr( 0, volumeLabel.find(" ") );
    std::string child = volumeLabel.substr( volumeLabel.find(" ") + 1 );

    for( std::list<CircleCheckStruct>::iterator iter = m_CircleCheckStructure.begin(); iter != m_CircleCheckStructure.end(); iter++ )
    {
      if( iter->VolumeLabel == parent )
        iter->ChildList.push_back( child );
      if( iter->VolumeLabel == child )
        iter->ParentCount++;
    }
  }
}

bool mitk::ParentChild::SaveSingleRelation( const std::string& childVolumeLabel, const std::string& parentVolumeLabel )
{
  if( m_currentXML.xmlDocument )
  {
    TiXmlElement* relation = m_currentXML.xmlDocument->FirstChildElement("relations");
    if( !relation ) return false;

    //delete possible existing relations
    std::string possibleCombinationOne =childVolumeLabel + " " + parentVolumeLabel;
    std::string possibleCombinationTwo =parentVolumeLabel + " " + childVolumeLabel;
    for( TiXmlElement* singleRelation = relation->FirstChildElement(); singleRelation; singleRelation = singleRelation->NextSiblingElement() )
    {
      if( singleRelation->GetText() == possibleCombinationOne || singleRelation->GetText() == possibleCombinationTwo )
        relation->RemoveChild( singleRelation );
    }

    //CIRCLE-CHECK
    std::list<CircleCheckStruct> workingCopy = m_CircleCheckStructure;
    //add relation to workingCopy
    for( std::list<CircleCheckStruct>::iterator iter = workingCopy.begin(); iter != workingCopy.end(); iter++ )
    {
      if( iter->VolumeLabel == parentVolumeLabel )
        iter->ChildList.push_back( childVolumeLabel );
      if( iter->VolumeLabel == childVolumeLabel )
        iter->ParentCount++;
    }

    //circle-test
    bool search = true;
    bool circleFound = false;
    while( search )
    {
      //search for element with no parent
      std::list<CircleCheckStruct>::iterator parentLessElement;
      for( parentLessElement = workingCopy.begin(); parentLessElement != workingCopy.end(); parentLessElement++ )
        if( parentLessElement->ParentCount == 0 ) break;

      if( parentLessElement != workingCopy.end() )
      {
        parentLessElement->ParentCount = -1;
        //decrease parent count of all children
        for( std::list<std::string>::iterator childListIter = parentLessElement->ChildList.begin(); childListIter != parentLessElement->ChildList.end(); childListIter++ )
          for( std::list<CircleCheckStruct>::iterator circleListIter = workingCopy.begin(); circleListIter != workingCopy.end(); circleListIter++ )
            if( circleListIter->VolumeLabel == (*childListIter) )
            {
              circleListIter->ParentCount--;
              break;
            }
      }
      else
      {
        search = false;
        //check if the ParentCount of all elements are -1
        for( std::list<CircleCheckStruct>::iterator circleListIter = workingCopy.begin(); circleListIter != workingCopy.end(); circleListIter++ )
        {
          if( circleListIter->ParentCount != -1 )
          {
            circleFound = true;
            break;
          }
        }
      }
    }
    if( !circleFound )
    {
      std::string relationship = parentVolumeLabel + " " + childVolumeLabel;
      TiXmlElement * relationElement = new TiXmlElement( "Relation" );
      relationElement->LinkEndChild( new TiXmlText( relationship.c_str() ) );
      relation->LinkEndChild( relationElement );
      std::string pathAndFile = m_tmpDirectory + "ParentChild.xml";
      m_currentXML.xmlDocument->SaveFile( pathAndFile.c_str() );
    }
    return !circleFound;
  }
  return false;
}

std::string mitk::ParentChild::GetLabel( std::list< std::string > ImageInstanceUIDs )
{
  if( m_currentXML.xmlDocument )
  {
    TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
    if( !volume )  return "";

    for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
    {
      unsigned int idCount = 0;  //we have to count the ids, they can be different sized
      bool match = true;
      TiXmlElement* singleID = singleVolume->FirstChildElement();
      while( singleID && match )
      {
        if( singleID->GetText() && find( ImageInstanceUIDs.begin(), ImageInstanceUIDs.end(), singleID->GetText() ) != ImageInstanceUIDs.end() )
          idCount++;
        else
          match = false; //one slice dont match, thats not the searched volume, take the next
        singleID = singleID->NextSiblingElement();
      }
      if( ( ImageInstanceUIDs.size() == idCount ) && match )
      {
        for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
        {
          std::string reference = search->Name();
          if( reference == "label" )
            return search->Value();
        }
      }
    }
  }
  return "";
}

std::list<std::string> mitk::ParentChild::GetSlices( const std::string& label, const std::string& seriesOID )
{
  std::list<std::string> result;
  result.clear();

  if( m_currentXML.xmlDocument )
  {
    TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
    if( volume )
    {
      //search for the right element
      for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
      {
        if( singleVolume->FirstAttribute() )
        {
          //search for label and oid
          std::string labelAtt = "";
          std::string oidAtt = "";
          for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
          {
            std::string reference = search->Name();
            if( reference == "label" )
              labelAtt = search->Value();
            else
              if( reference == "seriesOID" )
                oidAtt = search->Value();
          }

          //check if the seriesOID and label are equal
          if( oidAtt == seriesOID && labelAtt == label )
          {
            //need to know if to load a text or image
            TiXmlElement* singleID = singleVolume->FirstChildElement();
            std::string identification = singleID->Value();
            if( identification == "TextOID" )
              result.push_back( "Text" );
            else
              if( identification == "ImageInstanceUID" )
                result.push_back( "Image" );

            for( TiXmlElement* singleID = singleVolume->FirstChildElement(); singleID; singleID = singleID->NextSiblingElement() )
              if( singleID->GetText() )
                result.push_back( singleID->GetText() );
          }
        }
      }
    }
  }
  return result;
}

mitk::PACSPlugin::PSRelationInformationList mitk::ParentChild::GetSeriesRelationInformation( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory )
{
  PACSPlugin::PSRelationInformationList result;
  result.clear();

  //init the right parent-child-xml-file
  PACSPlugin::StudyInformation currentStudy = PACSPlugin::GetInstance()->GetStudyInformation( seriesOID );
  PACSPlugin::PatientInformation currentPatient = PACSPlugin::GetInstance()->GetPatientInformation( seriesOID );
  InitParentChild( instance, currentStudy.OID, currentStudy.InstanceUID, currentPatient.OID, tmpDirectory );

  if( m_currentXML.xmlDocument )
  {
    TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
    if( !volume ) return result;

    //check every volume
    for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
    {
      if( singleVolume->FirstAttribute() )
      {
        //search for label, id and oid
        std::string label = "";
        std::string id = "";
        std::string oid = "";
        for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
        {
          std::string reference = search->Name();
          if( reference == "label" )
            label = search->Value();
          else
            if( reference == "id" )
              id = search->Value();
            else
              if( reference== "seriesOID" )
                oid = search->Value();
        }

        if( oid == seriesOID )  //if the saved oid equal the searched one
        {
          //save element
          PACSPlugin::PSRelationInformation newElement;
          newElement.Label = label;
          newElement.ID = id;
          newElement.OID = oid;
          newElement.ParentLabel.clear();
          newElement.ChildLabel.clear();

          //is it image or text?
          TiXmlElement* singleID = singleVolume->FirstChildElement();
          std::string identification = singleID->Value();
          if( identification == "TextOID" )
            newElement.Image = false;
          else newElement.Image = true;

          result.push_back( newElement );
        }
      }
    }
  }
  return result;
}

mitk::PACSPlugin::PSRelationInformationList mitk::ParentChild::GetStudyRelationInformation( QcPlugin* instance, const std::string& studyOID, const std::string& tmpDirectory )
{
  PACSPlugin::PSRelationInformationList result;
  result.clear();

  if( studyOID == "" )
  {
    study_t* currentStudy= pCurrentStudy();
    patient_t* currentPatient= pCurrentPatient();
    if( currentStudy && currentPatient )
      InitParentChild( instance, currentStudy->oid, currentStudy->instanceUID, currentPatient->oid, tmpDirectory );
  }
  else
  {
    study_t study;
    patient_t patient;
    initStudyStruct( &study );
    initPatientStruct( &patient );

    if( pQueryStudy( instance, &study, &patient ) )
      InitParentChild( instance, study.oid, study.instanceUID, patient.oid, tmpDirectory );

    clearStudyStruct( &study );
    clearPatientStruct( &patient );
  }

  if( m_currentXML.xmlDocument )
  {
    TiXmlElement* volume = m_currentXML.xmlDocument->FirstChildElement("volumes");
    TiXmlElement* relation = m_currentXML.xmlDocument->FirstChildElement("relations");
    if( !volume || !relation ) return result;

    //check every volume
    for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
    {
      if( singleVolume->FirstAttribute() )
      {
        //search for label, id and oid
        std::string label = "";
        std::string id = "";
        std::string oid = "";
        for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
        {
          std::string reference = search->Name();
          if( reference == "label" )
            label = search->Value();
          else
            if( reference == "id" )
              id = search->Value();
            else
              if( reference == "seriesOID" )
                oid = search->Value();
        }

        //save element
        PACSPlugin::PSRelationInformation newElement;
        newElement.Label = label;
        newElement.ID = id;
        newElement.OID = oid;
        newElement.ParentLabel.clear();
        newElement.ChildLabel.clear();

        //is it image or text?
        TiXmlElement* singleID = singleVolume->FirstChildElement();
        std::string identification = singleID->Value();
        if( identification == "TextOID" )
          newElement.Image = false;
        else newElement.Image = true;

        //get relation
        for( TiXmlElement* singleRelation = relation->FirstChildElement(); singleRelation; singleRelation = singleRelation->NextSiblingElement() )
        {
          std::string volumeLabel = singleRelation->GetText();
          std::string parent = volumeLabel.substr( 0, volumeLabel.find(" ") );
          std::string child = volumeLabel.substr( volumeLabel.find(" ") + 1 );
          if( child == newElement.Label )
            newElement.ParentLabel.push_back( parent );
          if( parent == newElement.Label )
            newElement.ChildLabel.push_back( child );
        }

        result.push_back( newElement );
      }
    }
  }
  return result;
}

#endif
