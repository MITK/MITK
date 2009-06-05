/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSiemensDicomDiffusionVolumeHeaderReader.h"

#include "gdcm.h"
#include "gdcmGlobal.h"

// relevant Siemens private tags
const gdcm::DictEntry SiemensDictBValue( 0x0019, 0x100c, "IS", "1", "B Value of diffusion weighting" );       
const gdcm::DictEntry SiemensDictDiffusionDirection( 0x0019, 0x100e, "FD", "3", "Diffusion Gradient Direction" );       
const gdcm::DictEntry SiemensDictDiffusionMatrix( 0x0019, 0x1027, "FD", "6", "Diffusion Matrix" );       
const gdcm::DictEntry SiemensDictShadowInfo( 0x0029, 0x1010, "OB", "1", "Siemens DWI Info" );       

mitk::SiemensDicomDiffusionVolumeHeaderReader::SiemensDicomDiffusionVolumeHeaderReader()
{
}

mitk::SiemensDicomDiffusionVolumeHeaderReader::~SiemensDicomDiffusionVolumeHeaderReader()
{
}

int mitk::SiemensDicomDiffusionVolumeHeaderReader::ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray )
{
  int atPosition = tagString.find( nameString );
  if ( atPosition == std::string::npos)
  {
    return 0;
  }
  else
  {
    std::string infoAsString = tagString.substr( atPosition, tagString.size()-atPosition+1 );
    const char * infoAsCharPtr = infoAsString.c_str();

    int vm = *(infoAsCharPtr+64);
    std::string vr = infoAsString.substr( 68, 4 );
    //int syngodt = *(infoAsCharPtr+72);
    //int nItems = *(infoAsCharPtr+76);
    //int localDummy = *(infoAsCharPtr+80);

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

// do the work
void mitk::SiemensDicomDiffusionVolumeHeaderReader::Update()
{

  // check if there are filenames
  if(m_DicomFilenames.size())
  {
    // adapted from slicer
    // DicomToNrrdConverter.cxx

    VolumeReaderType::DictionaryArrayRawPointer inputDict 
      = m_VolumeReader->GetMetaDataDictionaryArray();

    //if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensMosiacParameters.GetKey()) == 0)
    //  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensMosiacParameters);
    //if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictNMosiac.GetKey()) == 0)
    //  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictNMosiac);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictBValue.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictBValue);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictDiffusionDirection.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionDirection);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictDiffusionMatrix.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionMatrix);
    if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictShadowInfo.GetKey()) == 0)
      gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictShadowInfo);

    ReadPublicTags();

    //int mMosaic;   // number of raws in each mosaic block;
    //int nMosaic;   // number of columns in each mosaic block

    float x0, y0, z0;
    float x1, y1, z1;
    std::string tag;
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    //std::cout << "Slice 0: " << tag << std::endl;
    tag.clear();

    // assume volume interleaving, i.e. the second dicom file stores
    // the second slice in the same volume as the first dicom file
    if((*inputDict).size() > 1)
    {
      itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|0032", tag );
      sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
      //std::cout << "Slice 1: " << tag << std::endl;
      x1 -= x0; y1 -= y0; z1 -= z0;
      x0 = x1*this->m_Output->xSlice + y1*this->m_Output->ySlice + z1*this->m_Output->zSlice;
      if (x0 < 0)
      {
        m_SliceOrderIS = false;
      }
    }

    ReadPublicTags2();

    int nStride = 1;
    //std::cout << orthoSliceSpacing << std::endl;
    this->m_Output->nSliceInVolume = m_sliceLocations.size();
    //nVolume = nSlice/this->m_Output->nSliceInVolume;
    //std::cout << "Number of Slices: " << m_nSlice << std::endl;
    //std::cout << "Number of Volume: " << nVolume << std::endl;
    //std::cout << "Number of Slices in each volume: " << this->m_Output->nSliceInVolume << std::endl;
    nStride = this->m_Output->nSliceInVolume;

    std::cout << "Dims " << this->m_Output->nRows << "x" 
      << this->m_Output->nCols << "x" << this->m_Output->nSliceInVolume << " ";

    for (int k = 0; k < m_nSlice; k += nStride )
    {

      gdcm::File *header0 = new gdcm::File;
      gdcm::BinEntry* binEntry;

      header0->SetMaxSizeLoadEntry(65536);
      header0->SetFileName( m_DicomFilenames[k] );
      header0->SetLoadMode( gdcm::LD_ALL );
      header0->Load();

      // copy information stored in 0029,1010 into a string for parsing
      gdcm::DocEntry* docEntry = header0->GetFirstEntry();
      while(docEntry)
      {
        if ( docEntry->GetKey() == "0029|1010"  )
        {
          binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
          int binLength = binEntry->GetFullLength();
          tag.resize( binLength );
          uint8_t * tagString = binEntry->GetBinArea();

          for (int n = 0; n < binLength; n++)
          {
            tag[n] = *(tagString+n);
          }
          break;
        }
        docEntry = header0->GetNextEntry();
      }

      // parse B_value from 0029,1010 tag
      std::vector<double> valueArray(0);
      vnl_vector_fixed<double, 3> vect3d;
      int nItems = ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);
      if (nItems != 1 || valueArray[0] == 0)  // did not find enough information
      {
        //std::cout << "Warning: Cannot find complete information on B_value in 0029|1010\n";
        this->m_Output->bValue = 0.0;
        vect3d.fill( 0.0 );
        this->m_Output->DiffusionVector = vect3d;  
        continue;
      }
      else 
      {
        this->m_Output->bValue = valueArray[0];
      }

      // parse DiffusionGradientDirection from 0029,1010 tag
      valueArray.resize(0);
      nItems = ExtractSiemensDiffusionInformation(tag, "DiffusionGradientDirection", valueArray);
      if (nItems != 3)  // did not find enough information
      {
        //std::cout << "Warning: Cannot find complete information on DiffusionGradientDirection in 0029|1010\n";
        vect3d.fill( 0 );
        this->m_Output->DiffusionVector = vect3d;
      }
      else 
      {
        vect3d[0] = valueArray[0];
        vect3d[1] = valueArray[1];
        vect3d[2] = valueArray[2];
        vect3d.normalize();
        this->m_Output->DiffusionVector = vect3d;
        std::cout << "BV: " << this->m_Output->bValue << " GD: " << this->m_Output->DiffusionVector;
      }
    }
    
    TransformGradients();

  }
}

//header = new mitk::DWIHeader(nRows, nCols, xRes, yRes, xOrigin,yOrigin, 
//  zOrigin, sliceThickness, sliceSpacing,nSliceInVolume, xRow, yRow, 
//  zRow, xCol,yCol, zCol, xSlice, ySlice, zSlice,bValues[0], DiffusionVectors[0], 
//  vendor,SliceMosaic);


//
//// do the work
//void DicomDiffusionVolumeHeaderReader::Update()
//{
//
//  // check if there are filenames
//  if(m_DicomFilenames.IsNotNull()
//    && m_DicomFilenames->size() > 0)
//  {
//    // adapted from namic-sandbox
//    // DicomToNrrdConverter.cxx
//
//    bool SliceOrderIS = true;
//    bool SingleSeries = true;
//
//    VolumeReaderType::DictionaryArrayRawPointer inputDict 
//      = m_VolumeReader->GetMetaDataDictionaryArray();
//
//    if ( vendor.find("GE") != std::string::npos )
//    {
//      // for GE data
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(GEDictBValue.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictBValue);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(GEDictXGradient.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictXGradient);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(GEDictYGradient.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictYGradient);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(GEDictZGradient.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictZGradient);
//    }
//    else if( vendor.find("SIEMENS") != std::string::npos )
//    {
//      // for Siemens data
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensMosiacParameters.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensMosiacParameters);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictNMosiac.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictNMosiac);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictBValue.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictBValue);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictDiffusionDirection.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionDirection);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictDiffusionMatrix.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionMatrix);
//      if(gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(SiemensDictShadowInfo.GetKey()) == 0)
//        gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictShadowInfo);
//
//    }
//    else if( vendor.find("PHILIPS") != std::string::npos )
//    {
//      // for philips data
//    }
//    else
//    {
//      std::cerr << "Unrecognized vendor.\n" << std::endl;
//    }
//
//    ReadPublicTags();
//
//    int mMosaic;   // number of raws in each mosaic block;
//    int nMosaic;   // number of columns in each mosaic block
//    int nSliceInVolume;
//
//    // figure out slice order and mosaic arrangement.
//    if ( vendor.find("GE") != std::string::npos ||
//      (vendor.find("SIEMENS") != std::string::npos && !SliceMosaic) )
//    {
//      float x0, y0, z0;
//      float x1, y1, z1;
//      tag.clear();
//      itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
//      sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
//      std::cout << "Slice 0: " << tag << std::endl;
//      tag.clear();
//
//      // assume volume interleaving, i.e. the second dicom file stores
//      // the second slice in the same volume as the first dicom file
//      itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|0032", tag );
//      sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
//      std::cout << "Slice 1: " << tag << std::endl;
//      x1 -= x0; y1 -= y0; z1 -= z0;
//      x0 = x1*xSlice + y1*ySlice + z1*zSlice;
//      if (x0 < 0)
//      {
//        SliceOrderIS = false;
//      }
//    }
//    else if ( vendor.find("SIEMENS") != std::string::npos && SliceMosaic )
//    {
//      std::cout << "Siemens SliceMosaic......" << std::endl;
//
//      SliceOrderIS = false;
//
//      // for siemens mosaic image, figure out mosaic slice order from 0029|1010
//      tag.clear();
//      gdcm::File *header0 = new gdcm::File;
//      gdcm::BinEntry* binEntry;
//
//      header0->SetMaxSizeLoadEntry(65536);
//      header0->SetFileName( filenames[0] );
//      header0->SetLoadMode( gdcm::LD_ALL );
//      header0->Load();
//
//      // copy information stored in 0029,1010 into a string for parsing
//      gdcm::DocEntry* docEntry = header0->GetFirstEntry();
//      while(docEntry)
//      {
//        if ( docEntry->GetKey() == "0029|1010"  )
//        {
//          binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
//          int binLength = binEntry->GetFullLength();
//          tag.resize( binLength );
//          uint8_t * tagString = binEntry->GetBinArea();
//
//          for (int k = 0; k < binLength; k++)
//          {
//            tag[k] = *(tagString+k);
//          }
//          break;
//        }
//        docEntry = header0->GetNextEntry();
//      }
//
//      // parse SliceNormalVector from 0029,1010 tag
//      std::vector<double> valueArray(0);
//      int nItems = ExtractSiemensDiffusionInformation(tag, "SliceNormalVector", valueArray);
//      if (nItems != 3)  // did not find enough information
//      {
//        std::cout << "Warning: Cannot find complete information on SliceNormalVector in 0029|1010\n";
//        std::cout << "         Slice order may be wrong.\n";
//      }
//      else if (valueArray[2] > 0)
//      {
//        SliceOrderIS = true;
//      }
//
//      // parse NumberOfImagesInMosaic from 0029,1010 tag
//      valueArray.resize(0);
//      nItems = ExtractSiemensDiffusionInformation(tag, "NumberOfImagesInMosaic", valueArray);
//      if (nItems == 0)  // did not find enough information
//      {
//        std::cout << "Warning: Cannot find complete information on NumberOfImagesInMosaic in 0029|1010\n";
//        std::cout << "         Resulting image may contain empty slices.\n";
//      }
//      else 
//      {
//        nSliceInVolume = static_cast<int>(valueArray[0]);
//        mMosaic = static_cast<int> (ceil(sqrt(valueArray[0])));
//        nMosaic = mMosaic;
//      }
//      std::cout << "Mosaic in " << mMosaic << " X " << nMosaic << " blocks (total number of blocks = " << valueArray[0] << ").\n"; 
//    }
//    else
//    {
//    }
//
//    ReadPublicTags2();
//
//    ////////////////////////////////////////////////////////////
//    // vendor dependent tags.
//    // read in gradient vectors and determin nBaseline and nMeasurement
//
//    if ( vendor.find("GE") != std::string::npos )
//    {
//      nSliceInVolume = sliceLocations.size();
//      nVolume = nSlice/nSliceInVolume;
//
//      // assume volume interleaving
//      std::cout << "Number of Slices: " << nSlice << std::endl;
//      std::cout << "Number of Volume: " << nVolume << std::endl;
//      std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
//
//      for (int k = 0; k < nSlice; k += nSliceInVolume)
//      {
//        tag.clear();
//        bool exist = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0043|1039",  tag);
//        float b = atof( tag.c_str() );
//        bValues.push_back(b);
//
//        vnl_vector_fixed<double, 3> vect3d;
//        if (!exist || b == 0)
//        {
//          vect3d.fill( 0 );
//          DiffusionVectors.push_back(vect3d);      
//          DiffusionVectorsOrig.push_back(vect3d);      
//          continue;
//        }
//
//        vect3d.fill( 0 );
//        tag.clear();
//        itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bb",  tag);
//        vect3d[0] = atof( tag.c_str() );
//
//        tag.clear();
//        itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bc",  tag);
//        vect3d[1] = atof( tag.c_str() );
//
//        tag.clear();
//        itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bd",  tag);
//        vect3d[2] = atof( tag.c_str() );
//
//        DiffusionVectorsOrig.push_back(vect3d);      
//        vect3d.normalize();
//        DiffusionVectors.push_back(vect3d);      
//      }
//    }
//    else if ( vendor.find("SIEMENS") != std::string::npos )
//    {
//
//      int nStride = 1;
//      if ( !SliceMosaic )
//      {
//        std::cout << orthoSliceSpacing << std::endl;
//        nSliceInVolume = sliceLocations.size();
//        nVolume = nSlice/nSliceInVolume;
//        std::cout << "Number of Slices: " << nSlice << std::endl;
//        std::cout << "Number of Volume: " << nVolume << std::endl;
//        std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
//        nStride = nSliceInVolume;
//      }
//      else
//      {
//        std::cout << "Data in Siemens Mosaic Format\n";
//        nVolume = nSlice;
//        std::cout << "Number of Volume: " << nVolume << std::endl;
//        std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
//        nStride = 1;
//      }
//
//
//      for (int k = 0; k < nSlice; k += nStride )
//      {
//
//        gdcm::File *header0 = new gdcm::File;
//        gdcm::BinEntry* binEntry;
//
//        header0->SetMaxSizeLoadEntry(65536);
//        header0->SetFileName( filenames[k] );
//        header0->SetLoadMode( gdcm::LD_ALL );
//        header0->Load();
//
//        // copy information stored in 0029,1010 into a string for parsing
//        gdcm::DocEntry* docEntry = header0->GetFirstEntry();
//        while(docEntry)
//        {
//          if ( docEntry->GetKey() == "0029|1010"  )
//          {
//            binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
//            int binLength = binEntry->GetFullLength();
//            tag.resize( binLength );
//            uint8_t * tagString = binEntry->GetBinArea();
//
//            for (int n = 0; n < binLength; n++)
//            {
//              tag[n] = *(tagString+n);
//            }
//            break;
//          }
//          docEntry = header0->GetNextEntry();
//        }
//
//        // parse B_value from 0029,1010 tag
//        std::vector<double> valueArray(0);
//        vnl_vector_fixed<double, 3> vect3d;
//        int nItems = ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);
//        if (nItems != 1 || valueArray[0] == 0)  // did not find enough information
//        {
//          std::cout << "Warning: Cannot find complete information on B_value in 0029|1010\n";
//          bValues.push_back( 0.0 );
//          vect3d.fill( 0.0 );
//          DiffusionVectors.push_back(vect3d);      
//          DiffusionVectorsOrig.push_back(vect3d);    
//          continue;
//        }
//        else 
//        {
//          bValues.push_back( valueArray[0] );
//        }
//
//        // parse DiffusionGradientDirection from 0029,1010 tag
//        valueArray.resize(0);
//        nItems = ExtractSiemensDiffusionInformation(tag, "DiffusionGradientDirection", valueArray);
//        if (nItems != 3)  // did not find enough information
//        {
//          std::cout << "Warning: Cannot find complete information on DiffusionGradientDirection in 0029|1010\n";
//          vect3d.fill( 0 );
//          DiffusionVectors.push_back(vect3d);      
//          DiffusionVectorsOrig.push_back(vect3d);      
//        }
//        else 
//        {
//          vect3d[0] = valueArray[0];
//          vect3d[1] = valueArray[1];
//          vect3d[2] = valueArray[2];
//          DiffusionVectorsOrig.push_back(vect3d);      
//          vect3d.normalize();
//          DiffusionVectors.push_back(vect3d);      
//          int p = bValues.size();
//          std::cout << "Image#: " << k << " BV: " << bValues[p-1] << " GD: " << DiffusionVectors[p-1] << std::endl;
//        }
//      }
//    }
//    else
//    {
//    }
//
//    TransformGradients();
//
//    ///////////////////////////////////////////////
//    // construct header info
//
//    header = new mitk::DWIHeader(nRows, nCols, xRes, yRes, xOrigin,yOrigin, 
//      zOrigin, sliceThickness, sliceSpacing,nSliceInVolume, xRow, yRow, 
//      zRow, xCol,yCol, zCol, xSlice, ySlice, zSlice,bValues[0], DiffusionVectors[0], 
//      vendor,SliceMosaic);
//
//    // set m_Output
//
//  }
//
//}
//
