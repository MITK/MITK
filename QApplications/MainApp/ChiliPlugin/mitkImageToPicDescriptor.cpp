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

#include <chili/isg.h>
#include <chili/plugin.h>
#include <ipDicom/ipDicom.h>
#include <ipPic/ipPicTags.h>

#include "mitkFrameOfReferenceUIDManager.h"
#include "mitkImageSliceSelector.h"

/** Helper class for property import from pic/dicom-headers. */
class HeaderTagInfo
{
  public:

    typedef enum { MissingInDicom, String, Int, UnsignedInt } DataType;

    HeaderTagInfo( const char* ppicTagKey, ipUInt2_t pdicomGroup, ipUInt2_t pdicomElement, DataType ptype )
    : picTagKey( ppicTagKey ), dicomGroup( pdicomGroup ), dicomElement( pdicomElement ), type( ptype )
    {
    }

  std::string picTagKey;
  ipUInt2_t dicomGroup;
  ipUInt2_t dicomElement;
  DataType type;
};

/** Constructor */
mitk::ImageToPicDescriptor::ImageToPicDescriptor()
{
  m_LevelWindowInitialized = false;
  m_TagListInitialized = false;
  m_ImageNumberInitialized = false;
}

/** Destructor */
mitk::ImageToPicDescriptor::~ImageToPicDescriptor()
{
}

/** Create a string-pic-tag. */
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

/** Create an int-pic-tag. */
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

/** Create a unsigned-int-pic-tag (tagPATIENT_SEX current the only one). */
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

/** Delete a pic-tag from the given Picdescriptor. */
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

/** Set the image. The Image have to be set, otherwise update dont work. */
void mitk::ImageToPicDescriptor::SetImage( Image* sourceImage)
{
  m_SourceImage = sourceImage;
}

/** Set the levelwindow. This function can be use. If the levelwindow dont get set, it will be created with SetAuto(). */
void mitk::ImageToPicDescriptor::SetLevelWindow( LevelWindow levelWindow )
{
  m_LevelWindow = levelWindow;
  m_LevelWindowInitialized = true;
}

/** Set the tagList. "useSavedPicTags" decided if the picDescriptor create new slices (false), or override existing slices (true). To override existing slices all pic-tags have to be the same. So the tags dont get changed. If you want to create new slices, the new slices needed the current Date, Time, ImageInstanceUID, ... . With false all this tags created and added to the ipPicDescriptors. But therefore the Patient-, Study- and Series-Information needed. This provide the inputTags. The mitkChiliPlugin have a function to create the needed one. This function have to be use, otherwise update dont work.*/
void mitk::ImageToPicDescriptor::SetTagList( TagInformationList inputTags, bool useSavedPicTags )
{
  m_TagList = inputTags;
  m_UseSavedPicTags = useSavedPicTags;
  m_TagListInitialized = true;
}

/** This function set the imageNumber. If the picdescriptors added to an existing series, the image number sould not be twice. Then the slice-stack-reader can splitt the results better. Therefor you can set the start-number. This function can be use. If the imageNumber dont set, the number set to one. Bu its recommend to set the ImageNumber! */
void mitk::ImageToPicDescriptor::SetImageNumber( int imageNumber )
{
  m_ImageNumber = imageNumber;
  m_ImageNumberInitialized = true;
}

/** This function separate a mitk::image into a list of ipPicDescriptor. If no input set before, the function create an empty output. */
void mitk::ImageToPicDescriptor::Update()
{
  m_Output.clear();
  m_imageInstanceUIDs.clear();

  if( m_SourceImage.IsNull() || !m_TagListInitialized ) return;

  if( !m_LevelWindowInitialized )
    m_LevelWindow.SetAuto( m_SourceImage );

  if( !m_ImageNumberInitialized )
    m_ImageNumber = 1;

  // used var
  ipPicDescriptor* currentPicDescriptor;
  int slice, time;
  int maxSlice, maxTime;
  ImageSliceSelector::Pointer resultSlice = ImageSliceSelector::New();
  mitk::SlicedGeometry3D* slicedGeometry;
  mitk::Geometry3D* geometry3DofSlice;
  Point3D origin;
  Vector3D v;
  interSliceGeometry_t isg;

  resultSlice->SetInput( m_SourceImage );
  maxSlice = m_SourceImage->GetDimension(2);
  maxTime = m_SourceImage->GetDimension(3);

  // GeomtryInformation
  slicedGeometry = m_SourceImage->GetSlicedGeometry();
  origin = slicedGeometry->GetCornerPoint();
  memcpy( isg.forUID, FrameOfReferenceUIDManager::GetFrameOfReferenceUID( slicedGeometry->GetFrameOfReferenceID() ), 128 );
  isg.psu = ipPicUtilMillimeter;

  for( slice = maxSlice - 1; slice >= 0; --slice )  // Slices
  {
    resultSlice->SetSliceNr( slice );
    for( time = 0; time < maxTime; ++time )  // Time
    {
      resultSlice->SetTimeNr( time );
      resultSlice->Update();
      // get current slice
      currentPicDescriptor = ipPicClone( resultSlice->GetOutput()->GetPic() );  // if you dont do, you change the mitk::image

      if( !m_UseSavedPicTags )
      {
        // create the needed ImageTags
        ipPicDescriptor* missingImageTags = ipPicNew();
        QcPlugin::addTags( missingImageTags, missingImageTags, false );

        // put the generated Image-tags to the slice
        // IMAGE_INSTANCE_UID
        ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( missingImageTags, tagIMAGE_INSTANCE_UID );
        if( missingImageTagQuery )
        {
          DeleteTag( currentPicDescriptor, tagIMAGE_INSTANCE_UID );
          ipPicAddTag( currentPicDescriptor, CreateASCIITag( tagIMAGE_INSTANCE_UID, static_cast<char*>( missingImageTagQuery->value ) ) );
          m_imageInstanceUIDs.push_back( static_cast<char*>( missingImageTagQuery->value ) );
        }
        // IMAGE_DATE
        missingImageTagQuery = ipPicQueryTag( missingImageTags, tagIMAGE_DATE );
        if( missingImageTagQuery )
        {
          DeleteTag( currentPicDescriptor, tagIMAGE_DATE );
          ipPicAddTag( currentPicDescriptor, CreateASCIITag( tagIMAGE_DATE, static_cast<char*>( missingImageTagQuery->value ) ) );
        }
        // IMAGE_TIME
        missingImageTagQuery = ipPicQueryTag( missingImageTags, tagIMAGE_TIME );
        if( missingImageTagQuery )
        {
          DeleteTag( currentPicDescriptor, tagIMAGE_TIME );
          ipPicAddTag( currentPicDescriptor, CreateASCIITag( tagIMAGE_TIME, static_cast<char*>( missingImageTagQuery->value ) ) );
        }
        ipPicFree( missingImageTags );
        // IMAGE_NUMBER
        DeleteTag( currentPicDescriptor, tagIMAGE_NUMBER );
        ipPicAddTag( currentPicDescriptor, CreateIntTag( tagIMAGE_NUMBER, m_ImageNumber ) );
        m_ImageNumber++;

        // add the Study-, Patient- and Series-Tags
        for( TagInformationList::iterator iter = m_TagList.begin(); iter != m_TagList.end(); iter ++ )
        {
          // some tags are not string
          if( (*iter).PicTagDescription == tagSERIES_NUMBER )
          {
            // add as INT-Tag
            std::stringstream ssStream( (*iter).PicTagContent.c_str() );
            int tempTag;
            ssStream >> tempTag;
            DeleteTag( currentPicDescriptor, (*iter).PicTagDescription );
            ipPicAddTag( currentPicDescriptor, CreateIntTag( (*iter).PicTagDescription , tempTag ) );
          }
          else
          {
            if( (*iter).PicTagDescription == tagPATIENT_SEX )
            {
              // add as UINT-Tag
              DeleteTag( currentPicDescriptor, (*iter).PicTagDescription );
              ipPicAddTag( currentPicDescriptor, CreateUIntTag( (*iter).PicTagDescription , (int)*(*iter).PicTagContent.c_str() ) );
            }
            else
            {
              // add as ASCII-Tag
              DeleteTag( currentPicDescriptor, (*iter).PicTagDescription );
              ipPicAddTag( currentPicDescriptor, CreateASCIITag( (*iter).PicTagDescription , (*iter).PicTagContent ) );
            }
          }
        }

        QcPlugin::addIcon( currentPicDescriptor, true );  // create an icon for the lightbox
        QcPlugin::addLevelWindow( currentPicDescriptor, (int)( m_LevelWindow.GetLevel() ), (int)( m_LevelWindow.GetWindow() ) );  // add the Level/Window
      }
      else
      //if the slices should override, no tags get changed, but we need the imageInstanceUID
      {
        ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( currentPicDescriptor, tagIMAGE_INSTANCE_UID );
        if( missingImageTagQuery )
          m_imageInstanceUIDs.push_back( static_cast<char*>( missingImageTagQuery->value ) );
      }

      // the following passage have to be used, if a new series create or not, i dont know why
      // dont put it in the passage "if( !m_UseSavedPicTags ) ...", then it is not possible to load override images

      // create the GeometryInformation
      geometry3DofSlice = slicedGeometry->GetGeometry2D( slice );
      v=geometry3DofSlice->GetOrigin().GetVectorFromOrigin();
      itk2vtk(v, isg.o);

      v-=origin;
      isg.sl = v.GetNorm();

      v.Set_vnl_vector(geometry3DofSlice->GetMatrixColumn( 0 ));
      isg.ps[0] = v.GetNorm();
      v/=isg.ps[0];
      itk2vtk(v, isg.u);
      v.Set_vnl_vector(geometry3DofSlice->GetMatrixColumn( 1 ));
      isg.ps[1] = v.GetNorm();
      v/=isg.ps[1];
      itk2vtk(v, isg.v);
      v.Set_vnl_vector(geometry3DofSlice->GetMatrixColumn( 2 ));
      isg.ps[2] = v.GetNorm();
      v/=isg.ps[2];
      itk2vtk(v, isg.w);

      currentPicDescriptor->dim = 2;

      if( !m_UseSavedPicTags )
      {
        CopyDicomHeaderInformationToPicHeader( currentPicDescriptor );
        DeleteTag( currentPicDescriptor, "SOURCE HEADER" );
        QcPlugin::addDicomHeader( currentPicDescriptor, &isg );
      }

      // add slice to output
      m_Output.push_back( currentPicDescriptor );
    }
  }
}

/** The Dicom-Header have to deleted, because the function QcPlugin::addDicomHeader() dont change the existing, it create a new one. If we dont want to loose to much information, we copy them from the Dicom- to Pic-Header. */
void mitk::ImageToPicDescriptor::CopyDicomHeaderInformationToPicHeader( ipPicDescriptor* pic )
{
  #define NUMBER_OF_CHILI_PIC_TAGS 15
  HeaderTagInfo tagsToImport[NUMBER_OF_CHILI_PIC_TAGS] =
  {
    HeaderTagInfo( tagPATIENT_NAME,                  0x0010, 0x0010, HeaderTagInfo::String ),
    HeaderTagInfo( tagPATIENT_SEX,                   0x0010, 0x0040, HeaderTagInfo::UnsignedInt ),
    HeaderTagInfo( tagPATIENT_ID,                    0x0010, 0x0020, HeaderTagInfo::String ),
    HeaderTagInfo( tagPATIENT_BIRTHDATE,             0x0010, 0x0030, HeaderTagInfo::String ),
    HeaderTagInfo( tagPATIENT_BIRTHTIME,             0x0010, 0x0032, HeaderTagInfo::String ),
    HeaderTagInfo( tagMEDICAL_RECORD_LOCATOR,        0x0010, 0x1090, HeaderTagInfo::String ),
    HeaderTagInfo( tagREFERING_PHYSICIAN_NAME,       0x0008, 0x0090, HeaderTagInfo::String ),
    HeaderTagInfo( tagPERFORMING_PHYSICIAN_NAME,       0x0008, 0x1050, HeaderTagInfo::String ),
    HeaderTagInfo( tagMODEL_NAME,                    0x0008, 0x1090, HeaderTagInfo::String ),
    HeaderTagInfo( tagSERIES_ECHO_NUMBER,            0x0018, 0x0086, HeaderTagInfo::Int ),
    HeaderTagInfo( tagSERIES_ACQUISITION,            0x0020, 0x1001, HeaderTagInfo::Int ),
    HeaderTagInfo( tagSERIES_CONTRAST,               0x0018, 0x0010, HeaderTagInfo::String ),
    HeaderTagInfo( tagSERIES_BODY_PART_EXAMINED,     0x0018, 0x0015, HeaderTagInfo::String ),
    HeaderTagInfo( tagSERIES_SCANNING_SEQUENCE,      0x0018, 0x0020, HeaderTagInfo::String ),
    HeaderTagInfo( tagIMAGE_TYPE,                    0x0008, 0x0008, HeaderTagInfo::String )
  };
  ipPicTSV_t* picHeader;

  for( int x = 0; x < NUMBER_OF_CHILI_PIC_TAGS; ++x )
  {
    //check if the tag exist in pic-header
    picHeader = ipPicQueryTag( pic, (char*)tagsToImport[x].picTagKey.c_str() );
    if( !picHeader )
    {
      //try to read from dicom-header
      ipPicTSV_t *dicomHeader = ipPicQueryTag( pic, "SOURCE HEADER" );
      void* data = NULL;
      ipUInt4_t len = 0;
      if( dicomHeader && dicomFindElement( (unsigned char*) dicomHeader->value, tagsToImport[x].dicomGroup, tagsToImport[x].dicomElement, &data, &len ) && data != NULL )
      {
        //found, create a pic-tag
        if( tagsToImport[x].type == HeaderTagInfo::String )
          ipPicAddTag( pic, CreateASCIITag( (char*)tagsToImport[x].picTagKey.c_str(), static_cast<char*>(data) ) );
        else
        {
          if( tagsToImport[x].type == HeaderTagInfo::UnsignedInt )
            ipPicAddTag( pic, CreateUIntTag( (char*)tagsToImport[x].picTagKey.c_str(), (int)*((char*)data) ) );
          else
          {
            if( tagsToImport[x].type == HeaderTagInfo::Int )
              ipPicAddTag( pic, CreateIntTag( (char*)tagsToImport[x].picTagKey.c_str(), atoi((char*)data) ) );
          }
        }
      }
    }
  }
}

/** Return the generated Output. */
std::list<ipPicDescriptor*> mitk::ImageToPicDescriptor::GetOutput()
{
  return m_Output;
}

/** Return the saved ImageInstanceUIDs. The imageInstanceUIDs get used to identify the single slices. The single slices addicted a volume. Volumes have parent-child-relations. Therefore they needed. */
std::list< std::string > mitk::ImageToPicDescriptor::GetSaveImageInstanceUIDs()
{
  return m_imageInstanceUIDs;
}

