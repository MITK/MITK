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

#include "mitkTrackingHandlerPeaks.h"

namespace mitk
{

TrackingHandlerPeaks::TrackingHandlerPeaks()
  : m_PeakThreshold(0.1)
{

}

TrackingHandlerPeaks::~TrackingHandlerPeaks()
{
}

void TrackingHandlerPeaks::InitForTracking()
{
  MITK_INFO << "Initializing peak tracker.";

  itk::Vector<float, 4> spacing4 = m_PeakImage->GetSpacing();
  itk::Point<float, 4> origin4 = m_PeakImage->GetOrigin();
  itk::Matrix<double, 4, 4> direction4 = m_PeakImage->GetDirection();
  itk::ImageRegion<4> imageRegion4 = m_PeakImage->GetLargestPossibleRegion();


  spacing3[0] = spacing4[0]; spacing3[1] = spacing4[1]; spacing3[2] = spacing4[2];
  origin3[0] = origin4[0]; origin3[1] = origin4[1]; origin3[2] = origin4[2];
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction3[r][c] = direction4[r][c];
  imageRegion3.SetSize(0, imageRegion4.GetSize()[0]);
  imageRegion3.SetSize(1, imageRegion4.GetSize()[1]);
  imageRegion3.SetSize(2, imageRegion4.GetSize()[2]);

  m_DummyImage = ItkUcharImgType::New();
  m_DummyImage->SetSpacing( spacing3 );
  m_DummyImage->SetOrigin( origin3 );
  m_DummyImage->SetDirection( direction3 );
  m_DummyImage->SetRegions( imageRegion3 );
  m_DummyImage->Allocate();
  m_DummyImage->FillBuffer(0.0);

  m_NumDirs = imageRegion4.GetSize(3)/3;
}

vnl_vector_fixed<double,3> TrackingHandlerPeaks::GetDirection(itk::Index<3> idx3, int dirIdx)
{
  vnl_vector_fixed<double,3> dir; dir.fill(0.0);
  if ( !m_DummyImage->GetLargestPossibleRegion().IsInside(idx3) )
    return dir;

  PeakImgType::IndexType idx4;
  idx4.SetElement(0,idx3[0]);
  idx4.SetElement(1,idx3[1]);
  idx4.SetElement(2,idx3[2]);

  for (int k=0; k<3; k++)
  {
    idx4.SetElement(3, dirIdx*3 + k);
    dir[k] = m_PeakImage->GetPixel(idx4);
  }

  vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0] = 1;
  vnl_vector_fixed<double,3> ref2; ref2.fill(0); ref2[1] = 1;
  vnl_vector_fixed<double,3> ref3; ref3.fill(0); ref3[2] = 1;
  if (dot_product(ref,dir)<0)
    dir *= -1;
  if (dot_product(ref2,dir)<0)
    dir *= -1;
  if (dot_product(ref3,dir)<0)
    dir *= -1;

  if (m_FlipX)
    dir[0] *= -1;
  if (m_FlipY)
    dir[1] *= -1;
  if (m_FlipZ)
    dir[2] *= -1;

  return dir;
}

vnl_vector_fixed<double,3> TrackingHandlerPeaks::GetDirection(int dirIdx, itk::Point<float, 3> itkP, bool interpolate){
  // transform physical point to index coordinates
  itk::Index<3> idx3;
  itk::ContinuousIndex< double, 3> cIdx;
  m_DummyImage->TransformPhysicalPointToIndex(itkP, idx3);
  m_DummyImage->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

  vnl_vector_fixed<double,3> dir; dir.fill(0.0);

  if ( !m_DummyImage->GetLargestPossibleRegion().IsInside(idx3) )
    return dir;
  else
    dir = GetDirection(idx3, dirIdx);

  if (!interpolate)
    return dir;
  else
  {
    double frac_x = cIdx[0] - idx3[0];
    double frac_y = cIdx[1] - idx3[1];
    double frac_z = cIdx[2] - idx3[2];
    if (frac_x<0)
    {
      idx3[0] -= 1;
      frac_x += 1;
    }
    if (frac_y<0)
    {
      idx3[1] -= 1;
      frac_y += 1;
    }
    if (frac_z<0)
    {
      idx3[2] -= 1;
      frac_z += 1;
    }
    frac_x = 1-frac_x;
    frac_y = 1-frac_y;
    frac_z = 1-frac_z;

    // int coordinates inside image?
    if (idx3[0] >= 0 && idx3[0] < m_DummyImage->GetLargestPossibleRegion().GetSize(0)-1 &&
        idx3[1] >= 0 && idx3[1] < m_DummyImage->GetLargestPossibleRegion().GetSize(1)-1 &&
        idx3[2] >= 0 && idx3[2] < m_DummyImage->GetLargestPossibleRegion().GetSize(2)-1)
    {
      // trilinear interpolation
      vnl_vector_fixed<double, 8> interpWeights;
      interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
      interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
      interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
      interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
      interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
      interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
      interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
      interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

      dir = GetDirection(idx3, dirIdx) * interpWeights[0];

      itk::Index<3> tmpIdx = idx3; tmpIdx[0]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[1];

      tmpIdx = idx3; tmpIdx[1]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[2];

      tmpIdx = idx3; tmpIdx[2]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[3];

      tmpIdx = idx3; tmpIdx[0]++; tmpIdx[1]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[4];

      tmpIdx = idx3; tmpIdx[1]++; tmpIdx[2]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[5];

      tmpIdx = idx3; tmpIdx[2]++; tmpIdx[0]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[6];

      tmpIdx = idx3; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
      dir +=  GetDirection(tmpIdx, dirIdx) * interpWeights[7];
    }
  }
  return dir;
}

vnl_vector_fixed<double,3> TrackingHandlerPeaks::ProposeDirection(itk::Point<double, 3>& pos, int& candidates, std::deque<vnl_vector_fixed<double, 3> >& olddirs, double angularThreshold, double& w, itk::Index<3>& oldIndex, ItkUcharImgType::Pointer mask)
{
  vnl_vector_fixed<double,3> output_direction; output_direction.fill(0);

  itk::Index<3> index;
  m_DummyImage->TransformPhysicalPointToIndex(pos, index);

  vnl_vector_fixed<double,3> oldDir;
  if (olddirs.size()>0 && olddirs.back().magnitude()>mitk::eps)
    oldDir = olddirs.at(0);
  else
  {
    float mag_max = 0;
    float mag = 0;
    for (int i=0; i<m_NumDirs; i++)
    {
        vnl_vector_fixed<double,3> temp_dir = GetDirection(i, pos, m_Interpolate);
        mag = temp_dir.magnitude();
        if (mag>mag_max)
        {
          mag_max = mag;
          output_direction = temp_dir;
        }
    }

    if (mag_max>m_PeakThreshold)
    {
      candidates = 1;
      w = mag_max;
      output_direction.normalize();
    }
    else
      output_direction.fill(0);

    return output_direction;
  }

  if (!m_Interpolate && oldIndex==index)
  {
    w = 1;
    candidates = 1;
    return oldDir;
  }

  float max_angle = 0;
  for (int i=0; i<m_NumDirs; i++)
  {
    vnl_vector_fixed<double,3> newDir = GetDirection(i, pos, m_Interpolate);
    float mag = newDir.magnitude();

    if (mag<m_PeakThreshold)
      continue;
    else
      newDir.normalize();

    double angle = dot_product(oldDir, newDir);
    if (angle<0)
    {
      newDir *= -1;
      angle *= -1;
    }

    if (angle>max_angle)
    {
      w = mag;
      max_angle = angle;
      output_direction = newDir;
    }
  }

  if (max_angle<angularThreshold)
  {
    output_direction.fill(0);
    return output_direction;
  }

  if (output_direction.magnitude()>mitk::eps)
    candidates = 1;

  return output_direction;
}

}

