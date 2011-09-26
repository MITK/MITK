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
#include "mitkPicHelper.h"
#include <itkImageFileReader.h>

extern "C" 
{
  mitkIpPicDescriptor * MITKipPicGet( char *infile_name, mitkIpPicDescriptor *pic );
  mitkIpPicDescriptor * MITKipPicGetTags( char *infile_name, mitkIpPicDescriptor *pic );
}

void mitk::PicFileReader::GenerateOutputInformation()
{
    Image::Pointer output = this->GetOutput();

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
        mitkIpPicDescriptor* header=mitkIpPicGetHeader(const_cast<char *>(m_FileName.c_str()), NULL);

        if ( !header )
        {
            throw itk::ImageFileReaderException(__FILE__, __LINE__, "File could not be read.");
        }

        header=MITKipPicGetTags(const_cast<char *>(m_FileName.c_str()), header);

        int channels = 1;

        mitkIpPicTSV_t *tsv;
        if ( (tsv = mitkIpPicQueryTag( header, "SOURCE HEADER" )) != NULL)
        {
          if(tsv->n[0]>1e+06)
          {
            mitkIpPicTSV_t *tsvSH;
            tsvSH = mitkIpPicDelTag( header, "SOURCE HEADER" );
            mitkIpPicFreeTag(tsvSH);
          }
        }
        if ( (tsv = mitkIpPicQueryTag( header, "ICON80x80" )) != NULL)
        {
          mitkIpPicTSV_t *tsvSH;
          tsvSH = mitkIpPicDelTag( header, "ICON80x80" );
          mitkIpPicFreeTag(tsvSH);
        }
        if ( (tsv = mitkIpPicQueryTag( header, "VELOCITY" )) != NULL)
        {
          ++channels;
          mitkIpPicDelTag( header, "VELOCITY" );
        }

        if( header == NULL || header->bpe == 0)
        {
            itk::ImageFileReaderException e(__FILE__, __LINE__);
            itk::OStringStream msg;
            msg << " Could not read file "
                << m_FileName.c_str();
            e.SetDescription(msg.str().c_str());
            throw e;
            return;
        }

        // First initialize the geometry of the output image by the pic-header
        SlicedGeometry3D::Pointer slicedGeometry = mitk::SlicedGeometry3D::New();
        PicHelper::InitializeEvenlySpaced(header, header->n[2], slicedGeometry);

        // if pic image only 3D, the n[3] value is not initialized
        unsigned int timesteps = 1;
        if( header->dim > 3 )
            timesteps = header->n[3];

        TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
        timeSliceGeometry->InitializeEvenlyTimed(slicedGeometry, timesteps);
        timeSliceGeometry->ImageGeometryOn();

        // Cast the pic descriptor to ImageDescriptor and initialize the output

        output->Initialize( CastToImageDescriptor(header));
        output->SetGeometry( timeSliceGeometry );
        mitkIpPicFree ( header );
    }
    else
    {
        int numberOfImages=0;
        m_StartFileIndex=0;

        mitkIpPicDescriptor* header=NULL;

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
                    header=mitkIpPicGetHeader(fullName, NULL);
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
        output->Initialize( CastToImageDescriptor(header) );
        mitkIpPicFree ( header );
    }

    m_ReadHeaderTime.Modified();
}

void mitk::PicFileReader::ConvertHandedness(mitkIpPicDescriptor* pic)
{
  //left to right handed conversion
  if(pic->dim>=3)
  {
      mitkIpPicDescriptor* slice=mitkIpPicCopyHeader(pic, NULL);
      slice->dim=2;
      size_t size=_mitkIpPicSize(slice);
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
      mitkIpPicFree(slice);
  }
}

void mitk::PicFileReader::GenerateData()
{
    Image::Pointer output = this->GetOutput();

    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FileName == "" && m_FilePrefix == "" )
    {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
    }

    if( m_FileName != "")
    {
        mitkIpPicDescriptor* outputPic = mitkIpPicNew();
        outputPic = CastToIpPicDescriptor(output, outputPic);
        mitkIpPicDescriptor* pic=MITKipPicGet(const_cast<char *>(m_FileName.c_str()),
                                              outputPic);
        // comes upside-down (in MITK coordinates) from PIC file
        ConvertHandedness(pic);

        mitkIpPicTSV_t *tsv;
        if ( (tsv = mitkIpPicQueryTag( pic, "SOURCE HEADER" )) != NULL)
        {
          if(tsv->n[0]>1e+06)
          {
            mitkIpPicTSV_t *tsvSH;
            tsvSH = mitkIpPicDelTag( pic, "SOURCE HEADER" );
            mitkIpPicFreeTag(tsvSH);
          }
        }
        if ( (tsv = mitkIpPicQueryTag( pic, "ICON80x80" )) != NULL)
        {
          mitkIpPicTSV_t *tsvSH;
          tsvSH = mitkIpPicDelTag( pic, "ICON80x80" );
          mitkIpPicFreeTag(tsvSH);
        }
        if ( (tsv = mitkIpPicQueryTag( pic, "VELOCITY" )) != NULL)
        {
          mitkIpPicDescriptor* header = mitkIpPicCopyHeader(pic, NULL);
          header->data = tsv->value;
          ConvertHandedness(header);
          output->SetChannel(header->data, 1);
          header->data = NULL;
          mitkIpPicFree(header);
          mitkIpPicDelTag( pic, "VELOCITY" );
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
        //      mitkIpPicDescriptor* pic=mitkIpPicGetSlice(const_cast<char *>(m_FileName.c_str()), NULL, t*smax+s+1);
        //      output->SetPicSlice(pic,s,t);
        //    }
        //  }
        //}
    }
    else
    {
        int position;
        mitkIpPicDescriptor*  pic=NULL;

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
            /* FIXME else
            if(output->SetPicSlice(pic, position)==false)
            {
                itkDebugMacro("Image '" << fullName << "' could not be added to Image."); 
            }*/
       }
       if(pic!=NULL)
         mitkIpPicFree(pic);
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
      //MITK_INFO<<"No filename specified."<<std::endl;
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
      //MITK_INFO<<"The filename extension is not recognized."<<std::endl;
    return false;
    }

  return true;
}

mitk::PicFileReader::PicFileReader()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::PicFileReader::~PicFileReader()
{
}
