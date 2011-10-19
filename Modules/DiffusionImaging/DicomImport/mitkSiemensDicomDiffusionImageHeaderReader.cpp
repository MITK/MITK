/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSiemensDicomDiffusionImageHeaderReader.h"
#include "gdcmGlobal.h"
//#include <gdcmVersion.h>

#include "gdcmFile.h"
#include "gdcmImageReader.h"
#include "gdcmDictEntry.h"
#include "gdcmDicts.h"
#include "gdcmTag.h"

mitk::SiemensDicomDiffusionImageHeaderReader::SiemensDicomDiffusionImageHeaderReader()
{
}

mitk::SiemensDicomDiffusionImageHeaderReader::~SiemensDicomDiffusionImageHeaderReader()
{
}

int mitk::SiemensDicomDiffusionImageHeaderReader::ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray, int startPos )
{
  std::string::size_type atPosition = tagString.find( nameString, startPos );
  if ( atPosition == std::string::npos)
  {
    return 0;
  }
  else
  {
    std::string infoAsString = tagString.substr( atPosition, tagString.size()-atPosition+1 );
    const char * infoAsCharPtr = infoAsString.c_str();

    int vm = *(infoAsCharPtr+64);

    int offset = 84;
    for (int k = 0; k < vm; k++)
    {
      int itemLength = *(infoAsCharPtr+offset+4);
      int strideSize = static_cast<int> (ceil(static_cast<double>(itemLength)/4) * 4);
      std::string valueString = infoAsString.substr( offset+16, itemLength );
      valueArray.push_back( atof(valueString.c_str()) );
      offset += 16+strideSize;
    }
    return vm;
  }
}

int mitk::SiemensDicomDiffusionImageHeaderReader::ExtractSiemensDiffusionGradientInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray )
{
  int nItems = 0;
  std::string::size_type pos = -1;
  while(nItems != 3)
  {
    nItems = ExtractSiemensDiffusionInformation( tagString, nameString, valueArray, pos+1 );
    pos = tagString.find( nameString, pos+1 );
    if ( pos == std::string::npos )
    {
      break;
    }
  }
  return nItems;
}

// do the work
void mitk::SiemensDicomDiffusionImageHeaderReader::Update()
{

  // check if there are filenames
  if(m_DicomFilenames.size())
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

    if ( locale.compare(currLocale)!=0 )
    {
      try
      {
        setlocale(LC_ALL, locale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not set locale " << locale;
      }
    }

    // adapted from slicer
    // DicomToNrrdConverter.cxx

    VolumeReaderType::DictionaryArrayRawPointer
        inputDict = m_VolumeReader->GetMetaDataDictionaryArray();

    ReadPublicTags();

    float x0, y0, z0;
    float x1, y1, z1;
    std::string tag;
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    //MITK_INFO << "Slice 0: " << tag << std::endl;
    tag.clear();

    // assume volume interleaving, i.e. the second dicom file stores
    // the second slice in the same volume as the first dicom file
    if((*inputDict).size() > 1)
    {
      itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|0032", tag );
      sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
      //MITK_INFO << "Slice 1: " << tag << std::endl;
      x1 -= x0; y1 -= y0; z1 -= z0;
      x0 = x1*this->m_Output->xSlice + y1*this->m_Output->ySlice + z1*this->m_Output->zSlice;
      if (x0 < 0)
      {
        m_SliceOrderIS = false;
      }
    }

    ReadPublicTags2();

    int nStride = 1;
    this->m_Output->nSliceInVolume = m_sliceLocations.size();

    nStride = this->m_Output->nSliceInVolume;

    MITK_INFO << m_DicomFilenames[0] << std::endl;
    MITK_INFO << "Dims " << this->m_Output->nRows << "x"
        << this->m_Output->nCols << "x" << this->m_Output->nSliceInVolume << " " << std::endl;

    for (int k = 0; k < m_nSlice; k += nStride )
    {

      gdcm::ImageReader reader;
      reader.SetFileName( m_DicomFilenames[k].c_str() );
      if( !reader.Read() )
      {
        itkExceptionMacro(<< "Cannot read requested file");
      }
      const gdcm::File &f = reader.GetFile();
      const gdcm::DataSet &ds = f.GetDataSet();

      //    gdcm::DataSet ds = header0->GetDataSet();
      gdcm::DataSet::ConstIterator it = ds.Begin();

      // Copy of the header->content
      // copy information stored in 0029,1010 into a string for parsing
      for(; it != ds.End(); ++it)
      {
        const gdcm::DataElement &ref = *it;
        if (ref.GetTag() == gdcm::Tag(0x0029,0x1010)) {
          tag = std::string(ref.GetByteValue()->GetPointer(),ref.GetByteValue()->GetLength());
        }
      }

      // parse B_value from 0029,1010 tag
      std::vector<double> valueArray(0);
      vnl_vector_fixed<double, 3> vect3d;
      int nItems = ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);
      if (nItems != 1 || valueArray[0] == 0)  // did not find enough information
      {
        tag.clear();
        MITK_INFO << "Reading diffusion info from 0019|100c and 0019|100e tags" << std::endl;
        bool success = false;

        for(it = ds.Begin(); it != ds.End(); ++it)
        {
          const gdcm::DataElement &ref = *it;
          if (ref.GetTag() == gdcm::Tag(0x0019,0x100c)) {
            tag = std::string(ref.GetByteValue()->GetPointer(),ref.GetByteValue()->GetLength());
            this->m_Output->bValue = atof( tag.c_str() );
            success = true;
          }
        }

        tag.clear();
        if(success)
        {
          if(this->m_Output->bValue == 0)
          {
            MITK_INFO << "BV: 0 (Baseline image)";
            continue;
          }

          success = false;
          for(it = ds.Begin(); it != ds.End(); ++it)
          {
            const gdcm::DataElement &ref = *it;
            if (ref.GetTag() == gdcm::Tag(0x0019,0x100e)) {
              tag = std::string(ref.GetByteValue()->GetPointer(),ref.GetByteValue()->GetLength());
              success = true;
            }
          }

          if(success)
          {
            memcpy( &vect3d[0], tag.c_str()+0, 8 );
            memcpy( &vect3d[1], tag.c_str()+8, 8 );
            memcpy( &vect3d[2], tag.c_str()+16, 8 );
            vect3d.normalize();
            this->m_Output->DiffusionVector = vect3d;
            TransformGradients();
            MITK_INFO << "BV: " << this->m_Output->bValue;
            MITK_INFO << " GD: " << this->m_Output->DiffusionVector;
            continue;
          }
        }
      }
      else
      {
        MITK_INFO << "Reading diffusion info from 0029,1010 tag" << std::endl;
        this->m_Output->bValue = valueArray[0];

        if(this->m_Output->bValue == 0)
        {
          MITK_INFO << "BV: 0 (Baseline image)";
          continue;
        }

        valueArray.resize(0);
        nItems = ExtractSiemensDiffusionGradientInformation(tag, "DiffusionGradientDirection", valueArray);
        if (nItems == 3)
        {
          vect3d[0] = valueArray[0];
          vect3d[1] = valueArray[1];
          vect3d[2] = valueArray[2];
          vect3d.normalize();
          this->m_Output->DiffusionVector = vect3d;
          TransformGradients();
          MITK_INFO << "BV: " << this->m_Output->bValue;
          MITK_INFO << " GD: " << this->m_Output->DiffusionVector;
          continue;
        }
      }

      MITK_ERROR << "No diffusion info found, forcing to BASELINE image." << std::endl;
      this->m_Output->bValue = 0.0;
      vect3d.fill( 0.0 );
      this->m_Output->DiffusionVector = vect3d;

    }
    try
    {
      setlocale(LC_ALL, currLocale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not reset locale " << currLocale;
    }
  }
}
