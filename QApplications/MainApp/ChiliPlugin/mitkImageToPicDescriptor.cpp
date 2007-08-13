/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-06-15 14:28:00 +0200 (Fr, 15 Jun 2007) $
Version:   $Revision: 10778 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkImageToPicDescriptor.h"

//Pic-Tags
#include <ipPic/ipPicTags.h>
//Chili-Inlcudes
#include <chili/isg.h>
#include <chili/plugin.h>
//Mitk
#include "mitkFrameOfReferenceUIDManager.h"
#include "mitkLevelWindowProperty.h"
#include "mitkImageSliceSelector.h"

//#include "mitkPicHeaderProperty.h"

//constructor
mitk::ImageToPicDescriptor::ImageToPicDescriptor()
{
  m_SourceImage = NULL;
  m_StudyPatientAndSeriesTags.clear();
  m_Output.clear();
}

//destructor
mitk::ImageToPicDescriptor::~ImageToPicDescriptor()
{
}

//create a string-pic-tag
ipPicTSV_t* mitk::ImageToPicDescriptor::CreateASCIITag( std::string description, std::string content )
{
  if( description != "" )
  {
    ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
    strcpy( tsv->tag, description.c_str() );
    tsv->type = ipPicASCII;
    tsv->bpe = 8;
    tsv->dim = 1;
    if( content == "" )
    {
      tsv->value = NULL;
      tsv->n[0] = 0;
    }
    else
    {
      tsv->value = strdup( content.c_str() );
      tsv->n[0] = strlen( content.c_str() );
    }
    return tsv;
  }
  else return 0;
}

//create an int-pic-tag
ipPicTSV_t* mitk::ImageToPicDescriptor::CreateIntTag( std::string description, int content )
{
  if( description != "" )
  {
    int* intTag = new int( content );
    ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
    strcpy( tsv->tag, description.c_str() );
    tsv->type = ipPicInt;
    tsv->bpe = 32;
    tsv->dim = 1;
    tsv->value = intTag;
    tsv->n[0] = 1;
    return tsv;
  }
  else return 0;
}

//create a unsigned-int-pic-tag (tagPATIENT_SEX is the only one)
ipPicTSV_t* mitk::ImageToPicDescriptor::CreateUIntTag( std::string description, int content )
{
  if( description != "" )
  {
    int* uIntTag = new int( content );
    ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
    strcpy( tsv->tag, description.c_str() );
    tsv->type = ipPicUInt;
    tsv->bpe = 8;
    tsv->dim = 1;
    tsv->value = uIntTag;
    tsv->n[0] = 1;
    return tsv;
  }
  else return 0;
}

//delete a pic-tag from the given Picdescriptor
void mitk::ImageToPicDescriptor::DeleteTag( ipPicDescriptor* cur, std::string description )
{
  if( description != "" && cur != NULL )
  {
    //search
    ipPicTSV_t* tag = ipPicQueryTag( cur, (char*)description.c_str() );
    if( tag != NULL )
    {
      //delete if exist
      ipPicTSV_t *tsvSH;
      tsvSH = ipPicDelTag( cur, (char*)description.c_str() );
      ipPicFreeTag( tsvSH );
    }
  }
}

// set the input
void mitk::ImageToPicDescriptor::SetInput( Image* sourceImage, ChiliPlugin::TagInformationList studyPatientAndSeriesTags, LevelWindow levelWindow )
{
  m_SourceImage = sourceImage;
  m_levelWindow = levelWindow;
  m_StudyPatientAndSeriesTags = studyPatientAndSeriesTags;
  //m_PropertyList = NULL;
}

/*
void mitk::ImageToPicDescriptor::SetInput( Image* sourceImage, const mitk::PropertyList::Pointer propertyList, LevelWindow levelWindow )
{
  m_SourceImage = sourceImage;
  m_levelWindow = levelWindow;
  m_StudyPatientAndSeriesTags.clear();
  m_PropertyList = propertyList;
}
*/

//this function separate a mitk::image into a list of ipPicDescriptor
void mitk::ImageToPicDescriptor::Write()
{
  m_Output.clear();

  if( m_SourceImage == NULL ) return;

  //used var
  ipPicDescriptor* cur;
  int s, t;
  int smax, tmax;
  int number = 1;
  bool firstTime = true;
  std::string newSeriesInstanceUID, newSeriesDate, newSeriesTime;
  ImageSliceSelector::Pointer resultslice = ImageSliceSelector::New();
  mitk::SlicedGeometry3D* SlicedGeometry;
  mitk::Geometry3D* geometry3DofSlice;
  Point3D origin;
  Vector3D v;
  interSliceGeometry_t isg;

  if( !m_StudyPatientAndSeriesTags.empty() )
  //get the information you needed for a new series (only one time)
  {
    ipPicDescriptor* getNeededSeriesInformations = ipPicNew();
    QcPlugin::addTags( getNeededSeriesInformations, getNeededSeriesInformations, true );
    //SERIES_INSTANCE_UID
    ipPicTSV_t* seriesInstanceUIDQuery = ipPicQueryTag( getNeededSeriesInformations, tagSERIES_INSTANCE_UID );
    if( seriesInstanceUIDQuery )
      newSeriesInstanceUID = static_cast<char*>( seriesInstanceUIDQuery->value );
    else
      newSeriesInstanceUID = "";
    //SERIES_DATE
    ipPicTSV_t* seriesDateQuery = ipPicQueryTag( getNeededSeriesInformations, tagSERIES_DATE );
    if( seriesDateQuery )
      newSeriesDate = static_cast<char*>( seriesDateQuery->value );
    else
      newSeriesDate = "";
    //SERIES_TIME
    ipPicTSV_t* seriesTimeQuery = ipPicQueryTag( getNeededSeriesInformations, tagSERIES_TIME );
    if( seriesTimeQuery )
      newSeriesTime = static_cast<char*>( seriesTimeQuery->value );
    else
      newSeriesTime = "";
  }

  resultslice->SetInput( m_SourceImage );
  smax = m_SourceImage->GetDimension(2);
  tmax = m_SourceImage->GetDimension(3);

  //GeomtryInformation
  SlicedGeometry = m_SourceImage->GetSlicedGeometry();
  origin = SlicedGeometry->GetCornerPoint();
  memcpy( isg.forUID, mitk::FrameOfReferenceUIDManager::GetFrameOfReferenceUID(SlicedGeometry->GetFrameOfReferenceID()), 128 );
  isg.psu = ipPicUtilMillimeter;

  for( s = smax-1; s >= 0; --s )  //Slices
  {
    resultslice->SetSliceNr(s);
    for( t = 0; t < tmax; ++t )  //Time
    {
      resultslice->SetTimeNr(t);
      resultslice->Update();
      //get current slice
      cur = ipPicClone( resultslice->GetOutput()->GetPic() );  //if you dont do, you change the mitk::image

      //create a new series
      if( !m_StudyPatientAndSeriesTags.empty() )
      {
        //delete the complete Pic-Header
        while( cur->info->tags_head != NULL )
        {
          _ipPicTagsElement_t* currentTagNode = cur->info->tags_head;
          cur->info->tags_head = cur->info->tags_head->next;
          ipPicFreeTag( currentTagNode->tsv );
          free( currentTagNode );
        }

        //add the elemental SeriesInformation
        ipPicAddTag( cur, CreateASCIITag( tagSERIES_INSTANCE_UID, newSeriesInstanceUID ) );
        ipPicAddTag( cur, CreateASCIITag( tagSERIES_DATE, newSeriesDate ) );
        ipPicAddTag( cur, CreateASCIITag( tagSERIES_TIME, newSeriesTime ) );

        //create the needed ImageTags
        ipPicDescriptor* getMissingImageTags = ipPicNew();
        QcPlugin::addTags( getMissingImageTags, getMissingImageTags, false );

        // put the generated Image-tags to the slice
        ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( getMissingImageTags, tagIMAGE_INSTANCE_UID );
        if( missingImageTagQuery )
        {
          ipPicAddTag( cur, CreateASCIITag( tagIMAGE_INSTANCE_UID, static_cast<char*>( missingImageTagQuery->value ) ) );
        }
        missingImageTagQuery = ipPicQueryTag( getMissingImageTags, tagIMAGE_DATE );
        if( missingImageTagQuery )
        {
          ipPicAddTag( cur, CreateASCIITag( tagIMAGE_DATE, static_cast<char*>( missingImageTagQuery->value ) ) );
        }
        missingImageTagQuery = ipPicQueryTag( getMissingImageTags, tagIMAGE_TIME );
        if( missingImageTagQuery )
        {
          ipPicAddTag( cur, CreateASCIITag( tagIMAGE_TIME, static_cast<char*>( missingImageTagQuery->value ) ) );
        }
        ipPicAddTag( cur, CreateIntTag( tagIMAGE_NUMBER, number ) );
        number++;

        //add the Study-, Patient- and Series-Tags
        for( mitk::ChiliPlugin::TagInformationList::iterator iter = m_StudyPatientAndSeriesTags.begin(); iter != m_StudyPatientAndSeriesTags.end(); iter ++ )
        {
          // some tags are not string
          if( (*iter).PicTagDescription == tagSERIES_ACQUISITION || (*iter).PicTagDescription == tagSERIES_NUMBER || (*iter).PicTagDescription == tagSERIES_ECHO_NUMBER || (*iter).PicTagDescription == tagSERIES_TEMPORAL_POSITION )
          {
            std::stringstream ssStream( (*iter).PicTagContent.c_str() );
            int tempTag;
            ssStream >> tempTag;
            ipPicAddTag( cur, CreateIntTag( (*iter).PicTagDescription , tempTag ) );
          }
          else
          {
            if( (*iter).PicTagDescription == tagPATIENT_SEX )
            {
              ipPicAddTag( cur, CreateUIntTag( (*iter).PicTagDescription , (int)*(*iter).PicTagContent.c_str() ) );
            }
            else
            {
              ipPicAddTag( cur, CreateASCIITag( (*iter).PicTagDescription , (*iter).PicTagContent ) );
            }
          }
        }

        QcPlugin::addIcon( cur, true );  //create an icon for the lightbox
        QcPlugin::addLevelWindow( cur, (int)(m_levelWindow.GetLevel()), (int)(m_levelWindow.GetWindow()) );  //add the Level/Window
      }

/*
      else
      //add all Chili-Properties from the propertyList
      {
        for( mitk::PropertyList::PropertyMap::const_iterator iter = m_PropertyList->GetMap()->begin(); iter != m_PropertyList->GetMap()->end(); iter++ )
        {
          if( !iter->first.find("CHILI: ", 0) )
          {
            mitk::BaseProperty* unknownProperty = iter->second.first;
            if( mitk::PicHeaderProperty* picHeader = dynamic_cast<mitk::PicHeaderProperty*>( unknownProperty ) )
            {
              ipPicTSV_t* picClone = _ipPicCloneTag( picHeader->GetValue() );
              ipPicAddTag( cur, picClone );
            }
            else
            {
              std::cout << "mitkImageToPicDescriptor (Write): Found Chili-Property which is no PicHeaderProperty." << std::endl;
            }
          }
        }
      }
*/

      //the following passage have to be used, if a new series create or not
      //dont put it in the passage "if( !m_StudyPatientAndSeriesTags.empty() ) ...", then it is not possible to load override images

      //create the GeometryInformation
      geometry3DofSlice = SlicedGeometry->GetGeometry2D(s);
      v=geometry3DofSlice->GetOrigin().GetVectorFromOrigin();
      itk2vtk(v, isg.o);

      v-=origin;
      isg.sl = v.GetNorm();

      v.Set_vnl_vector(geometry3DofSlice->GetMatrixColumn(0));
      isg.ps[0]=v.GetNorm();
      v/=isg.ps[0];
      itk2vtk(v, isg.u);
      v.Set_vnl_vector(geometry3DofSlice->GetMatrixColumn(1));
      isg.ps[1]=v.GetNorm();
      v/=isg.ps[1];
      itk2vtk(v, isg.v);
      v.Set_vnl_vector(geometry3DofSlice->GetMatrixColumn(2));
      isg.ps[2]=v.GetNorm();
      v/=isg.ps[2];
      itk2vtk(v, isg.w);

      cur->dim = 2;

      if( !m_StudyPatientAndSeriesTags.empty() )
      {
        QcPlugin::addDicomHeader( cur, &isg );  //create the Dicom-Header with the new GeomtryInformation
      }

      //add slice to output
      m_Output.push_back( cur );
    }
  }
}

// return the generated output
std::list<ipPicDescriptor*> mitk::ImageToPicDescriptor::GetOutput()
{
  return m_Output;
}

