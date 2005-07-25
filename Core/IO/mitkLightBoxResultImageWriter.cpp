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


#include "mitkLightBoxResultImageWriter.h"

#ifdef CHILIPLUGIN
#include <chili/plugin.h>
#include <chili/qclightbox.h>
#include <chili/qclightboxmanager.h>
#include <chili/isg.h>
#endif

#include <mitkImage.h>
#include <mitkImageSliceSelector.h>
#include <mitkFrameOfReferenceUIDManager.h>
#include <mitkChiliPlugin.h>

#include <itkRootTreeIterator.h>
#include <itkImageFileReader.h>



mitk::LightBoxResultImageWriter::LightBoxResultImageWriter()
{
  m_ImageTypeName = "Image, Segmentation";
}

mitk::LightBoxResultImageWriter::~LightBoxResultImageWriter()
{
}

const mitk::Image *mitk::LightBoxResultImageWriter::GetInput(void)
{
  return static_cast< const mitk::Image * >(this->ProcessObject::GetInput(0));		
}

void mitk::LightBoxResultImageWriter::SetInput(const mitk::Image *image)
{
  this->ProcessObject::SetNthInput( 0, const_cast< mitk::Image * >( image ) );	
}

void mitk::LightBoxResultImageWriter::SetInputByNode(const mitk::DataTreeNode *node)
{
  if(node==NULL)
    return;

  SetInput(dynamic_cast<mitk::Image*>(node->GetData()));

  node->GetLevelWindow(m_LevelWindow, NULL, "levelWindow");
  node->GetLevelWindow(m_LevelWindow, NULL);
  char imageTypeName[200];
  if(node->GetStringProperty("type name", imageTypeName, NULL))
    SetImageTypeName(imageTypeName);
  char name[200];
  if (node->GetStringProperty("name", name, NULL))
    SetName(name);
}

const mitk::Image *mitk::LightBoxResultImageWriter::GetSourceImage(void)
{
  return static_cast< const mitk::Image * >(this->ProcessObject::GetInput(1));		
}

void mitk::LightBoxResultImageWriter::SetSourceImage(const mitk::Image *source)
{
  this->ProcessObject::SetNthInput( 1, const_cast< mitk::Image * >( source ) );	
}

bool mitk::LightBoxResultImageWriter::SetSourceByTreeSearch(mitk::DataTreeIteratorBase* iterator)
{
  if(iterator==NULL)
    return false;

  const mitk::Image* image = GetInput();

  if(image == NULL)
    return false;


  mitk::DataTreeIteratorClone it=new itk::RootTreeIterator<DataTreeBase>(iterator->GetTree(), iterator->GetNode());
  bool LoadedFromChili;
  while(!it->IsAtEnd())
  {
    //const char * name;
    //if(it->Get()->GetStringProperty("name", name, NULL))
    //{
    //  itkDebugMacro(<<"candidate: "<<name);
    //}
    //else
    //{
    //  itkDebugMacro(<<"candidate: no name");
    //}



    itkGenericOutputMacro(<<"it-> name"<<it->Get()->GetPropertyList()->GetProperty("name")->GetValueAsString());
    if(it->Get()->GetBoolProperty("LoadedFromChili", LoadedFromChili) && LoadedFromChili)
    {
      mitk::Image::Pointer sourcecandidate=dynamic_cast<mitk::Image*>(it->Get()->GetData());
      if(sourcecandidate.IsNotNull())
      {
        itkDebugMacro(<<"found sourcecandidate: ");
        if(image->GetDimension()<=sourcecandidate->GetDimension())
        {
          int i, dim=image->GetDimension();
          for(i=0; i<dim;++i)
            if(image->GetDimension(i)!=sourcecandidate->GetDimension(i))
              break;
          if(i==dim)
          {
            SetSourceImage(sourcecandidate);
            return true;
            itkDebugMacro(<<"dim incorrect: "<<i <<" "<<dim);
          }
        }
      }
    }
    ++it;
    itkDebugMacro(<<"xxxx");
  }
  itkDebugMacro(<<"yyyyyyyyyyyyyyyyyyyyy");
  return false;
}

void mitk::LightBoxResultImageWriter::SetLightBox(QcLightbox* lightbox)
{
  if(lightbox!=m_LightBox)
  {
    m_LightBox=lightbox;
    Modified();    
  }
}

void mitk::LightBoxResultImageWriter::SetLightBoxToCurrentLightBox()
{
#ifdef CHILIPLUGIN
  QcPlugin* plugin = mitk::ChiliPlugin::GetPluginInstance();
  if(plugin==NULL)
  {
    itkExceptionMacro(<<"GetPluginInstance()==NULL: Plugin is not initialized correctly !");
  }
  SetLightBox(plugin->lightboxManager()->getActiveLightbox());
#endif
}

bool mitk::LightBoxResultImageWriter::SetLightBoxToNewLightBox()
{
#ifdef CHILIPLUGIN
  QcPlugin* plugin = mitk::ChiliPlugin::GetPluginInstance();
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
  //QWidget* parentWidget = (QWidget*) plugin->parent();
  //QcLightbox * newLightbox = new QcLightbox(parentWidget,NULL,(uint)0) ; 	
  //SetLightBox(newLightbox);
#endif
  return false;
}

bool mitk::LightBoxResultImageWriter::SetLightBoxToCorrespondingLightBox()
{
#ifdef CHILIPLUGIN
  QcPlugin* plugin = mitk::ChiliPlugin::GetPluginInstance();
  if(plugin==NULL)
  {
    itkExceptionMacro(<<"GetPluginInstance()==NULL: Plugin is not initialized correctly !");
  }
  QcLightbox * activeLightbox = plugin->lightboxManager()->getActiveLightbox();
  ipPicDescriptor * currPic =	activeLightbox->fetchVolume();
  ipPicTSV_t* seriesDescriptionTag = ipPicQueryTag(currPic, "SERIES DESCRIPTION");
  if (seriesDescriptionTag)
  {
    if (*((char*)seriesDescriptionTag->value) == *(m_Name.c_str()))
    {
      activeLightbox->clear();
      SetLightBox(activeLightbox);
      return true;
    }
    else return false;
  }
  else return false;  
#endif
  return false;
}

QcLightbox* mitk::LightBoxResultImageWriter::GetLightBox() const
{
  return m_LightBox;
}


void mitk::LightBoxResultImageWriter::GenerateData()
{
#ifdef CHILIPLUGIN
  itkDebugMacro(<<"GenerateData ");
  const Image* image = GetInput();
  const Image* sourceimage = GetSourceImage();
  if((image==NULL) || (sourceimage==NULL) || (m_LightBox==NULL))
  {
    itk::ImageFileReaderException e(__FILE__, __LINE__);
    itk::OStringStream msg;
    if(image==NULL)
      msg << "image not set. ";
    if(sourceimage==NULL)
      msg << "source-image not set. ";
    if(m_LightBox==NULL)
      msg << "lightbox not set. ";
    e.SetDescription(msg.str().c_str());
    throw e;
    return;
  }

  ImageSliceSelector::Pointer resultslice = ImageSliceSelector::New();
  ImageSliceSelector::Pointer sourceslice = ImageSliceSelector::New();

  resultslice->SetInput(image);
  sourceslice->SetInput(sourceimage);

  mitk::SlicedGeometry3D* sourceSlicedGeometry;
  mitk::Geometry3D* geometry3DofSlice;
  Point3D origin;
  Vector3D v;

  sourceSlicedGeometry = sourceimage->GetSlicedGeometry();

  origin = sourceSlicedGeometry->GetCornerPoint();

  interSliceGeometry_t isg;
  memcpy(isg.forUID, mitk::FrameOfReferenceUIDManager::GetFrameOfReferenceUID(sourceSlicedGeometry->GetFrameOfReferenceID()), 128);
  isg.psu = ipPicUtilMillimeter;

  ipPicDescriptor *cur, *source, *prev=NULL;
  LevelWindow levelwindow;
  int s, t;
  int smax, tmax;
  smax = image->GetDimension(2);
  tmax = image->GetDimension(3);
  itkDebugMacro(<<"writing image: "<<m_ImageTypeName);
  itkDebugMacro(<<"lv: "<<m_LevelWindow.GetMin() <<" "<<m_LevelWindow.GetMax());
  for(s=smax-1;s>=0;--s)
  {
    resultslice->SetSliceNr(s);
    sourceslice->SetSliceNr(s);
    for(t=0;t<tmax;++t)
    {
      resultslice->SetTimeNr(t);
      sourceslice->SetTimeNr(t);
      resultslice->Update();
      sourceslice->Update();

      geometry3DofSlice = sourceSlicedGeometry->GetGeometry2D(s);

      v=geometry3DofSlice->GetCornerPoint().GetVectorFromOrigin();
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

      itkDebugMacro(<<"isg: o("<<isg.o[0]<<" "<<isg.o[1]<<" "<<isg.o[2]<<") u("<<isg.u[0]<<" "<<isg.u[1]<<" "<<isg.u[2]<<") v("<<isg.v[0]<<" "<<isg.v[1]<<" "<<isg.v[2]<<") w("<<isg.w[0]<<" "<<isg.w[1]<<" "<<isg.w[2]<<") "<<t);

      itkDebugMacro(<<"writing slice: "<<s <<" "<<t);

      cur=ipPicClone(resultslice->GetOutput()->GetPic());
      ipPicDelTag( cur, "SOURCE HEADER" );

      source=ipPicClone(sourceslice->GetOutput()->GetPic());

      if (m_Name.c_str())
      {
        ipPicTSV_t* nameTag = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
        nameTag->type = ipPicASCII;
        nameTag->bpe = 8;
        strcpy( nameTag->tag, "SERIES DESCRIPTION");
        nameTag->dim = 1;
        nameTag->value = malloc( strlen(m_Name.c_str()) );
        strncpy((char *)nameTag->value, m_Name.c_str(), strlen(m_Name.c_str()));
        nameTag->n[0] = strlen(m_Name.c_str());
        ipPicAddTag( source, nameTag );
        if (nameTag) {
          ipPicFreeTag (nameTag);
        }
      }

      QcPlugin::addTags( cur, source, prev==NULL );

      QcPlugin::addDicomHeader( cur, &isg );
      QcPlugin::addIcon( cur, true );

      QcPlugin::addLevelWindow( cur, m_LevelWindow.GetLevel(), m_LevelWindow.GetWindow());

      QcPlugin::changeImageType( cur, const_cast<char*>(m_ImageTypeName.c_str()) ); 

      cur->dim = 2;

      if( prev )
        QcPlugin::changeSeries( cur, prev );
      m_LightBox->addImage( cur );

      prev = cur;
    }
  }
  
#endif
}

void mitk::LightBoxResultImageWriter::Write() const
{
  const_cast<mitk::LightBoxResultImageWriter*>(this)->GenerateData();
}
