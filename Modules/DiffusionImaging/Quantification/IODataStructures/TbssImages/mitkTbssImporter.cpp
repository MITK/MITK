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

#ifndef __mitkTbssImporter_cpp
#define __mitkTbssImporter_cpp

#include "mitkTbssImporter.h"

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
        dir[i][j] = p[t]; // row-major order (where the column index varies the fastest)
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


  tbssImg->SetGroupInfo(m_Groups);
  tbssImg->SetMeasurementInfo(m_MeasurementInfo);
  tbssImg->SetImage(m_Data);


  tbssImg->InitializeFromVectorImage();

  return tbssImg;



  }



}



#endif // __mitkTbssImporter_cpp
