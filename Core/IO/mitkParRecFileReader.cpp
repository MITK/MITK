#include "mitkParRecFileReader.h"
#include <itkImageFileReader.h>

void mitk::ParRecFileReader::GenerateOutputInformation()
{
  mitk::Image::Pointer output = this->GetOutput();
  
  if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
    return;
  
  itkDebugMacro(<<"Reading PAR file for GenerateOutputInformation()" << m_FileName);
  
  // Check to see if we can read the file given the name or prefix
  //
  if ( m_FileName == "" && m_FilePrefix == "" )
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
  }
  
  m_RecFileName = "";
  if( m_FileName != "")
  {
    int extPos=m_FileName.find_last_of(".");
    if(extPos>=std::npos)
    {
      const char *ext=m_FileName.str()+extPos+1;
      if(stricmp(ext,"par")==0)
        m_RecFileName = m_FileName.substr(0,extPos);
      else
        m_RecFileName = m_FileName;
    }
    else
        m_RecFileName = m_FileName;

    m_RecFileName.append(".rec");

    bool headerRead = false;

    mitk::PixelType type;
    unsigned int dimension=0;
    unsigned int dimensions[4]={0,0,1,1};
    mitk::Vector3D thickness(1.0,1.0,1.0);
    mitk::Vector3D gap(0.0,0.0,0.0);
    mitk::Vector3D spacing;

    FILE *f;
    f=fopen(m_FileName.c_str(), "r");
    if(f!=NULL)
    {
      while(!feof(f))
      {
        char s[300], *p;
        fgets(s,200,f);
        if(strstr(s,"Max. number of cardiac phases")
        {
          p=strchr(s,':');
          dimensions[3]=atoi(p);
          if(dimensions[3]>1)
            dimension=4;
        }
        else
        if(strstr(s,"Max. number of slices/locations")
        {
          p=strchr(s,':');
          dimensions[2]=atoi(p);
          if(dimensions[2]>1)
            dimension=3;
          else
            dimension=2;
        }
        else
        if(strstr(s,"Image pixel size")
        {
          p=strchr(s,':');
          int bpe=atoi(p);
          if(bpe==8)
            type=typeid(ipUInt1_t);
          else
            type=typeid(ipUInt2_t);
        }
        else
        if(strstr(s,"Recon resolution")
        {
          p=strcspn(s,"0123456789");
          sscanf(p,"%u %u", dimension[0], dimension[1]);
        }
        else
        if(strstr(s,"Slice thickness [mm]")
        {
          p=strcspn(s,"0123456789");
          sscanf(p,"%f %f %f", thickness.x, thickness.y, thickness.z);
        }
        else
        if(strstr(s,"Slice gap [mm]")
        {
          p=strcspn(s,"0123456789");
          sscanf(p,"%f %f %f", gap.x, gap.y, gap.z);
        }
      }
      fclose(f);

      spacing=thickness+gap;
      if((dimension>0) && (dimensions[0]>0) && (dimensions[1]>0))
        headerRead = true;
    }
    
    if( headerRead == false)
    {
      itk::ImageFileReaderException e(__FILE__, __LINE__);
      itk::OStringStream msg;
      msg << " Could not read file "
        << m_FileName.c_str();
      e.SetDescription(msg.str().c_str());
      throw e;
      return;
    }
    
    output->Initialize(type, dimension, dimensions);
    output->GetGeometry()->SetSpacing(spacing);
  }
  
  m_ReadHeaderTime.Modified();
}

void mitk::ParRecFileReader::GenerateData()
{
  mitk::Image::Pointer output = this->GetOutput();
  
  // Check to see if we can read the file given the name or prefix
  //
  if ( m_RecFileName == "" )
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "FileName for rec-file empty");
  }
  
  if( m_RecFileName != "")
  {
    char * c=const_cast<char *>(m_RecFileName.c_str());
    ipPicDescriptor* pic=ipPicGet(const_cast<char *>(m_FileName.c_str()), NULL);
    
    //left to right handed conversion
    if(pic->dim==3)
    {
      ipPicDescriptor* slice=ipPicCopyHeader(pic, NULL);
      slice->dim=2;
      int size=_ipPicSize(slice);
      slice->data=malloc(size);
      unsigned char *p_first=(unsigned char *)pic->data;
      unsigned char *p_last=(unsigned char *)pic->data;
      p_last+=size*(pic->n[2]-1);
      
      int i, smid=pic->n[2]/2;
      for(i=0; i<smid;++i,p_last-=size, p_first+=size)
      {
        memcpy(slice->data, p_last, size);
        memcpy(p_last, p_first, size);
        memcpy(p_first, slice->data, size);
      }
      ipPicFree(slice);
    }
    
    output->SetPicChannel(pic);
  }
}

mitk::ParRecFileReader::ParRecFileReader()
: m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::ParRecFileReader::~PicFileReader()
{
}
