#include "mitkLightBoxImageReader.h"
#include <ipFunc.h>
#include <itkImageFileReader.h>


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

    mitk::Image::Pointer output = this->GetOutput();
    
    if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
        return;

    itkDebugMacro(<<"Reading lightbox for GenerateOutputInformation()");

    int position, numberOfImages=0;
    ipPicDescriptor*  pic=NULL;

    for (position = 0; position < m_LightBox->getFrames (); ++position) //ehemals position < m_LightBox->images
    {
        if (m_LightBox->fetchHeader(position) != NULL)  //ehemals: if (m_LightBox->image_list[position].type == DB_OBJECT_IMAGE) 
        {
            if(pic==NULL) // only build header
                pic = m_LightBox->fetchHeader(position);
            ++numberOfImages;
            //            tagImageType = ipPicQueryTag (pic, tagIMAGE_TYPE);
            //     break;
            // }
        }
    }

    itkGenericOutputMacro(<<"numberofimages " << numberOfImages);

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
        header->dim=3;
        header->n[2]=numberOfImages;
    }

    itkGenericOutputMacro(<<"initialisize output");
    output->Initialize(header);

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

    int position, numberOfImages=0;
    ipPicDescriptor*  pic=NULL;

    int zDim=(output->GetDimension()>2?output->GetDimensions()[2]:1);
    itkGenericOutputMacro(<<" zdim is "<<zDim);

    for (position = 0; position < m_LightBox->getFrames (); ++position) 
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
            itkGenericOutputMacro(<<"add slice  "<< numberOfImages <<" x:" <<pic->n[0]<<"y:"<<pic->n[1]);
            output->SetPicSlice(pic, zDim-1-numberOfImages);
            itkGenericOutputMacro(<<" add slice   successful "<< numberOfImages<<"  "<< pic->n[0]<<"  "<<pic->n[1]);
            ++numberOfImages;
            //	if(numberOfImages==24) break;
        }
    }
    itkGenericOutputMacro(<<"fertig ");
}
/*
Vector3<float> mitk::LightBoxImageReader::GetSpacingFromLB()
{

// OrganicerPluginFrame::getSpacingFromLB( )
//{

   Vector3<float> spacing(1.0, 1.0, 1.0);

   image_t* image;
 
   float slice[2];
   ipPicDescriptor* slices[2];  

   float posz;
  
    for(int p = 0,counter = 0;  p < m_LightBox->images,counter < 2; ++p,++counter){
        switch( lightbox->image_list[0].type){
      
        case DB_OBJECT_IMAGE:{          
	         slices[counter] = pFetchImage(m_LightBox,p);
       
                image = (image_t*)m_LightBox->image_list[p].db_struct;
	         posz = (float)image->sliceLocation;
	
                 interSliceGeometry_t*  isg_t  = fetchDicomGeometry(m_LightBox,p);
 
                   slice[counter] = (float)isg_t->o[2];
           break;
	}
        default:
	  break;
        }    	
    }
   float result = fabs(slice[0]-slice[1]);
    spacing.z = result;
   cout<<" spacing x: "<<spacing.x<<" y: "<<spacing.y<<" z: "<<spacing.z<<endl; 
   return spacing;
}
*/



mitk::LightBoxImageReader::LightBoxImageReader() : m_LightBox(NULL)
{
}

mitk::LightBoxImageReader::~LightBoxImageReader()
{
}
