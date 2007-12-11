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


#include "mitkPicFileReader.h"
#include <itkImageFileReader.h>

extern "C" 
{
ipPicDescriptor * MITKipPicGet( char *infile_name, ipPicDescriptor *pic );
ipPicDescriptor * MITKipPicGetTags( char *infile_name, ipPicDescriptor *pic );
}

//##ModelId=3E1878400265
void mitk::PicFileReader::GenerateOutputInformation()
{
    mitk::Image::Pointer output = this->GetOutput();

    if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
        return;

    itkDebugMacro(<<"Reading file for GenerateOutputInformation()" << m_FileName);

    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FileName == "" && m_FilePrefix == "" )
    {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
    }

    if( m_FileName != "")
    {
        ipPicDescriptor* header=ipPicGetHeader(const_cast<char *>(m_FileName.c_str()), NULL);

        header=MITKipPicGetTags(const_cast<char *>(m_FileName.c_str()), header);

        int channels = 1;

        ipPicTSV_t *tsv;
        if ( (tsv = ipPicQueryTag( header, "SOURCE HEADER" )) != NULL)
        {
          if(tsv->n[0]>1e+06)
          {
            ipPicTSV_t *tsvSH;
            tsvSH = ipPicDelTag( header, "SOURCE HEADER" );
            ipPicFreeTag(tsvSH);
          }
        }
        if ( (tsv = ipPicQueryTag( header, "ICON80x80" )) != NULL)
        {
          ipPicTSV_t *tsvSH;
          tsvSH = ipPicDelTag( header, "ICON80x80" );
          ipPicFreeTag(tsvSH);
        }
        if ( (tsv = ipPicQueryTag( header, "VELOCITY" )) != NULL)
        {
          ++channels;
          ipPicDelTag( header, "VELOCITY" );
        }

        if( header == NULL)
        {
            itk::ImageFileReaderException e(__FILE__, __LINE__);
            itk::OStringStream msg;
            msg << " Could not read file "
                << m_FileName.c_str();
            e.SetDescription(msg.str().c_str());
            throw e;
            return;
        }

        output->Initialize(header, channels);
        ipPicFree ( header );
    }
    else
    {
        int numberOfImages=0;
        m_StartFileIndex=0;

        ipPicDescriptor* header=NULL;

        char fullName[1024];

        while(m_StartFileIndex<10)
        {
            sprintf(fullName, m_FilePattern.c_str(), m_FilePrefix.c_str(), m_StartFileIndex+numberOfImages);
            FILE * f=fopen(fullName,"r");
            if(f==NULL) 
            {
                //already found an image?
                if(numberOfImages>0)
                    break;
                //no? let's increase start
                ++m_StartFileIndex;
            }
            else
            {
                fclose(f);
                //only open the header of the first file found,
                //@warning what to do when images do not have the same size??
                if(header==NULL) 
                {
                    header=ipPicGetHeader(fullName, NULL);
                    header=MITKipPicGetTags(fullName, header);
                }
                ++numberOfImages;
            }
        }

        printf("\n numberofimages %d\n",numberOfImages);

        if(numberOfImages==0)
        {
            itk::ImageFileReaderException e(__FILE__, __LINE__);
            itk::OStringStream msg;
            msg << "no images found";
            e.SetDescription(msg.str().c_str());
            throw e;
            return;
        }

        //@FIXME: was ist, wenn die Bilder nicht alle gleich gross sind?
        if(numberOfImages>1)
        {  
            printf("\n numberofimages %d > 1\n",numberOfImages);
            header->dim=3;
            header->n[2]=numberOfImages;
        }

        printf(" \ninitialisize output\n");
        output->Initialize(header);
        ipPicFree ( header );
    }

    m_ReadHeaderTime.Modified();
}

void mitk::PicFileReader::ConvertHandedness(ipPicDescriptor* pic)
{
  //left to right handed conversion
  if(pic->dim>=3)
  {
      ipPicDescriptor* slice=ipPicCopyHeader(pic, NULL);
      slice->dim=2;
      size_t size=_ipPicSize(slice);
      slice->data=malloc(size);

      size_t v, volumes = (pic->dim>3? pic->n[3] : 1);
      size_t volume_size = size*pic->n[2];

      for(v=0; v<volumes; ++v)
      {
        unsigned char *p_first=(unsigned char *)pic->data;
        unsigned char *p_last=(unsigned char *)pic->data;
        p_first+=v*volume_size;
        p_last+=size*(pic->n[2]-1)+v*volume_size;

        size_t i, smid=pic->n[2]/2;
        for(i=0; i<smid;++i,p_last-=size, p_first+=size)
        {
            memcpy(slice->data, p_last, size);
            memcpy(p_last, p_first, size);
            memcpy(p_first, slice->data, size);
        }
      }
      ipPicFree(slice);
  }
}

//##ModelId=3E187255016C
void mitk::PicFileReader::GenerateData()
{
    mitk::Image::Pointer output = this->GetOutput();

    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FileName == "" && m_FilePrefix == "" )
    {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
    }

    if( m_FileName != "")
    {
        ipPicDescriptor* pic=MITKipPicGet(const_cast<char *>(m_FileName.c_str()), output->GetPic());
        ConvertHandedness(pic);

        ipPicTSV_t *tsv;
        if ( (tsv = ipPicQueryTag( pic, "SOURCE HEADER" )) != NULL)
        {
          if(tsv->n[0]>1e+06)
          {
            ipPicTSV_t *tsvSH;
            tsvSH = ipPicDelTag( pic, "SOURCE HEADER" );
            ipPicFreeTag(tsvSH);
          }
        }
        if ( (tsv = ipPicQueryTag( pic, "ICON80x80" )) != NULL)
        {
          ipPicTSV_t *tsvSH;
          tsvSH = ipPicDelTag( pic, "ICON80x80" );
          ipPicFreeTag(tsvSH);
        }
        if ( (tsv = ipPicQueryTag( pic, "VELOCITY" )) != NULL)
        {
          ipPicDescriptor* header = ipPicCopyHeader(pic, NULL);
          header->data = tsv->value;
          ConvertHandedness(header);
          output->SetChannel(header->data, 1);
          header->data = NULL;
          ipPicFree(header);
          ipPicDelTag( pic, "VELOCITY" );
        }

        //slice-wise reading
        //currently much too slow.
        //else
        //{
        //  int sstart, smax;
        //  int tstart, tmax;

        //  sstart=output->GetRequestedRegion().GetIndex(2);
        //  smax=sstart+output->GetRequestedRegion().GetSize(2);

        //  tstart=output->GetRequestedRegion().GetIndex(3);
        //  tmax=tstart+output->GetRequestedRegion().GetSize(3);

        //  int s,t;
        //  for(s=sstart; s<smax; ++s)
        //  {
        //    for(t=tstart; t<tmax; ++t)
        //    {
        //      ipPicDescriptor* pic=ipPicGetSlice(const_cast<char *>(m_FileName.c_str()), NULL, t*smax+s+1);
        //      output->SetPicSlice(pic,s,t);
        //    }
        //  }
        //}
    }
    else
    {
        int position;
        ipPicDescriptor*  pic=NULL;

        int zDim=(output->GetDimension()>2?output->GetDimensions()[2]:1);
        printf("\n zdim is %u \n",zDim);

        for (position = 0; position < zDim; ++position) 
        {
            char fullName[1024];

            sprintf(fullName, m_FilePattern.c_str(), m_FilePrefix.c_str(), m_StartFileIndex+position);

            pic=MITKipPicGet(fullName, pic);
            if(pic==NULL)
            {
                itkDebugMacro("Pic file '" << fullName << "' does not exist."); 
            }
            else
            if(output->SetPicSlice(pic, position)==false)
            {
                itkDebugMacro("Image '" << fullName << "' could not be added to Image."); 
            }
       }
       if(pic!=NULL)
         ipPicFree(pic);
    }
}

void mitk::PicFileReader::EnlargeOutputRequestedRegion(itk::DataObject *output)
{
  output->SetRequestedRegionToLargestPossibleRegion();
}

bool mitk::PicFileReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern) 
{
  // First check the extension
  if(  filename == "" )
  {
      //std::cout<<"No filename specified."<<std::endl;
    return false;
  }

  // check if image is serie
  if( filePattern != "" && filePrefix != "" )
    return false;

  bool extensionFound = false;
  std::string::size_type PICPos = filename.rfind(".pic");
  if ((PICPos != std::string::npos)
      && (PICPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  PICPos = filename.rfind(".PIC");
  if ((PICPos != std::string::npos)
      && (PICPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  PICPos = filename.rfind(".pic.gz");
  if ((PICPos != std::string::npos)
      && (PICPos == filename.length() - 7))
    {
    extensionFound = true;
    }

  PICPos = filename.rfind(".seq");
  if ((PICPos != std::string::npos)
      && (PICPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  if( !extensionFound )
    {
      //std::cout<<"The filename extension is not recognized."<<std::endl;
    return false;
    }

  return true;
}

//##ModelId=3E1874D202D0
mitk::PicFileReader::PicFileReader()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

//##ModelId=3E1874E50179
mitk::PicFileReader::~PicFileReader()
{
}
