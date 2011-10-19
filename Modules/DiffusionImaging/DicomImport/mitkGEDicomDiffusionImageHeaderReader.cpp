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


#include "mitkGEDicomDiffusionImageHeaderReader.h"

#include "gdcmGlobal.h"
#include "gdcmDict.h"
#include "gdcmDicts.h"
#include "gdcmDictEntry.h"
#include "gdcmDictEntry.h"
#include "gdcmDict.h"
#include "gdcmFile.h"
#include "gdcmSerieHelper.h"


mitk::GEDicomDiffusionImageHeaderReader::GEDicomDiffusionImageHeaderReader()
{
}

mitk::GEDicomDiffusionImageHeaderReader::~GEDicomDiffusionImageHeaderReader()
{
}

// do the work
void mitk::GEDicomDiffusionImageHeaderReader::Update()
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

    // adapted from namic-sandbox
    // DicomToNrrdConverter.cxx

    VolumeReaderType::DictionaryArrayRawPointer inputDict
      = m_VolumeReader->GetMetaDataDictionaryArray();

    ReadPublicTags();

    float x0, y0, z0;
    float x1, y1, z1;
    std::string tag;
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    std::cout << "Slice 0: " << tag << std::endl;
    tag.clear();

    // assume volume interleaving, i.e. the second dicom file stores
    // the second slice in the same volume as the first dicom file
    itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
    std::cout << "Slice 1: " << tag << std::endl;
    x1 -= x0; y1 -= y0; z1 -= z0;
    x0 = x1*this->m_Output->xSlice + y1*this->m_Output->ySlice + z1*this->m_Output->zSlice;
    if (x0 < 0)
    {
      m_SliceOrderIS = false;
    }

    ReadPublicTags2();

    int nSliceInVolume;
    int nVolume;

    nSliceInVolume = m_sliceLocations.size();
    nVolume = m_nSlice/nSliceInVolume;

    // assume volume interleaving
    std::cout << "Number of Slices: " << m_nSlice << std::endl;
    std::cout << "Number of Volume: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;

    for (int k = 0; k < m_nSlice; k += nSliceInVolume)
    {
      tag.clear();
      bool exist = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0043|1039",  tag);
      float b = atof( tag.c_str() );
      this->m_Output->bValue = b;

      vnl_vector_fixed<double, 3> vect3d;
      if (!exist || b == 0)
      {
        vect3d.fill( 0 );
        this->m_Output->DiffusionVector = vect3d;
        continue;
      }

      vect3d.fill( 0 );
      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bb",  tag);
      vect3d[0] = atof( tag.c_str() );

      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bc",  tag);
      vect3d[1] = atof( tag.c_str() );

      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bd",  tag);
      vect3d[2] = atof( tag.c_str() );

      vect3d.normalize();
      this->m_Output->DiffusionVector = vect3d;
    }

    TransformGradients();

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


