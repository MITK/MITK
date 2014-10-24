/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __mitkNrrdDiffusionImageReader_cpp
#define __mitkNrrdDiffusionImageReader_cpp

#include "mitkNrrdDiffusionImageReader.h"

#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkNiftiImageIO.h"

#include <iostream>
#include <fstream>

#include "itksys/SystemTools.hxx"
#include "mitkCustomMimeType.h"
#include "mitkDiffusionIOMimeTypes.h"

namespace mitk
{

NrrdDiffusionImageReader::
NrrdDiffusionImageReader(const NrrdDiffusionImageReader & other)
  : AbstractFileReader(other)
{
}


NrrdDiffusionImageReader* NrrdDiffusionImageReader::Clone() const
{
  return new NrrdDiffusionImageReader(*this);
}


NrrdDiffusionImageReader::
~NrrdDiffusionImageReader()
{}


NrrdDiffusionImageReader::
NrrdDiffusionImageReader()
  : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionIOMimeTypes::DWI_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::DWI_MIMETYPE_DESCRIPTION() )
{
  m_ServiceReg = this->RegisterService();
}

std::vector<itk::SmartPointer<mitk::BaseData> >
NrrdDiffusionImageReader::
Read()
{
  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  // Since everything is completely read in GenerateOutputInformation() it is stored
  // in a cache variable. A timestamp is associated.
  // If the timestamp of the cache variable is newer than the MTime, we only need to
  // assign the cache variable to the DataObject.
  // Otherwise, the tree must be read again from the file and OuputInformation must
  // be updated!

  if(m_OutputCache.IsNull()) InternalRead();

  OutputType::Pointer resultImage = OutputType::New();
  resultImage->SetVectorImage(m_OutputCache->GetVectorImage());
  resultImage->SetReferenceBValue(m_OutputCache->GetReferenceBValue());
  resultImage->SetMeasurementFrame(m_OutputCache->GetMeasurementFrame());
  resultImage->SetDirections(m_OutputCache->GetDirections());
  resultImage->InitializeFromVectorImage();

  result.push_back(resultImage.GetPointer());
  return result;
}


void NrrdDiffusionImageReader::InternalRead()
{
  OutputType::Pointer outputForCache = OutputType::New();
  if ( this->GetInputLocation() == "")
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename to be read is empty!");
  }
  else
  {
    try
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


      MITK_INFO << "NrrdDiffusionImageReader: reading image information";
      ImageType::Pointer img;

      std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetInputLocation());
      ext = itksys::SystemTools::LowerCase(ext);
      if (ext == ".hdwi" || ext == ".dwi")
      {
        typedef itk::ImageFileReader<ImageType> FileReaderType;
        FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetFileName(this->GetInputLocation());
        itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
        reader->SetImageIO(io);
        reader->Update();
        img = reader->GetOutput();

        int vecsize = img->GetVectorLength();
        std::cout << vecsize << std::endl;


      }
      else if(ext == ".fsl" || ext == ".fslgz")
      {
        // create temporary file with correct ending for nifti-io
        std::string fname3 = "temp_dwi";
        fname3 += ext == ".fsl" ? ".nii" : ".nii.gz";
        itksys::SystemTools::CopyAFile(this->GetInputLocation().c_str(), fname3.c_str());

        // create reader and read file
        typedef itk::Image<short,4> ImageType4D;
        itk::NiftiImageIO::Pointer io2 = itk::NiftiImageIO::New();
        typedef itk::ImageFileReader<ImageType4D> FileReaderType;
        FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetFileName(fname3);
        reader->SetImageIO(io2);
        reader->Update();
        ImageType4D::Pointer img4 = reader->GetOutput();

        // delete temporary file
        itksys::SystemTools::RemoveFile(fname3.c_str());

        // convert 4D file to vector image
        img = ImageType::New();

        ImageType::SpacingType spacing;
        ImageType4D::SpacingType spacing4 = img4->GetSpacing();
        for(int i=0; i<3; i++)
          spacing[i] = spacing4[i];
        img->SetSpacing( spacing );   // Set the image spacing

        ImageType::PointType origin;
        ImageType4D::PointType origin4 = img4->GetOrigin();
        for(int i=0; i<3; i++)
          origin[i] = origin4[i];
        img->SetOrigin( origin );     // Set the image origin

        ImageType::DirectionType direction;
        ImageType4D::DirectionType direction4 = img4->GetDirection();
        for(int i=0; i<3; i++)
          for(int j=0; j<3; j++)
            direction[i][j] = direction4[i][j];
        img->SetDirection( direction );  // Set the image direction

        ImageType::RegionType region;
        ImageType4D::RegionType region4 = img4->GetLargestPossibleRegion();

        ImageType::RegionType::SizeType size;
        ImageType4D::RegionType::SizeType size4 = region4.GetSize();

        for(int i=0; i<3; i++)
          size[i] = size4[i];

        ImageType::RegionType::IndexType index;
        ImageType4D::RegionType::IndexType index4 = region4.GetIndex();
        for(int i=0; i<3; i++)
          index[i] = index4[i];

        region.SetSize(size);
        region.SetIndex(index);
        img->SetRegions( region );

        img->SetVectorLength(size4[3]);
        img->Allocate();

        itk::ImageRegionIterator<ImageType>   it (img, img->GetLargestPossibleRegion() );
        typedef ImageType::PixelType VecPixType;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it)
        {
          VecPixType vec = it.Get();
          ImageType::IndexType currentIndex = it.GetIndex();
          for(int i=0; i<3; i++)
            index4[i] = currentIndex[i];
          for(unsigned int ind=0; ind<vec.Size(); ind++)
          {
            index4[3] = ind;
            vec[ind] = img4->GetPixel(index4);
          }
          it.Set(vec);
        }
      }

      m_DiffusionVectors = GradientDirectionContainerType::New();
      m_OriginalDiffusionVectors = GradientDirectionContainerType::New();
      if (ext == ".hdwi" || ext == ".dwi")
      {

        itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
        std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
        std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
        std::string metaString;

        GradientDirectionType vect3d;

        int numberOfImages = 0;
        int numberOfGradientImages = 0;
        bool readb0 = false;
        double xx, xy, xz, yx, yy, yz, zx, zy, zz;

        for (; itKey != imgMetaKeys.end(); itKey ++)
        {
          double x,y,z;

          itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
          if (itKey->find("DWMRI_gradient") != std::string::npos)
          {
            sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
            vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
            m_DiffusionVectors->InsertElement( numberOfImages, vect3d );
            ++numberOfImages;
            // If the direction is 0.0, this is a reference image
            if (vect3d[0] == 0.0 &&
                vect3d[1] == 0.0 &&
                vect3d[2] == 0.0)
            {
              continue;
            }
            ++numberOfGradientImages;;
          }
          else if (itKey->find("DWMRI_b-value") != std::string::npos)
          {
            readb0 = true;
            m_B_Value = atof(metaString.c_str());
          }
          else if (itKey->find("measurement frame") != std::string::npos)
          {
            sscanf(metaString.c_str(), " ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) \n", &xx, &xy, &xz, &yx, &yy, &yz, &zx, &zy, &zz);

            if (xx>10e-10 || xy>10e-10 || xz>10e-10 ||
                yx>10e-10 || yy>10e-10 || yz>10e-10 ||
                zx>10e-10 || zy>10e-10 || zz>10e-10 )
            {
              m_MeasurementFrame(0,0) = xx;
              m_MeasurementFrame(0,1) = xy;
              m_MeasurementFrame(0,2) = xz;
              m_MeasurementFrame(1,0) = yx;
              m_MeasurementFrame(1,1) = yy;
              m_MeasurementFrame(1,2) = yz;
              m_MeasurementFrame(2,0) = zx;
              m_MeasurementFrame(2,1) = zy;
              m_MeasurementFrame(2,2) = zz;
            }
            else
            {
              m_MeasurementFrame(0,0) = 1;
              m_MeasurementFrame(0,1) = 0;
              m_MeasurementFrame(0,2) = 0;
              m_MeasurementFrame(1,0) = 0;
              m_MeasurementFrame(1,1) = 1;
              m_MeasurementFrame(1,2) = 0;
              m_MeasurementFrame(2,0) = 0;
              m_MeasurementFrame(2,1) = 0;
              m_MeasurementFrame(2,2) = 1;
            }
          }
        }

        if(!readb0)
        {
          MITK_INFO << "BValue not specified in header file";
        }

      }
      else if(ext == ".fsl" || ext == ".fslgz")
      {

        std::string line;
        std::vector<float> bvec_entries;
        std::string fname = this->GetInputLocation();
        fname += ".bvecs";
        std::ifstream myfile (fname.c_str());
        if (myfile.is_open())
        {
          while ( myfile.good() )
          {
            getline (myfile,line);
            char* pch = strtok (const_cast<char*>(line.c_str())," ");
            while (pch != NULL)
            {
              bvec_entries.push_back(atof(pch));
              pch = strtok (NULL, " ");
            }
          }
          myfile.close();
        }
        else
        {
          MITK_INFO << "Unable to open bvecs file";
        }

        std::vector<float> bval_entries;
        std::string fname2 = this->GetInputLocation();
        fname2 += ".bvals";
        std::ifstream myfile2 (fname2.c_str());
        if (myfile2.is_open())
        {
          while ( myfile2.good() )
          {
            getline (myfile2,line);
            char* pch = strtok (const_cast<char*>(line.c_str())," ");
            while (pch != NULL)
            {
              bval_entries.push_back(atof(pch));
              pch = strtok (NULL, " ");
            }
          }
          myfile2.close();
        }
        else
        {
          MITK_INFO << "Unable to open bvals file";
        }


        m_B_Value = -1;
        unsigned int numb = bval_entries.size();
        for(unsigned int i=0; i<numb; i++)
        {

          // Take the first entry in bvals as the reference b-value
          if(m_B_Value == -1 && bval_entries.at(i) != 0)
          {
            m_B_Value = bval_entries.at(i);
          }

          float b_val = bval_entries.at(i);


          vnl_vector_fixed< double, 3 > vec;
          vec[0] = bvec_entries.at(i);
          vec[1] = bvec_entries.at(i+numb);
          vec[2] = bvec_entries.at(i+2*numb);

          // Adjust the vector length to encode gradient strength
          float factor = b_val/m_B_Value;
          if(vec.magnitude() > 0)
          {
            vec[0] = sqrt(factor)*vec[0];
            vec[1] = sqrt(factor)*vec[1];
            vec[2] = sqrt(factor)*vec[2];
          }

          m_DiffusionVectors->InsertElement(i,vec);
        }

        for(int i=0; i<3; i++)
          for(int j=0; j<3; j++)
            m_MeasurementFrame[i][j] = i==j ? 1 : 0;
      }

      outputForCache->SetVectorImage(img);
      outputForCache->SetReferenceBValue(m_B_Value);
      outputForCache->SetMeasurementFrame(m_MeasurementFrame);
      outputForCache->SetDirections(m_DiffusionVectors);


      // Since we have already read the tree, we can store it in a cache variable
      // so that it can be assigned to the DataObject in GenerateData();
      m_OutputCache = outputForCache;
      m_CacheTime.Modified();

      try
      {
        setlocale(LC_ALL, currLocale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not reset locale " << currLocale;
      }
    }
    catch(std::exception& e)
    {
      MITK_INFO << "Std::Exception while reading file!!";
      MITK_INFO << e.what();
      throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
    }
    catch(...)
    {
      MITK_INFO << "Exception while reading file!!";
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
    }
  }
}

} //namespace MITK

#endif
