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


#include "mitkLightBoxImageReader.h"
#include <ipFunc.h>
#include <itkImageFileReader.h>
#include "mitkPlaneGeometry.h"
#include "mitkFrameOfReferenceUIDManager.h"
#include <chili/isg.h>
#include <vtkTransform.h>
#include <ipDicom/ipDicom.h>
#include <list>
#include <vnl/vnl_cross.h>

void mitk::LightBoxImageReader::SetLightBox(QcLightbox* lightbox)
{
    if(lightbox!=m_LightBox)
    {
        m_LightBox=lightbox;
        Modified();    
    }
}

QcLightbox* mitk::LightBoxImageReader::GetLightBox() const
{
    return m_LightBox;
}

void mitk::LightBoxImageReader::GenerateOutputInformation()
{
    int RealPosition;
    SliceInfoArray sliceInfos;

    itkDebugMacro(<<"GenerateOutputInformation");

    //did someone tell us which lightbox we shall read?
    if(m_LightBox==NULL)
    {
        itk::ImageFileReaderException e(__FILE__, __LINE__);
        itk::OStringStream msg;
        msg << "lightbox not set";
        e.SetDescription(msg.str().c_str());
        itkWarningMacro(<<"lightbox not set");
        throw e;
        return;
    }

    mitk::Image::Pointer output = this->GetOutput();
    
    //did we already do it since the last change in the lightbox?
    if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
        return;    
    
    itkDebugMacro(<<"Reading lightbox for GenerateOutputInformation()");

    int position, numberOfImages=0,numberOfTimePoints=0,numberOfSlicesInCurrentTimePoint=0;
    ipPicDescriptor*  pic2=NULL;
    interSliceGeometry_t *isg;
    mitk::Point3D originCurrentSlice;
    mitk::Point3D originLastSlice;
    mitk::Point3D originFirstSlice;
    sliceInfo sliceInfo;

    //virtually sort the lightbox
    m_ImageNumbers.clear();
    m_ImageNumbers.reserve(m_LightBox->getFrames());
    SortImage(m_ImageNumbers);

    //read information of first slice
    RealPosition=GetRealPosition(0, m_ImageNumbers);
    if (m_LightBox->fetchHeader(RealPosition) != NULL)
    {
        isg=m_LightBox->fetchDicomGeometry(RealPosition);
        if(isg!=NULL)
        {
          mitk::vtk2itk(isg->o,originFirstSlice);
          originLastSlice = originFirstSlice;
        }
    }
        
    //count number of time points (numberOfTimePoints):
    // iterate through the virutally sorted lightbox:
    //  position= sorted position
    //  realPosition= original position in the (real) lightbox
    for (position = 0; position < m_LightBox->getFrames (); ++position)
    {
        //get original position of the image slice in the (real) lightbox
        RealPosition=GetRealPosition(position,m_ImageNumbers);

        //is it an image?
        if (m_LightBox->fetchHeader(RealPosition) != NULL)
        {         
            //get the isg
            isg=m_LightBox->fetchDicomGeometry(RealPosition);
            if(isg!=NULL)
            {
              itkDebugMacro(<<numberOfImages);
              mitk::vtk2itk(isg->o,originCurrentSlice);
              //timeconv1=ConvertTime(pic2);

              //check if we are still on the same slice
              if (originCurrentSlice == originLastSlice)
              {
                  /*if (numberOfTimePoints==1 && number==1)
                  {
                      tsv=ipPicQueryTag(pic2,"SOURCE HEADER");
                      dicomFindElement((unsigned char*) tsv->value, 0x0008, 0x0033, &data, &len);
                      sscanf( (char *) data, "%f", &imagetime );
                  }*/
                  //yes? so this is another time frame
                  ++numberOfTimePoints;
              }
              else
              {
                  sliceInfo.numberOfTimePoints=numberOfTimePoints;  
                  sliceInfo.startPosition=originLastSlice;
                  sliceInfos.push_back(sliceInfo);
                  numberOfTimePoints=1;
              }
            }

            originLastSlice=originCurrentSlice;
            //timeconv2=timeconv1;
            ++numberOfImages;                 
        }
        
    }

    if(numberOfImages==0)
    {
        itk::ImageFileReaderException e(__FILE__, __LINE__);
        itk::OStringStream msg;
        msg << "lightbox is empty";
        e.SetDescription(msg.str().c_str());
        throw e;
        return;
    }

    //add information for last slice
    sliceInfo.numberOfTimePoints=numberOfTimePoints;
    sliceInfo.startPosition=originCurrentSlice;
    sliceInfos.push_back(sliceInfo);

    //check whether all slices have the same number of time points
    bool differentNumberOfTimePoints = false;
    SliceInfoArray::iterator slit, slend =sliceInfos.end();
    for(slit = sliceInfos.begin(); slit!=slend;++slit)
    {
        if(slit->numberOfTimePoints != numberOfTimePoints)
        {
          itkWarningMacro(<<"problem: different number of time points in slices - taking minimum");
          if(slit->numberOfTimePoints < numberOfTimePoints)
            numberOfTimePoints = slit->numberOfTimePoints;
          differentNumberOfTimePoints = true;
        }
    }
    int numberOfSlices = sliceInfos.size();
    if(differentNumberOfTimePoints)
    {
      itkWarningMacro(<<"minimum of numberOfTimePoints is" << numberOfTimePoints);
    }
    else
    {
      itkDebugMacro(<<"numberOfTimePoints" << numberOfTimePoints);
      if(numberOfSlices != numberOfImages/numberOfTimePoints)
      {
        itkWarningMacro(<<"numberOfSlices!=numberOfImages/numberOfTimePoints: " << numberOfSlices << " != " << numberOfImages/numberOfTimePoints);
      }
    }


    //build pic-header to initialize output information
    itkDebugMacro(<<"copy header");
    RealPosition=GetRealPosition(0, m_ImageNumbers);
    ipPicDescriptor *header=ipPicCopyHeader(m_LightBox->fetchHeader(RealPosition), NULL);
    //@FIXME: was ist, wenn die Bilder nicht alle gleich gross sind?
    if(numberOfImages>1)
    {  
        itkDebugMacro(<<"numberofimages > 1 :" << numberOfImages);
        if (numberOfTimePoints>1)
        {
            header->dim=4;
            header->n[2]=numberOfSlices;
            header->n[3]=numberOfTimePoints;
        }
        else
        {
            header->dim=3;
            header->n[2]=numberOfSlices;
            itkDebugMacro(<<"dim=3:" );
        }
       
    }
    itkDebugMacro(<<"initialize output");
    output->Initialize(header);

    //build geometry
    interSliceGeometry_t *interSliceGeometry;
    interSliceGeometry=m_LightBox->fetchDicomGeometry(RealPosition);
    if(interSliceGeometry!=NULL)
    {
      mitk::Vector3D rightVector;
      mitk::Vector3D downVector;
      mitk::Vector3D spacing;

      mitk::vtk2itk(interSliceGeometry->u, rightVector);
      mitk::vtk2itk(interSliceGeometry->v, downVector);
      mitk::vtk2itk(interSliceGeometry->ps, spacing);
      itkDebugMacro(<<"spacing: "<<spacing);

      rightVector=rightVector*output->GetDimension(0);
      downVector=downVector*output->GetDimension(1);

      mitk::PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
      spacing = GetSpacingFromLB(m_ImageNumbers);
      itkDebugMacro(<<"get spacing: "<<spacing);
      planegeometry->InitializeStandardPlane( rightVector,downVector,&spacing);
      //planegeometry->InitializeStandardPlane( rightVector,downVector,&GetSpacingFromLB());

      mitk::Point3D origin;
      mitk::vtk2itk(interSliceGeometry->o, origin);
      itkDebugMacro(<<"origin: "<<origin);
      planegeometry->SetOrigin(origin);
      planegeometry->SetFrameOfReferenceID(FrameOfReferenceUIDManager::AddFrameOfReferenceUID(interSliceGeometry->forUID));

      SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
      itkDebugMacro(<<"output->GetDimension(2): "<<output->GetDimension(2));
      slicedGeometry->InitializeEvenlySpaced(planegeometry, output->GetDimension(2));
          
      
      if (numberOfTimePoints>1)
      {
          mitk::ScalarType timeBounds[] = {0.0, 1.0};
          slicedGeometry->SetTimeBoundsInMS( timeBounds );
      }

      TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
      itkDebugMacro(<<"output->GetDimension(3): "<<output->GetDimension(3));
      timeSliceGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
      timeSliceGeometry->TransferItkToVtkTransform();
      
      output->SetGeometry(timeSliceGeometry);  

      itkDebugMacro("origin of top slice: "<<0<<" lb pos: "<<position<< "origin:"<<output->GetGeometry()->GetCornerPoint(0));
    }
    else
    {
      itkWarningMacro(<<"interSliceGeometry is NULL");
      itkDebugMacro(<<"spacing from pic: "<<output->GetSlicedGeometry()->GetSpacing());
    }

    itkDebugMacro(<<" modified ");    
    m_ReadHeaderTime.Modified();
}

void mitk::LightBoxImageReader::GenerateData()
{
    itkDebugMacro(<<"GenerateData ");
    if(m_LightBox==NULL)
    {
        itk::ImageFileReaderException e(__FILE__, __LINE__);
        itk::OStringStream msg;
        msg << "lightbox not set";
        e.SetDescription(msg.str().c_str());
        throw e;
        return;
    }
    itkDebugMacro(<<"request output ");

    mitk::Image::Pointer output = this->GetOutput();

    int position, numberOfImages=0,time=0,time1=0,time2=0;
    ipPicDescriptor*  pic0=NULL;
    ipPicDescriptor*  pic=NULL;
    interSliceGeometry_t* isg0;
    interSliceGeometry_t* isg;
    mitk::Point3D origin1;
    mitk::Point3D origin0;
    mitk::Point3D origin;

    //sort image
    int RealPosition;
        
    int zDim=(output->GetDimension()>2?output->GetDimensions()[2]:1);
    itkDebugMacro(<<" zdim is "<<zDim);
    RealPosition=GetRealPosition(0, m_ImageNumbers);
    pic0 = m_LightBox->fetchPic (RealPosition);// pFetchImage (m_LightBox, position);
    isg0 = m_LightBox->fetchDicomGeometry(RealPosition);
    if(isg0!=NULL)
    {
      mitk::vtk2itk(isg0->o,origin0);
      origin=origin0;
      itkDebugMacro(<<"origin    "<<origin);
    }
    else
    {
      itkWarningMacro(<<"interSliceGeometry is NULL");
    }
        
    output->SetPicSlice(pic0, numberOfImages,time);
    for (position = 1; position < m_LightBox->getFrames (); ++position) 
    {
        //GetRealPosition of image
        RealPosition=GetRealPosition(position, m_ImageNumbers);

        if (m_LightBox->fetchHeader(RealPosition) != NULL)//ehemals (m_LightBox->image_list[position].type == DB_OBJECT_IMAGE) 
        {
            if(numberOfImages>zDim)
            {
                itk::ImageFileReaderException e(__FILE__, __LINE__);
                itk::OStringStream msg;
                msg << "lightbox contains more images than calculated in the last GenerateOutputInformation call (>"<<zDim<<")";
                e.SetDescription(msg.str().c_str());
                itkDebugMacro(<<"zu viele images");
                throw e;
                return;
            }
            
            pic = m_LightBox->fetchPic (RealPosition);// pFetchImage (m_LightBox, position);
            isg = m_LightBox->fetchDicomGeometry(RealPosition);
            if(isg!=NULL)
            {
              mitk::vtk2itk(isg->o,origin1);
              if (origin1 != origin0 && origin1!=origin)
              {
//                  itkDebugMacro("origin1: "<<origin1<<" origin0: "<<origin0);
                  ++numberOfImages;
                  time=time1;
                  origin0=origin1;                
              }
              else 
              {
                  ++time;
                  if (origin1==origin && origin0 != origin)
                  {
                    ++time2;
                    time1=time2;
                    time=time2;
                    numberOfImages=0;
                  }

              }
            }
            else
              ++numberOfImages;
            output->SetPicSlice(pic, numberOfImages,time);
itkDebugMacro("setting slice: "<<numberOfImages<<" lb pos: "<<RealPosition<< "origin:"<<origin1);
                //itkDebugMacro(<<"add slice  "<< numberOfImages <<" x:" <<pic->n[0]<<"y:"<<pic->n[1]);
                //output->SetPicSlice(pic, zDim-1-numberOfImages,time);
                //itkDebugMacro(<<" add slice   successful "<< numberOfImages<<"  "<< pic->n[0]<<"  "<<pic->n[1]);
                //++numberOfImages;                    
        }
    }
    itkDebugMacro(<<"fertig ");
}

mitk::Vector3D mitk::LightBoxImageReader::GetSpacingFromLB(LocalImageInfoArray& imageNumbers)
{
    mitk::Vector3D spacing=(1.0 ,1.0 , 1.0);     

    interSliceGeometry_t*  isg_t  = m_LightBox->fetchDicomGeometry(0);
    if(isg_t!=NULL)
    {
      mitk::vtk2itk(isg_t->ps, spacing);
    }

    if(imageNumbers.size()==0) return spacing;

    LocalImageInfoArray::iterator it = imageNumbers.begin(), infoEnd=imageNumbers.end();
    
    mitk::Vector3D& origin0 = it->origin;

    while(it!=infoEnd)
    {
      if(mitk::Equal(it->origin, origin0)==false)
        break;
      ++it;
    }
    if(it!=infoEnd)
    {
      Vector3D toNext = it->origin-origin0;
      spacing[2]=toNext.GetNorm();
    }

    return spacing;
}
bool mitk::LightBoxImageReader::ImageOriginLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 )
{
  if(mitk::Equal(elem1.origin, elem2.origin))
  {
    if(elem1.imageNumber == elem2.imageNumber)
      return elem1.pos < elem2.pos;
    return elem1.imageNumber < elem2.imageNumber;
  }
  return elem1.origin**elem1.direction < elem2.origin**elem2.direction;
};

bool mitk::LightBoxImageReader::ImageNumberLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 )
{
  return elem1.imageNumber < elem2.imageNumber;
};

void mitk::LightBoxImageReader::SortImage(LocalImageInfoArray& imageNumbers)
{
    ipPicDescriptor*  pic=NULL;
    ipPicTSV_t *tsv;
    void* data;
    ipUInt4_t len;
    int imageNumber;
    LocalImageInfo info;
    Vector3D direction;
    info.direction = &direction;
    bool directionInitialized=false;

    for (int position = 0; position < m_LightBox->getFrames (); ++position)
    {
      info.pos=position;

      pic = m_LightBox->fetchHeader(position);
      tsv=ipPicQueryTag(pic,"SOURCE HEADER");
      if (tsv)
      {
        dicomFindElement((unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len);
        sscanf( (char *) data, "%d", &imageNumber );
        info.imageNumber=imageNumber;
        //itkDebugMacro(<<"number image: "<<imageNumber);
      }
      else
      {
        info.imageNumber=position;
        itkWarningMacro(<<"DICOM header not available (pic tag SOURCE HEADER is NULL) at lighbox-position "<<position);
      }
      interSliceGeometry_t *isg;
      isg = m_LightBox->fetchDicomGeometry(position);
      if(isg!=NULL)
      {
        mitk::vtk2itk(isg->o, info.origin);
        if(directionInitialized==false)
        {
          VnlVector u(3), v(3);
          mitk::vtk2vnl(isg->u, u);
          mitk::vtk2vnl(isg->v, v);
          VnlVector normal = vnl_cross_3d(u, v);
          normal.normalize();
          direction.Set_vnl_vector(normal);
itkDebugMacro(<<"DIRECTION: "<<direction);
          directionInitialized=true;
        }
      }
      else
      {
        FillVector3D(info.origin, 0, 0, -info.imageNumber);
        itkWarningMacro(<<"interSliceGeometry not available at lighbox-position "<<position);
      }

      imageNumbers.push_back(info);
    }

    LocalImageInfoArray::iterator it = imageNumbers.begin(), infoEnd=imageNumbers.end();

    int maxNumber = std::max_element(it, infoEnd, ImageNumberLesser)->imageNumber;
    int minNumber = std::min_element(it, infoEnd, ImageNumberLesser)->imageNumber;
    int layers=imageNumbers.size()/maxNumber;//number of the layer

    int afterFirstLayer=imageNumbers.size()-(maxNumber-minNumber+1);
    if(afterFirstLayer>0)
    {
      it+=afterFirstLayer;
      imageNumbers.erase(it, infoEnd);
    }

    std::sort(imageNumbers.begin(), imageNumbers.end(), ImageOriginLesser);
}

int mitk::LightBoxImageReader::GetRealPosition(int position, LocalImageInfoArray& list)
{
  return liste[position].pos;
}


mitk::LightBoxImageReader::LightBoxImageReader() : m_LightBox(NULL)
{
}

mitk::LightBoxImageReader::~LightBoxImageReader()
{
}
