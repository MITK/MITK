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

#include "mitkLightBoxResultImageWriterImpl.h"
#include "mitkChiliPlugin.h"

//ChiliIncludes
#include <chili/qclightbox.h>
#include <chili/qclightboxmanager.h>
//Chili-Tags
#include <ipPic/ipPicTags.h>
//reading single-slices
#include <mitkImageSliceSelector.h>
//geometry
#include <chili/isg.h>
#include <mitkFrameOfReferenceUIDManager.h>

#include <mitkPropertyList.h>
#include "mitkProperties.h"
#include <mitkGenericProperty.h>
#include <mitkLevelWindowProperty.h>

#include <qmessagebox.h>

mitk::LightBoxResultImageWriterImpl::LightBoxResultImageWriterImpl()
{
  m_LightBox = NULL;
  m_LevelWindow = -1;
  m_Image = NULL;
  m_SeriesDescription = "";
  m_PropertyList = NULL;
}

mitk::LightBoxResultImageWriterImpl::~LightBoxResultImageWriterImpl()
{
}

void mitk::LightBoxResultImageWriterImpl::SetInput( const mitk::Image* image )
{
  m_Image = image;
}

void mitk::LightBoxResultImageWriterImpl::SetInputByDataTreeNode( const mitk::DataTreeNode* node )
{
  if ( node )
  {
    mitk::BaseData* data = node->GetData();
    if ( data )
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>( data );
      if ( image )
      {
        m_Image = image;
        m_PropertyList = node->GetPropertyList();
        m_SeriesDescription = node->GetPropertyList()->GetProperty( "name" )->GetValueAsString();
        mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>( node->GetProperty("levelwindow").GetPointer() );
        if( levelWindowProperty.IsNotNull() )
          m_LevelWindow = levelWindowProperty->GetLevelWindow();
        else
          m_LevelWindow = -1;
      }
      else m_Image = NULL;
    }
    else m_Image = NULL;
  }
  else m_Image = NULL;
}

void mitk::LightBoxResultImageWriterImpl::SetLightBox( QcLightbox* lightbox )
{
  if(lightbox != m_LightBox)
  {
    m_LightBox = lightbox;
    Modified();
  }
}

void mitk::LightBoxResultImageWriterImpl::SetSeriesDescription( const std::string& description )
{
  m_SeriesDescription = description;
}

void mitk::LightBoxResultImageWriterImpl::SetLightBoxToCurrentLightBox()
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  if( !mitk::PACSPlugin::GetInstance()->IsPlugin() )
  {
    itkExceptionMacro(<<"GetPluginInstance()==NULL: Plugin is not initialized correctly !");
  }
  SetLightBox( realPluginInstance->lightboxManager()->getActiveLightbox() );
}

bool mitk::LightBoxResultImageWriterImpl::SetLightBoxToNewLightBox()
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  if( !mitk::PACSPlugin::GetInstance()->IsPlugin() )
  {
    itkExceptionMacro(<<"GetPluginInstance()==NULL: Plugin is not initialized correctly !");
  }
  int lightboxCount = realPluginInstance->lightboxManager()->getLightboxes().count();
  QcLightbox * newLightbox;
  for( int i = 0; i < lightboxCount; i++ )
  {
    newLightbox = realPluginInstance->lightboxManager()->getLightboxes().at(i);
    if( newLightbox->getFrames() == 0 )
    {
      newLightbox->activate();
      newLightbox->show();
      SetLightBox( newLightbox );
      return true;
    }
  }
  return false;
}

void mitk::LightBoxResultImageWriterImpl::SetLevelWindow( LevelWindow levelwindow )
{
  if( m_LevelWindow!=levelwindow )
  {
    m_LevelWindow = levelwindow;
    Modified();
  }
}

void mitk::LightBoxResultImageWriterImpl::SetPropertyList( const mitk::PropertyList::Pointer property )
{
  m_PropertyList = property;
}

ipPicTSV_t* mitk::LightBoxResultImageWriterImpl::CreateASCIITag( std::string Description, std::string Content )
{
  if( Description != NULL && Content != NULL )
  {
    ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
    strcpy( tsv->tag, Description.c_str() );
    tsv->type = ipPicASCII;
    tsv->bpe = 8;
    tsv->dim = 1;
    tsv->value = strdup( Content.c_str() );
    tsv->n[0] = strlen( Content.c_str() );
    return tsv;
  }
  else return 0;
}

ipPicTSV_t* mitk::LightBoxResultImageWriterImpl::CreateIntTag( std::string Description, int Content )
{
  if( Description != NULL )
  {
    int* IntTag = new int( Content );
    ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
    strcpy( tsv->tag, Description.c_str() );
    tsv->type = ipPicInt;
    tsv->bpe = 32;
    tsv->dim = 1;
    tsv->value = IntTag;
    tsv->n[0] = 1;
    return tsv;
  }
  else return 0;
}

ipPicTSV_t* mitk::LightBoxResultImageWriterImpl::CreateUIntTag( std::string Description, int Content )
{
  if( Description != NULL )
  {
    int* UIntTag = new int( Content );
    ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
    strcpy( tsv->tag, Description.c_str() );
    tsv->type = ipPicUInt;
    tsv->bpe = 8;
    tsv->dim = 1;
    tsv->value = UIntTag;
    tsv->n[0] = 1;
    return tsv;
  }
  else return 0;
}

void mitk::LightBoxResultImageWriterImpl::DeleteTag( ipPicDescriptor* cur, std::string Description )
{
  if( Description != NULL && cur != NULL )
  {
    //search
    ipPicTSV_t* tag = ipPicQueryTag( cur, (char*)Description.c_str() );
    if( tag != NULL )
    {
      //delete if exist
      ipPicTSV_t *tsvSH;
      tsvSH = ipPicDelTag( cur, (char*)Description.c_str() );
      ipPicFreeTag(tsvSH);
    }
  }
}

void mitk::LightBoxResultImageWriterImpl::Write()
{
  //check + default
  if( m_Image == NULL )
  {
    std::cout<< "No input set." <<std::endl;
    return;
  }
  mitk::PACSPlugin::StudyInformation CurrentStudy = mitk::PACSPlugin::GetInstance()->GetCurrentSelectedStudy();
  if( CurrentStudy.InstanceUID == "" )
  {
    std::cout<< "There is no Study (no StudyInstanceUID) selected." <<std::endl;
    return;
  }

  if( m_PropertyList.IsNotNull() )
  {
    std::string temp = "Chili: " + (std::string)tagSTUDY_INSTANCE_UID;
    BaseProperty* tempprop = m_PropertyList->GetProperty( temp.c_str() );
    if( tempprop != NULL )
    {
      temp = tempprop->GetValueAsString();
      if( CurrentStudy.InstanceUID != temp )
      {
        switch(QMessageBox::question( 0, "Save To Lightbox", "This data was not loaded from this study. Do you want to save?",
          QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel))
        {
          case QMessageBox::Yes: break;
          case QMessageBox::No: return;
          case QMessageBox::Cancel: return;
        }
      }
    }
  }

  if( m_LevelWindow == -1 )
    m_LevelWindow.SetAuto( m_Image );
  if( m_LightBox == NULL )
    SetLightBoxToNewLightBox();
  if( m_SeriesDescription == "" )
    m_SeriesDescription = "empty";
  m_LightBox->clear();

  //used var
  ipPicDescriptor* cur;
  int s, t;
  int smax, tmax;
  int number = 1;
  bool firstTime = true;
  ImageSliceSelector::Pointer resultslice = ImageSliceSelector::New();
  ipPicDescriptor* firstSlice = new ipPicDescriptor();
  mitk::SlicedGeometry3D* SlicedGeometry;
  mitk::Geometry3D* geometry3DofSlice;
  Point3D origin;
  Vector3D v;
  interSliceGeometry_t isg;

  //captions and content for the tags which are needed to create a new series; DONT CHANGE THEM!!!
  std::string CurrentSelectedStudyAttributeToSave[6][2]=
  {
    { tagMODALITY, CurrentStudy.Modality },
    { tagSTUDY_DESCRIPTION, CurrentStudy.Description },
    { tagSTUDY_DATE, CurrentStudy.Date },
    { tagSTUDY_TIME, CurrentStudy.Time },
    { tagSTUDY_INSTANCE_UID, CurrentStudy.InstanceUID },
    { tagSERIES_DESCRIPTION, m_SeriesDescription.c_str() }
  };

  //while walking through the "CurrentSelectedStudyAttributeToSave" we delete the same tags from the propertyList
  //the lightboximagereader read them
  //in the propertylist stay the old study association, we dont want to save to the "old" study
  //we want to save to a new study, so we have to use the "CurrentSelectedStudyAttributeToSave"
  //later the propertylist get write to the PIC-Header, then stands there the right attribute to save
  for(int x = 0; x < 6; x++)
  {
    std::string temp = "Chili: " + CurrentSelectedStudyAttributeToSave[x][0];
    m_PropertyList->DeleteProperty( temp.c_str() );
    m_PropertyList->SetProperty( temp.c_str(), new StringProperty( CurrentSelectedStudyAttributeToSave[x][1] ) );
  }

  std::string temp = "Chili: " + (std::string)tagSERIES_NUMBER;
  m_PropertyList->DeleteProperty( temp.c_str() );
  m_PropertyList->SetProperty( temp.c_str(), new IntProperty( mitk::PACSPlugin::GetInstance()->GetCurrentSelectedSeries().size()+1 ) );

  resultslice->SetInput( m_Image );
  smax = m_Image->GetDimension(2);
  tmax = m_Image->GetDimension(3);

  //GeomtryInformation
  SlicedGeometry = m_Image->GetSlicedGeometry();
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
      cur = ipPicClone( resultslice->GetOutput()->GetPic() );

      if( firstTime )  //the first slice got a new pic-header, the other become a copy of the first one
      {
        QcPlugin::addTags( cur, cur, true );  //create a new SeriesInstanceUID and delete the old one
        firstSlice = cur;  //save the first slice
        firstTime = false;
      }
      else QcPlugin::addTags ( cur, firstSlice , false );

      //add the properties to the pic-header
      if( m_PropertyList.IsNotNull() )
      {
        std::string Description;
        for( mitk::PropertyList::PropertyMap::const_iterator iter = m_PropertyList->GetMap()->begin(); iter != m_PropertyList->GetMap()->end(); iter++ )
        {
          if( !iter->first.find("Chili: ", 0) )
          {
            Description = iter->first;
            Description = Description.erase( 0, 7 );  //delete "Chili: "
            DeleteTag( cur, Description );  //delete the tag if exist
            mitk::BaseProperty* unknownProperty = NULL;
            unknownProperty = iter->second.first;
            if( mitk::StringProperty* StringProp = dynamic_cast<mitk::StringProperty*>( unknownProperty ) )
              ipPicAddTag( cur, CreateASCIITag( Description, StringProp->GetValueAsString() ) );
            else
              if( mitk::GenericProperty<int>* IntProp = dynamic_cast<mitk::GenericProperty<int>*>( unknownProperty ) )
                if( Description == tagPATIENT_SEX ) //actually the only one with type ipPicUInt
                  ipPicAddTag( cur, CreateUIntTag( Description, IntProp->GetValue() ) );
                else
                  ipPicAddTag( cur, CreateIntTag( Description, IntProp->GetValue() ) );
          }
        }
      }

      DeleteTag( cur, "SOURCE HEADER" );  //delete the SOURCE HEADER (thats the binary Dicom-Header)
      DeleteTag( cur, tagIMAGE_NUMBER );  //delete the IMAGE NUMBER and create a new one
      ipPicAddTag( cur, CreateIntTag( tagIMAGE_NUMBER, number ) );
      number++;
      DeleteTag( cur, tagSTUDY_ID );  //delete the STUDY ID and create a new one (QcPlugin::addTag dont copy it)
      ipPicAddTag( cur, CreateASCIITag( tagSTUDY_ID, CurrentStudy.ID ) );
      DeleteTag( cur, tagSTUDY_DESCRIPTION );  //delete the STUDY DESCRIPTION and create a new one (QcPlugin::addTag dont copy it, if it "")
      ipPicAddTag( cur, CreateASCIITag( tagSTUDY_DESCRIPTION, CurrentStudy.Description ) );
      QcPlugin::addIcon( cur, true );  //create an icon for the lightbox
      QcPlugin::addLevelWindow( cur, (int)(m_LevelWindow.GetLevel()), (int)(m_LevelWindow.GetWindow()) );  //add the Level/Window

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

      QcPlugin::addDicomHeader( cur, &isg );  //create the Dicom-Header with the GeomtryInformation
      m_LightBox->addImage( cur );  //add the pic to the lightbox
    }
  }
}
