#include "mitkLightBoxImageReader.h"
#include <ipFunc.h>
#include <itkImageFileReader.h>
#include "mitkPlaneGeometry.h"
#include <isg.h>
#include <vtkTransform.h>
#include <ipDicom.h>


void mitk::LightBoxImageReader::SetLightBox(QcLightbox* lightbox)
{
    if(lightbox!=m_LightBox)
    {
        m_LightBox=lightbox;
        Modified();    
    }
}

QcLightbox* mitk::LightBoxImageReader::GetLightBox()
{
    return m_LightBox;
}

void mitk::LightBoxImageReader::GenerateOutputInformation()
{
    itkGenericOutputMacro(<<"GenerateOutputInformation");
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
//DEBUG itkGenericOutputMacro(<<"1");

    mitk::Image::Pointer output = this->GetOutput();
//DEBUG itkGenericOutputMacro(<<"2");
    
    if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
        return;
//DEBUG itkGenericOutputMacro(<<"3");

    itkDebugMacro(<<"Reading lightbox for GenerateOutputInformation()");

    int position, numberOfImages=0,numberOfTimePoints=0;
    ipPicDescriptor*  pic=NULL;
    //ipPicDescriptor*  pic2=NULL;
    interSliceGeometry_t *interSliceGeometry;
    interSliceGeometry_t *isg;
    mitk::Point3D origin1;
    mitk::Point3D origin0;
    //double timeconv0=0,timeconv1=0,timeconv2=0;
    
//DEBUG itkGenericOutputMacro(<<"4");
    for (position = 0; position < m_LightBox->getFrames (); ++position) //ehemals position < m_LightBox->images
    {
        if (m_LightBox->fetchHeader(position) != NULL)  //ehemals: if (m_LightBox->image_list[position].type == DB_OBJECT_IMAGE) 
        {
            if(pic==NULL) // only build header
            {
//DEBUG itkGenericOutputMacro(<<"5:"<<position);
                pic = m_LightBox->fetchHeader(position);
                interSliceGeometry=m_LightBox->fetchDicomGeometry(position);
//DEBUG itkGenericOutputMacro(<<"5a:"<<numberOfImages);
                if(interSliceGeometry!=NULL)
                {
                  mitk::vtk2itk(interSliceGeometry->o,origin0);
//DEBUG itkGenericOutputMacro(<<"5b:"<<numberOfImages);
                }
                //timeconv0=ConvertTime(pic);
            }
//DEBUG itkGenericOutputMacro(<<"6:"<<numberOfImages);

            //image time
            //pic2 = m_LightBox->fetchHeader(position);
            isg=m_LightBox->fetchDicomGeometry(position);
            if(isg!=NULL)
            {
//DEBUG itkGenericOutputMacro(<<"7:"<<numberOfImages);
              mitk::vtk2itk(isg->o,origin1);
              //timeconv1=ConvertTime(pic2);

              if (origin1 == origin0)
                  ++numberOfTimePoints;
              else
                  numberOfTimePoints=1;
            }

            origin0=origin1;
            //timeconv2=timeconv1;

            ++numberOfImages;
            //            tagImageType = ipPicQueryTag (pic, tagIMAGE_TYPE);
            //     break;
            
        }
    }
    
    //itkGenericOutputMacro(<<"numberofimages " << numberOfImages);
    itkGenericOutputMacro(<<"numberOfTimePoints" << numberOfTimePoints);
    
    if(numberOfImages==0)
    {
        itk::ImageFileReaderException e(__FILE__, __LINE__);
        itk::OStringStream msg;
        msg << "lightbox is empty";
        e.SetDescription(msg.str().c_str());
        throw e;
        return;
    }

    itkGenericOutputMacro(<<"copy header");
    ipPicDescriptor *header=ipPicCopyHeader(pic, NULL);
    itkGenericOutputMacro(<<"copy tags ");
    ipFuncCopyTags(header, pic);

    //@FIXME: was ist, wenn die Bilder nicht alle gleich gross sind?
    if(numberOfImages>1)
    {  
        itkGenericOutputMacro(<<"numberofimages > 1 :" << numberOfImages);
        if (numberOfTimePoints>1)
        {
            header->dim=4;
            header->n[2]=numberOfImages/numberOfTimePoints;
            header->n[3]=numberOfTimePoints;
        }
        else
        {
            header->dim=3;
            header->n[2]=numberOfImages;
            itkGenericOutputMacro(<<"dim=3:" );
        }
       
    }

    itkGenericOutputMacro(<<"initialisize output");
    output->Initialize(header);

    if(interSliceGeometry!=NULL)
    {
      mitk::Vector3D rightVector;
      mitk::Vector3D downVector;
      mitk::Vector3D spacing;

      mitk::vtk2itk(interSliceGeometry->u, rightVector);
      mitk::vtk2itk(interSliceGeometry->v, downVector);
      mitk::vtk2itk(interSliceGeometry->ps, spacing);
      itkGenericOutputMacro(<<"spacing: "<<spacing);

      rightVector=rightVector*output->GetDimension(0);
      downVector=downVector*output->GetDimension(1);

      mitk::PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
      itkGenericOutputMacro(<<"get spacing: "<<GetSpacingFromLB());
      planegeometry->InitializeStandardPlane( rightVector,downVector,&spacing);
      //planegeometry->InitializeStandardPlane( rightVector,downVector,&GetSpacingFromLB());

      mitk::Point3D origin;
      mitk::vtk2itk(interSliceGeometry->o, origin);
      itkGenericOutputMacro(<<"origin: "<<origin);
      planegeometry->SetOrigin(origin);

      SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
      itkGenericOutputMacro(<<"output->GetDimension(2): "<<output->GetDimension(2));
      slicedGeometry->InitializeEvenlySpaced(planegeometry, output->GetDimension(2));
          
      
      if (numberOfTimePoints>1)
      {
          mitk::ScalarType timeBounds[] = {0.0, 1.0};
          slicedGeometry->SetTimeBoundsInMS( timeBounds );
      }

      TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
      itkGenericOutputMacro(<<"output->GetDimension(3): "<<output->GetDimension(3));
      timeSliceGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
      timeSliceGeometry->TransferItkToVtkTransform();
      
      output->SetGeometry(timeSliceGeometry);  
    }
    else
    {
      itkWarningMacro(<<"interSliceGeometry is NULL");
      itkGenericOutputMacro(<<"spacing from pic: "<<output->GetSlicedGeometry()->GetSpacing());
    }

    itkGenericOutputMacro(<<" modifie ");
    m_ReadHeaderTime.Modified();
}

void mitk::LightBoxImageReader::GenerateData()
{
    itkGenericOutputMacro(<<"GenerateData ");
    if(m_LightBox==NULL)
    {
        itk::ImageFileReaderException e(__FILE__, __LINE__);
        itk::OStringStream msg;
        msg << "lightbox not set";
        e.SetDescription(msg.str().c_str());
        throw e;
        return;
    }
    itkGenericOutputMacro(<<"request output ");

    mitk::Image::Pointer output = this->GetOutput();

    int position, numberOfImages=0,time=0;;
    ipPicDescriptor*  pic0=NULL;
    ipPicDescriptor*  pic=NULL;
    interSliceGeometry_t* isg0;
    interSliceGeometry_t* isg;
    mitk::Point3D origin1;
    mitk::Point3D origin0;
    
    int zDim=(output->GetDimension()>2?output->GetDimensions()[2]:1);
    itkGenericOutputMacro(<<" zdim is "<<zDim);
    pic0 = m_LightBox->fetchPic (0);// pFetchImage (m_LightBox, position);
    isg0 = m_LightBox->fetchDicomGeometry(0);
    if(isg0!=NULL)
    {
      mitk::vtk2itk(isg0->o,origin0);
    }
    else
    {
      itkWarningMacro(<<"interSliceGeometry is NULL");
    }
    output->SetPicSlice(pic0, zDim-1-numberOfImages,time);
    
    for (position = 1; position < m_LightBox->getFrames (); ++position) 
    {
        if (m_LightBox->fetchHeader(position) != NULL)//ehemals (m_LightBox->image_list[position].type == DB_OBJECT_IMAGE) 
        {
            if(numberOfImages>zDim)
            {
                itk::ImageFileReaderException e(__FILE__, __LINE__);
                itk::OStringStream msg;
                msg << "lightbox contains more images than calculated in the last GenerateOutputInformation call (>"<<zDim<<")";
                e.SetDescription(msg.str().c_str());
                itkGenericOutputMacro(<<"zu viele images");
                throw e;
                return;
            }
            
            pic = m_LightBox->fetchPic (position);// pFetchImage (m_LightBox, position);
            isg = m_LightBox->fetchDicomGeometry(position);
            if(isg!=NULL)
            {
              mitk::vtk2itk(isg->o,origin1);
              if (origin1 != origin0)
              {
                  //itkGenericOutputMacro("origin1: "<<origin1<<" origin0: "<<origin0);
                  ++numberOfImages;
                  time=0;
                  origin0=origin1;                
              }
              else 
              {
                  ++time;
              }
            }
            else
              ++numberOfImages;
            output->SetPicSlice(pic, zDim-1-numberOfImages,time);
                       
                //itkGenericOutputMacro(<<"add slice  "<< numberOfImages <<" x:" <<pic->n[0]<<"y:"<<pic->n[1]);
                //output->SetPicSlice(pic, zDim-1-numberOfImages,time);
                //itkGenericOutputMacro(<<" add slice   successful "<< numberOfImages<<"  "<< pic->n[0]<<"  "<<pic->n[1]);
                //++numberOfImages;                    
        }
    }
    itkGenericOutputMacro(<<"fertig ");
}

mitk::Vector3D mitk::LightBoxImageReader::GetSpacingFromLB()
{
    mitk::Vector3D spacing=(1.0 ,1.0 , 1.0);     
    float slicex[2]={0.0,1.0};
    float slicey[2]={0.0,1.0};
    float slicez[2]={0.0,1.0};

    for(int p = 0,counter = 0;  p < m_LightBox->getFrames(),counter < 2; ++p,++counter){
        if (m_LightBox->fetchHeader(0) != NULL)
        {          
	         interSliceGeometry_t*  isg_t  = m_LightBox->fetchDicomGeometry(p);
           if(isg_t!=NULL)
           {
              mitk::vtk2itk(isg_t->ps, spacing);
              slicex[counter] = (float)isg_t->o[0];
              slicey[counter] = (float)isg_t->o[1];
              slicez[counter] = (float)isg_t->o[2];
           //itkGenericOutputMacro(<<"slicez[0]: "<<slicez[0]<<"  slicez[1]:"<<slicez[1]<<"isg_t->o "<<(float)isg_t->o[0]<<" "<<(float)isg_t->o[1]<<"  "<<(float)isg_t->o[2]);
           }
        }    	
    }
    
    float result = fabs(slicex[0]-slicex[1])+fabs(slicey[0]-slicey[1])+fabs(slicez[0]-slicez[1]);
    //float result = fabs(slicez[0]-slicez[1]);
    
    spacing[2] = result; 
    return spacing;
}

//double mitk::LightBoxImageReader::ConvertTime(ipPicDescriptor*  pic)
//{
//    ipPicTSV_t *tsv;
//    void* data;
//    ipUInt4_t len;
//    int i;   
//    char imagetime[13];
//    int time[13];
//    char zero = '0';
//    double timeconv='0';
//
//    tsv=ipPicQueryTag(pic,"SOURCE HEADER");
//    dicomFindElement((unsigned char*) tsv->value, 0x0008, 0x0033, &data, &len);
//    sscanf( (char *) data, "%s", &imagetime );
//
//    for(i=0;i<10;++i)
//    {
//      time[i] = (int)imagetime[i] - (int)zero; 
//      //itkGenericOutputMacro(<<"time: "<<time[i]);
//    }
//
//    timeconv=(time[0]*10+time[1])*60;
//    timeconv=(timeconv+time[2]*10+time[3])*60;
//    timeconv=(timeconv+time[4]*10+time[5]);
//    timeconv=timeconv+time[7]*0.1+time[8]*0.01+time[9]*0.001;
//    return timeconv;
//}

mitk::LightBoxImageReader::LightBoxImageReader() : m_LightBox(NULL)
{
}

mitk::LightBoxImageReader::~LightBoxImageReader()
{
}
