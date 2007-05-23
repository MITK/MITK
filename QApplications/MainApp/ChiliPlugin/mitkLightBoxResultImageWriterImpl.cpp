/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <chili/plugin.h>
#include <chili/qclightbox.h>
#include <chili/qclightboxmanager.h>
//Chili-Tags
#include <ipPic/ipPicTags.h>
//reading single-slices
#include <mitkImageSliceSelector.h>
#include <list>
//geometry
#include <chili/isg.h>
#include <mitkFrameOfReferenceUIDManager.h>

mitk::LightBoxResultImageWriterImpl::LightBoxResultImageWriterImpl()
{
  m_LightBox = NULL;
  m_LevelWindow = -1;
  m_Image = NULL;
  m_SeriesDescription = "";
}

mitk::LightBoxResultImageWriterImpl::~LightBoxResultImageWriterImpl()
{
}

void mitk::LightBoxResultImageWriterImpl::SetInput(const mitk::Image* image)
{
  m_Image=image;
}

void mitk::LightBoxResultImageWriterImpl::SetLightBox(QcLightbox* lightbox)
{
  if(lightbox!=m_LightBox)
  {
    m_LightBox=lightbox;
    Modified();
  }
}

void mitk::LightBoxResultImageWriterImpl::SetSeriesDescription( const std::string& description )
{
  m_SeriesDescription = description;
}

void mitk::LightBoxResultImageWriterImpl::SetLightBoxToCurrentLightBox()
{
  QcPlugin* plugin = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();
  if(plugin==NULL)
  {
    itkExceptionMacro(<<"GetPluginInstance()==NULL: Plugin is not initialized correctly !");
  }
  SetLightBox(plugin->lightboxManager()->getActiveLightbox());
}

bool mitk::LightBoxResultImageWriterImpl::SetLightBoxToNewLightBox()
{
  QcPlugin* plugin = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();
  if(plugin==NULL)
  {
    itkExceptionMacro(<<"GetPluginInstance()==NULL: Plugin is not initialized correctly !");
  }
  int lightboxCount = plugin->lightboxManager()->getLightboxes().count();
  QcLightbox * newLightbox;
  for (int i = 0; i < lightboxCount; i++)
  {
    newLightbox = plugin->lightboxManager()->getLightboxes().at(i);
    if (newLightbox->getFrames() == 0)
    {
      newLightbox->activate();
      newLightbox->show();
      SetLightBox(newLightbox);
      return true;
    }
  }
  return false;
}

void mitk::LightBoxResultImageWriterImpl::SetLevelWindow(LevelWindow levelwindow)
{
  if(m_LevelWindow!=levelwindow)
  {
    m_LevelWindow = levelwindow;
    Modified();
  }
}

void mitk::LightBoxResultImageWriterImpl::Write()
{
  //reasons to abort
  if( m_Image == NULL )
  {
    std::cout<< "Image not set." <<std::endl;
    return;
  }
  mitk::ChiliPlugin::StudyInformation CurrentStudy = mitk::ChiliPlugin::GetInstance()->GetCurrentStudy();
  if( CurrentStudy.StudyInstanceUID == "" )
  {
    std::cout<< "There is no Study (no StudyInstanceUID) selected." <<std::endl;
    return;
  }
  //check the input
  if( m_LevelWindow == -1 )
    m_LevelWindow.SetAuto( m_Image );
  if( m_LightBox == NULL )
    SetLightBoxToNewLightBox();
  if( m_SeriesDescription == "" )
    m_SeriesDescription = "empty";

  bool firstTime = true;
  ipPicDescriptor* firstSlice = new ipPicDescriptor();

  m_LightBox->clear();

  //captions and content for the tags
  std::string MyTags[6][2]=
  {
    { tagMODALITY, CurrentStudy.Modality },
    { tagSTUDY_DESCRIPTION, CurrentStudy.StudyDescription },
    { tagSTUDY_DATE, CurrentStudy.StudyDate },
    { tagSTUDY_TIME, CurrentStudy.StudyTime },
    { tagSTUDY_INSTANCE_UID, CurrentStudy.StudyInstanceUID },
    { tagSERIES_DESCRIPTION, m_SeriesDescription.c_str() }
  };

  typedef std::list<ipPicTSV_t*> TagList;
  TagList* MyTagList = new TagList();

  for(int x = 0; x < 6; x++)
  {
     //create the ASCII-tags
     ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
     strcpy( tsv->tag, MyTags[x][0].c_str() );
     tsv->type = ipPicASCII;
     tsv->bpe = 8;
     tsv->dim = 1;
     tsv->value = strdup( MyTags[x][1].c_str() );
     tsv->n[0] = strlen( MyTags[x][1].c_str() );
     //add to list
     MyTagList->push_back( tsv );
  }
  //SERIES NUMBER is an INT-tag
  int* tagSeriesNumber = new int( mitk::ChiliPlugin::GetInstance()->GetCurrentSeries().size()+1 );
  ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv->tag, tagSERIES_NUMBER );
  tsv->type = ipPicInt;
  tsv->bpe = 32;
  tsv->dim = 1;
  tsv->value = tagSeriesNumber;
  tsv->n[0] = 1;
  MyTagList->push_back( tsv );

  ipPicDescriptor *cur;
  ipPicTSV_t* tag;
  int s, t;
  int smax, tmax;
  int number = 1;

  ImageSliceSelector::Pointer resultslice = ImageSliceSelector::New();
  resultslice->SetInput( m_Image );
  smax = m_Image->GetDimension(2);
  tmax = m_Image->GetDimension(3);

  //GeomtryInformation
  mitk::SlicedGeometry3D* SlicedGeometry;
  mitk::Geometry3D* geometry3DofSlice;
  Point3D origin;
  Vector3D v;

  SlicedGeometry = m_Image->GetSlicedGeometry();
  origin = SlicedGeometry->GetCornerPoint();

  interSliceGeometry_t isg;
  memcpy(isg.forUID, mitk::FrameOfReferenceUIDManager::GetFrameOfReferenceUID(SlicedGeometry->GetFrameOfReferenceID()), 128);
  isg.psu = ipPicUtilMillimeter;

  //Slices
  for( s = smax-1; s >= 0; --s )
  {
    resultslice->SetSliceNr(s);
    //Time
    for( t = 0; t < tmax; ++t )
    {
      resultslice->SetTimeNr(t);
      resultslice->Update();
      cur = ipPicClone(resultslice->GetOutput()->GetPic());

      //the first slice got a new pic-header, the other become a copy
      if( firstTime )
      {
        firstTime = false;
        //to create a new series use addTags( ..., ..., true)
        //we want to create one new series, so we use this for the first slice only (for the other slices look at --> else ... )
        QcPlugin::addTags( cur, cur, true );

        while( !MyTagList->empty() )
        {
          //if one tag which we created always exist
          char* tagName = MyTagList->front()->tag;
          tag = ipPicQueryTag( cur, tagName );
          if( tag != NULL )
          {
            //delete it
            ipPicTSV_t *tsvSH;
            tsvSH = ipPicDelTag( cur, tagName );
            ipPicFreeTag(tsvSH);
          }
          //add own tag
          ipPicAddTag( cur, MyTagList->front() );
          MyTagList->pop_front();
        }
        //save the first slice
        firstSlice = cur;
      }
      else QcPlugin::addTags ( cur, firstSlice , false );

      //delete the SOURCE HEADER if exist
      tag = ipPicQueryTag( cur, "SOURCE HEADER" );
      if( tag != NULL )
      {
        ipPicTSV_t *tsvSH;
        tsvSH = ipPicDelTag( cur, "SOURCE HEADER" );
        ipPicFreeTag(tsvSH);
      }

      //delete the IMAGE NUMBER
      tag = ipPicQueryTag( cur, tagIMAGE_NUMBER );
      if( tag != NULL )
      {
        ipPicTSV_t *tsvSH;
        tsvSH = ipPicDelTag( cur, tagIMAGE_NUMBER );
        ipPicFreeTag(tsvSH);
      }
      //create a new one
      int* tagImageNumber = new int(number);
      ipPicTSV_t* tsv = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
      strcpy( tsv->tag, tagIMAGE_NUMBER );
      tsv->type = ipPicInt;
      tsv->bpe = 32;
      tsv->dim = 1;
      tsv->value = tagImageNumber;
      tsv->n[0] = 1;
      ipPicAddTag( cur, tsv );
      number++;

      //delete the STUDY ID
      tag = ipPicQueryTag( cur, tagSTUDY_ID );
      if( tag != NULL )
      {
        ipPicTSV_t *tsvSH;
        tsvSH = ipPicDelTag( cur, tagSTUDY_ID );
        ipPicFreeTag(tsvSH);
      }
      //create a new one (QcPlugin::addTag dont copy it)
      ipPicTSV_t* tsv1 = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
      strcpy( tsv1->tag, tagSTUDY_ID );
      tsv1->type = ipPicASCII;
      tsv1->bpe = 8;
      tsv1->dim = 1;
      tsv1->value = strdup( CurrentStudy.StudyID.c_str() );
      tsv1->n[0] = strlen( CurrentStudy.StudyID.c_str() );
      ipPicAddTag( cur, tsv1 );

      //create an icon for the lightbox
      QcPlugin::addIcon( cur, true );
      //Level/Window
      QcPlugin::addLevelWindow( cur, (int)(m_LevelWindow.GetLevel()), (int)(m_LevelWindow.GetWindow()) );

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

      //create the Dicom-Header with the GeomtryInformation
      QcPlugin::addDicomHeader( cur, &isg );
      //add the pic to the lightbox
      m_LightBox->addImage( cur );
    }
  }
  delete MyTagList;
}
