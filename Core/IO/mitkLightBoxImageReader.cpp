#include "mitkLightBoxImageReader.h"
#include <ipFunc.h>
#include <itkImageFileReader.h>

void mitk::LightBoxImageReader::SetLightBox(lightbox_t lightbox)
{
  if(lightbox!=m_LightBox)
    {
      m_LightBox=lightbox;
      Modified();
    }
}

lightbox_t mitk::LightBoxImageReader::GetLightBox()
{
  return m_LightBox;
}

void mitk::LightBoxImageReader::GenerateOutputInformation()
{
  printf("\nGenerateOutputInformation\n");
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

    for (position = 0; position < m_LightBox->images; ++position) 
	{
	 if (m_LightBox->image_list[position].type == DB_OBJECT_IMAGE) 
	 {
	   if(pic==NULL) // only build header
	     pic = pFetchImage (m_LightBox, position);
	   ++numberOfImages;
	    //            tagImageType = ipPicQueryTag (pic, tagIMAGE_TYPE);
	    //     break;
	    // }
	 }
	}

    printf("\n numberofimages %d\n",numberOfImages);

	if(numberOfImages==0)
	{
		itk::ImageFileReaderException e(__FILE__, __LINE__);
		itk::OStringStream msg;
		msg << "lightbox is empty";
		e.SetDescription(msg.str().c_str());
		throw e;
		return;
	}
       
	printf("\n copy header \n");
       ipPicDescriptor *header=ipPicCopyHeader(pic, NULL);
       printf("\n copy tags \n");
       ipFuncCopyTags(header, pic);

       //FIXME: was ist, wenn die Bilder nicht alle gleich gross sind?
       if(numberOfImages>1)
	 {  printf("\n numberofimages %d > 1\n",numberOfImages);
	   header->dim=3;
	   header->n[2]=numberOfImages;
	 }

       printf(" \ninitialisize output\n");
	output->Initialize(header);

        printf("\n modifie \n");
	m_ReadHeaderTime.Modified();
}

void mitk::LightBoxImageReader::GenerateData()
{
        printf("\n GenerateData \n");
	if(m_LightBox==NULL)
	{
		itk::ImageFileReaderException e(__FILE__, __LINE__);
		itk::OStringStream msg;
		msg << "lightbox not set";
		e.SetDescription(msg.str().c_str());
		throw e;
		return;
	}
        printf("\n request output \n");
       
	mitk::Image::Pointer output = this->GetOutput();

	int position, numberOfImages=0;
        ipPicDescriptor*  pic=NULL;

	int zDim=(output->GetDimension()>2?output->GetDimensions()[2]:1);
        printf("\n zdim is %u \n",zDim);

    for (position = 0; position < m_LightBox->images; ++position) 
	{
	 if (m_LightBox->image_list[position].type == DB_OBJECT_IMAGE) 
	 {
	   if(numberOfImages>zDim)
	     {
		itk::ImageFileReaderException e(__FILE__, __LINE__);
		itk::OStringStream msg;
		msg << "lightbox contains more images than calculated in the last GenerateOutputInformation call (>"<<zDim<<")";
		e.SetDescription(msg.str().c_str());
        printf("\n zu viele images \n");
		throw e;
		return;
	     }
	   pic = pFetchImage (m_LightBox, position);
        printf("\n add slice %u   x:%u y:%u\n", numberOfImages, pic->n[0],pic->n[1]);
	   output->SetPicSlice(pic, numberOfImages);
        printf("\n add slice %u  successful\n", numberOfImages, pic->n[0],pic->n[1]);	   ++numberOfImages;
	//	if(numberOfImages==24) break;
	 }
	}
        printf("\n fertig \n");
}

mitk::LightBoxImageReader::LightBoxImageReader() : m_LightBox(NULL)
{
}

mitk::LightBoxImageReader::~LightBoxImageReader()
{
}
