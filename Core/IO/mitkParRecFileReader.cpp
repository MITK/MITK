#include "mitkParRecFileReader.h"
#include <itkImageFileReader.h>

#ifdef __GNUC__
#define stricmp strcasecmp 
#endif

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
    if(extPos>=-1)
    {
      const char *ext=m_FileName.c_str()+extPos+1;
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
    float sliceThickness=0.0;
    float sliceGap=0.0;
    float sliceSpacing=0.0;
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
        if(strstr(s,"Max. number of cardiac phases"))
        {
          p=strchr(s,':')+1;
          dimensions[3]=atoi(p);
          if(dimensions[3]>1)
            dimension=4;
        }
        else
        if(strstr(s,"Max. number of slices/locations"))
        {
          p=strchr(s,':')+1;
          dimensions[2]=atoi(p);
          if(dimension==0)
          {
            if(dimensions[2]>1)
              dimension=3;
            else
              dimension=2;
          }
        }
        else
        if(strstr(s,"Image pixel size"))
        {
          p=strchr(s,':')+1;
          int bpe=atoi(p);
          if(bpe==8)
            type=typeid(ipUInt1_t);
          else
            type=typeid(ipUInt2_t);
        }
        else
        if(strstr(s,"Recon resolution"))
        {
          p=s+strcspn(s,"0123456789");
          sscanf(p,"%u %u", dimensions, dimensions+1);
        }
        else
        if(strstr(s,"FOV (ap,fh,rl) [mm]"))
        {
          p=s+strcspn(s,"0123456789");
          sscanf(p,"%f %f %f", &thickness.x, &thickness.y, &thickness.z);
        }
        else
        if(strstr(s,"Slice thickness [mm]"))
        {
          p=s+strcspn(s,"0123456789");
          sscanf(p,"%f", &sliceThickness);
        }
        else
        if(strstr(s,"Slice gap [mm]"))
        {
          p=s+strcspn(s,"-0123456789");
          sscanf(p,"%f", &sliceGap);
        }
      }
      fclose(f);

//C:\home\ivo\data\coronaries\ucsf-wholeheart-2.par
      sliceSpacing = sliceThickness+sliceGap;
      if(fabs(thickness.x/dimensions[2]-sliceSpacing)<0.0001)
        thickness.x=thickness.y;
      else
      if(fabs(thickness.y/dimensions[2]-sliceSpacing)<0.0001)
        thickness.y=thickness.x;
      thickness.z=sliceSpacing;

      thickness.x/=dimensions[0];
      thickness.y/=dimensions[1];
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
    output->GetSlicedGeometry()->SetSpacing(spacing);
    output->GetSlicedGeometry()->SetGeometry2D((ipPicDescriptor*)1, 0, 0);
    output->GetSlicedGeometry()->SetEvenlySpaced();
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
    FILE *f = fopen(m_RecFileName.c_str(), "r");
    if(f==NULL)
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Could not open rec-file.");
    }

    int z, zmax;
    int t, tmax;
    
    z=output->GetRequestedRegion().GetIndex(2);
    t=output->GetRequestedRegion().GetIndex(3);
    
    zmax=z+output->GetRequestedRegion().GetSize(2);
    tmax=t+output->GetRequestedRegion().GetSize(3);
    
    int sliceSize=output->GetDimension(0)*output->GetDimension(1)*output->GetPixelType().GetBpe()/8;
    void *data = malloc(sliceSize);

    bool ignore4Dtopogram=false;
    {
      int slicePlusTimeSize=output->GetDimension(0)*output->GetDimension(1)*output->GetDimension(3)*output->GetPixelType().GetBpe()/8;
    if(output->GetDimension(3)>1)
      ignore4Dtopogram=true;

      for(;t<tmax;++t)
        for(;z<zmax;++z)
        {
			    if(ignore4Dtopogram)
            fseek(f,slicePlusTimeSize*z+(sliceSize+1)*t,SEEK_SET);				  
          else
            fseek(f,slicePlusTimeSize*z+sliceSize*t,SEEK_SET);				  
      	  fread(data, sliceSize, 1, f);
          output->SetSlice(data,z,t,0);
        }
    }
    //else
    //{
    //  for(;z<zmax;++z)
    //  {
    //    fseek(f,sliceSize*z,SEEK_SET);				  
    //  	fread(data, sliceSize, 1, f);
    //    output->SetSlice(data,z,0,0);
    //  }
    //}
    free(data);
  }
}

mitk::ParRecFileReader::ParRecFileReader()
: m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::ParRecFileReader::~ParRecFileReader()
{
}
