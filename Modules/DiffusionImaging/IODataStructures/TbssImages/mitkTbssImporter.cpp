/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkTbssImporter_cpp
#define __mitkTbssImporter_cpp

#include "mitkTbssImporter.h"
#include <QDir>
#include <QStringList>
#include "itkNrrdImageIO.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{



  mitk::TbssImage::Pointer mitk::TbssImporter::Import()
  {
    // read all images with all_*.nii.gz
    mitk::TbssImage::Pointer tbssImg = mitk::TbssImage::New();
    m_Data = DataImageType::New();

    mitk::Geometry3D* geo = m_InputVolume->GetGeometry();
    mitk::Vector3D spacing = geo->GetSpacing();
    mitk::Point3D origin = geo->GetOrigin();

    //Size size
    DataImageType::SizeType dataSize;
    dataSize[0] = m_InputVolume->GetDimension(0);
    dataSize[1] = m_InputVolume->GetDimension(1);
    dataSize[2] = m_InputVolume->GetDimension(2);

    m_Data->SetRegions(dataSize);

    // Set spacing
    DataImageType::SpacingType dataSpacing;
    dataSpacing[0] = spacing[0];
    dataSpacing[1] = spacing[1];
    dataSpacing[2] = spacing[2];
    m_Data->SetSpacing(dataSpacing);

    DataImageType::PointType dataOrigin;
    dataOrigin[0] = origin[0];
    dataOrigin[1] = origin[1];
    dataOrigin[2] = origin[2];
    m_Data->SetOrigin(dataOrigin);

    //Direction must be set
    DataImageType::DirectionType dir;
    const itk::Transform<float, 3, 3>* transform3D = geo->GetParametricTransform();
    itk::Transform<float,3,3>::ParametersType p = transform3D->GetParameters();
    int t=0;
    for(int i=0; i<3; i++)
    {
      for(int j=0; j<3; j++)
      {
        dir[j][i] = p[t]; // row-major order (where the column index varies the fastest)
        t++;
      }
    }

    m_Data->SetDirection(dir);

    // Set the length to one because otherwise allocate fails. Should be changed when groups/measurements are added
    m_Data->SetVectorLength(m_InputVolume->GetDimension(3));
    m_Data->Allocate();


    for(int i=0; i<dataSize[0]; i++)
    {
      for(int j=0; j<dataSize[1]; j++)
      {
        for(int k=0; k<dataSize[2]; k++)
        {
          itk::Index<3> ix;
          ix[0] = i;
          ix[1] = j;
          ix[2] = k;
          itk::VariableLengthVector<float> pixel = m_Data->GetPixel(ix);

          for(int z=0; z<pixel.Size(); z++)
          {
            mitk::Index3D ix;
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;

            float value = m_InputVolume->GetPixelValueByIndex(ix, z);
            pixel.SetElement(z, value);
          }
          m_Data->SetPixel(ix, pixel);
        }
      }
    }


//    mitk::CastToTbssImage(m_Data.GetPointer(), tbssImg);

  tbssImg->SetGroupInfo(m_Groups);
  tbssImg->SetMeasurementInfo(m_MeasurementInfo);
  tbssImg->SetImage(m_Data);


  tbssImg->InitializeFromVectorImage();

  return tbssImg;



  }

  mitk::TbssImage::Pointer mitk::TbssImporter::ImportMeta()
  {
    mitk::TbssImage::Pointer tbssImg = mitk::TbssImage::New();

    m_Data = DataImageType::New();

    std::vector< std::pair<mitk::TbssImage::MetaDataFunction, int> > metaInfo;

    // Gradient images are vector images, so they will add more dimensions to the vector
    int vecLength = m_MetaFiles.size();
    //Check if there is a gradient image

    for(int i=0; i < m_MetaFiles.size(); i++)
    {
      std::pair<std::string, std::string> p = m_MetaFiles.at(i);
      if(RetrieveTbssFunction(p.first) == mitk::TbssImage::GRADIENT_X)
      {
        vecLength += 2;
      }
    }


    int currIndex = 0;


    for(int i=0; i < m_MetaFiles.size(); i++)
    {
      std::pair<std::string, std::string> p = m_MetaFiles.at(i);
      std::string function = p.first;
      std::string file = p.second;

      // Add to metainfo to give the tbss image a function-index pair
      std::pair<mitk::TbssImage::MetaDataFunction, int> pair;


      pair.first = RetrieveTbssFunction(function);
      pair.second = i;

      if(pair.first == mitk::TbssImage::GRADIENT_X)
      {
        metaInfo.push_back(std::pair<mitk::TbssImage::MetaDataFunction, int>(mitk::TbssImage::GRADIENT_X, i));
        metaInfo.push_back(std::pair<mitk::TbssImage::MetaDataFunction, int>(mitk::TbssImage::GRADIENT_Y, i+1));
        metaInfo.push_back(std::pair<mitk::TbssImage::MetaDataFunction, int>(mitk::TbssImage::GRADIENT_Z, i+2));


        VectorReaderType::Pointer fileReader = VectorReaderType::New();
        fileReader->SetFileName(file);
        itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
        fileReader->SetImageIO(io);
        fileReader->Update();

        VectorImageType::Pointer img = fileReader->GetOutput();

        VectorImageType::SizeType size = img->GetLargestPossibleRegion().GetSize();

        if(i==0)
        {
          // First image in serie. Properties should be used to initialize m_Data
          m_Data->SetRegions(img->GetLargestPossibleRegion().GetSize());
          m_Data->SetSpacing(img->GetSpacing());
          m_Data->SetOrigin(img->GetOrigin());
          m_Data->SetDirection(img->GetDirection());
          m_Data->SetVectorLength(vecLength);
          m_Data->Allocate();
        }


        /* Dealing with a gradient image, so the size of the vector need to be increased by 2
            since this image contains 3 volumes. Old data should not be deleted*/



        for(int x=0; x<size[0]; x++)
        {
          for(int y=0; y<size[1]; y++)
          {
            for(int z=0; z<size[2]; z++)
            {
              itk::Index<3> ix;
              ix[0] = x;
              ix[1] = y;
              ix[2] = z;

              itk::VariableLengthVector<int> vec = img->GetPixel(ix);
              itk::VariableLengthVector<float> pixel = m_Data->GetPixel(ix);
              for(int j=0; j<vec.Size(); j++)
              {
                int pos = currIndex+j;
                float f = vec.GetElement(j);
                pixel.SetElement(pos, f);

              }
              m_Data->SetPixel(ix, pixel);
            }
          }
        }

        currIndex += img->GetVectorLength();
        tbssImg->SetContainsGradient(true);
        // Read vector image and add to m_Data
      }

      else {

        metaInfo.push_back(pair);
        FileReaderType3D::Pointer fileReader = FileReaderType3D::New();
        fileReader->SetFileName(file);
        fileReader->Update();

        FloatImage3DType::Pointer img = fileReader->GetOutput();

        FloatImage3DType::SizeType size = img->GetLargestPossibleRegion().GetSize();

        if(i==0)
        {
          // First image in serie. Properties should be used to initialize m_Data
          m_Data->SetRegions(img->GetLargestPossibleRegion().GetSize());
          m_Data->SetSpacing(img->GetSpacing());
          m_Data->SetOrigin(img->GetOrigin());
          m_Data->SetDirection(img->GetDirection());
          m_Data->SetVectorLength(vecLength);         
          m_Data->Allocate();
        }

        for(int x=0; x<size[0]; x++)
        {
          for(int y=0; y<size[1]; y++)
          {
            for(int z=0; z<size[2]; z++)
            {
              itk::Index<3> ix;
              ix[0] = x;
              ix[1] = y;
              ix[2] = z;

              float f = img->GetPixel(ix);
              itk::VariableLengthVector<float> pixel = m_Data->GetPixel(ix);
              pixel.SetElement(currIndex, f);
              m_Data->SetPixel(ix, pixel);

            }
          }
        }
      }

      if(pair.first == mitk::TbssImage::MEAN_FA_SKELETON)
      {
        tbssImg->SetContainsMeanSkeleton(true);
      }
      else if(pair.first == mitk::TbssImage::MEAN_FA_SKELETON_MASK)
      {
        tbssImg->SetContainsSkeletonMask(true);
      }
      else if(pair.first == mitk::TbssImage::DISTANCE_MAP)
      {
        tbssImg->SetContainsDistanceMap(true);
      }

      currIndex++;

    }

    tbssImg->SetIsMeta(true);
    tbssImg->SetImage(m_Data);
    tbssImg->SetMetaInfo(metaInfo);
    tbssImg->InitializeFromVectorImage();

    return tbssImg;
  }


  mitk::TbssImage::MetaDataFunction mitk::TbssImporter::RetrieveTbssFunction(std::string s)
  {
    if(s == "skeleton mask")
    {
      return mitk::TbssImage::MEAN_FA_SKELETON_MASK;
    }
    else if(s == "mean fa skeleton")
    {
      return mitk::TbssImage::MEAN_FA_SKELETON;
    }
    else if(s == "gradient image")
    {
      return mitk::TbssImage::GRADIENT_X;
    }
    else if(s == "tubular structure")
    {
      return mitk::TbssImage::TUBULAR_STRUCTURE;
    }
    else if(s == "distance map")
    {
      return mitk::TbssImage::DISTANCE_MAP;
    }
    return mitk::TbssImage::MISC;
  }

}



#endif // __mitkTbssImporter_cpp
