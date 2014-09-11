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
#include <mitkPixelTypeMultiplex.h>
#include "mitkImagePixelReadAccessor.h"

namespace mitk
{

TbssImage::Pointer TbssImporter::Import()
{
  mitk::TbssImage::Pointer tbssImg = mitk::TbssImage::New();
  mitkPixelTypeMultiplex1( Import, m_InputVolume->GetPixelType(), tbssImg);
  return tbssImg;
}

template <typename TPixel>
void TbssImporter::Import(const mitk::PixelType , mitk::TbssImage::Pointer tbssImg)
{
  // read all images with all_*.nii.gz
  MITK_INFO << "called import ...";
  m_Data = DataImageType::New();

  mitk::BaseGeometry* geo = m_InputVolume->GetGeometry();
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
  const itk::Transform<ScalarType, 3, 3>* transform3D = geo->GetIndexToWorldTransform();
  itk::Transform<ScalarType,3,3>::ParametersType p = transform3D->GetParameters();
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

  // Determine vector size of m_Data

  int vecSize = m_Data->GetVectorLength();

  MITK_INFO << "vecsize " <<vecSize;
  try {
    mitk::ImagePixelReadAccessor<TPixel,4> readTbss( m_InputVolume );

    for(unsigned int i=0; i<dataSize[0]; i++)
    {
      MITK_INFO << "i " << i << " / " << dataSize[0];
      for(unsigned int j=0; j<dataSize[1]; j++)
      {
        for(unsigned int k=0; k<dataSize[2]; k++)
        {
          itk::VariableLengthVector<float> pixel;
          itk::Index<3> id;
          itk::Index<4> ix;

          ix[0] = id[0] = i;
          ix[1] = id[1] = j;
          ix[2] = id[2] = k;

          pixel = m_Data->GetPixel(id);

          for(int z=0; z<vecSize; z++)
          {
            ix[3] = z;
            float value = readTbss.GetPixelByIndex(ix);
            pixel.SetElement(z, value);
          }
          m_Data->SetPixel(id, pixel);
        }
      }
    }
  }
  catch ( mitk::Exception& e )
  {
    MITK_ERROR << "TbssImporter::Import: No read access to tbss image: " << e.what() ;
  }

  tbssImg->SetGroupInfo(m_Groups);
  tbssImg->SetMeasurementInfo(m_MeasurementInfo);
  tbssImg->SetImage(m_Data);

  tbssImg->InitializeFromVectorImage();

}


}
#endif // __mitkTbssImporter_cpp
