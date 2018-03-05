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

#include <mitkDataCollectionUtilities.h>

#include <mitkDataCollectionImageIterator.h>

#include <mitkImageCast.h>

int mitk::DCUtilities::VoxelInMask(mitk::DataCollection::Pointer dc, std::string mask)
{
  mitk::DataCollectionImageIterator<unsigned char, 3> maskIter(dc, mask);
  int count = 0;
  while ( ! maskIter.IsAtEnd() )
  {
    if (maskIter.GetVoxel() > 0)
      ++count;
    ++maskIter;
  }
  return count;
}

Eigen::MatrixXd mitk::DCUtilities::DC3dDToMatrixXd(mitk::DataCollection::Pointer dc, std::string name, std::string mask)
{
  std::vector<std::string> names;
  names.push_back(name);
  return DC3dDToMatrixXd(dc, names, mask);
}

Eigen::MatrixXd mitk::DCUtilities::DC3dDToMatrixXd(mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask)
{
  typedef mitk::DataCollectionImageIterator<double, 3> DataIterType;

  int numberOfVoxels = DCUtilities::VoxelInMask(dc,mask);
  int numberOfNames = names.size();

  mitk::DataCollectionImageIterator<unsigned char, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
    DataIterType iter(dc, names[i]);
    dataIter.push_back(iter);
  }

  Eigen::MatrixXd result(numberOfVoxels, names.size());
  int row = 0;
  while ( ! maskIter.IsAtEnd() )
  {
    if (maskIter.GetVoxel() > 0)
    {
      for (int col = 0; col < numberOfNames; ++col)
      {
        result(row,col) = dataIter[col].GetVoxel();
      }
      ++row;
    }
    for (int col = 0; col < numberOfNames; ++col)
    {
      ++(dataIter[col]);
    }
    ++maskIter;
  }
  return result;
}

Eigen::MatrixXi mitk::DCUtilities::DC3dDToMatrixXi(mitk::DataCollection::Pointer dc, std::string name, std::string mask)
{
  std::vector<std::string> names;
  names.push_back(name);
  return DC3dDToMatrixXi(dc, names, mask);
}

Eigen::MatrixXi mitk::DCUtilities::DC3dDToMatrixXi(mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask)
{
  typedef mitk::DataCollectionImageIterator<unsigned char, 3> DataIterType;

  int numberOfVoxels = DCUtilities::VoxelInMask(dc,mask);
  int numberOfNames = names.size();

  mitk::DataCollectionImageIterator<unsigned char, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
    DataIterType iter(dc, names[i]);
    dataIter.push_back(iter);
  }

  Eigen::MatrixXi result(numberOfVoxels, names.size());
  result.setZero();
  int row = 0;
  while ( ! maskIter.IsAtEnd() )
  {
    if (maskIter.GetVoxel() > 0)
    {
      for (int col = 0; col < numberOfNames; ++col)
      {
        result(row,col) = dataIter[col].GetVoxel();
      }
      ++row;
    }
    for (int col = 0; col < numberOfNames; ++col)
    {
      ++(dataIter[col]);
    }
    ++maskIter;
  }
  return result;
}

void mitk::DCUtilities::MatrixToDC3d(const Eigen::MatrixXd &matrix, mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask)
{
  typedef mitk::DataCollectionImageIterator<double, 3> DataIterType;

  int numberOfNames = names.size();

  mitk::DataCollectionImageIterator<unsigned char, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
    EnsureDoubleImageInDC(dc,names[i],mask);
    DataIterType iter(dc, names[i]);
    dataIter.push_back(iter);
  }

  int row = 0;
  while ( ! maskIter.IsAtEnd() )
  {
    if (maskIter.GetVoxel() > 0)
    {
      for (int col = 0; col < numberOfNames; ++col)
      {
        dataIter[col].SetVoxel(matrix(row,col));
      }
      ++row;
    }
    for (int col = 0; col < numberOfNames; ++col)
    {
      ++(dataIter[col]);
    }
    ++maskIter;
  }
}

void mitk::DCUtilities::MatrixToDC3d(const Eigen::MatrixXi &matrix, mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask)
{
  typedef mitk::DataCollectionImageIterator<unsigned char, 3> DataIterType;

  int numberOfNames = names.size();

  mitk::DataCollectionImageIterator<unsigned char, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
    EnsureUCharImageInDC(dc,names[i],mask);
    DataIterType iter(dc, names[i]);
    dataIter.push_back(iter);
  }

  int row = 0;
  while ( ! maskIter.IsAtEnd() )
  {
    if (maskIter.GetVoxel() > 0)
    {
      for (int col = 0; col < numberOfNames; ++col)
      {
        (dataIter[col]).SetVoxel(matrix(row,col));
      }
      ++row;
    }
    for (int col = 0; col < numberOfNames; ++col)
    {
      ++(dataIter[col]);
    }
    ++maskIter;
  }
}

void mitk::DCUtilities::MatrixToDC3d(const Eigen::MatrixXd &matrix, mitk::DataCollection::Pointer dc, const std::string &name, std::string mask)
{
  std::vector<std::string> names;
  names.push_back(name);
  return MatrixToDC3d(matrix, dc, names, mask);
}

void mitk::DCUtilities::MatrixToDC3d(const Eigen::MatrixXi &matrix, mitk::DataCollection::Pointer dc, const std::string &name, std::string mask)
{
  std::vector<std::string> names;
  names.push_back(name);
  return MatrixToDC3d(matrix, dc, names, mask);
}

void mitk::DCUtilities::EnsureUCharImageInDC(mitk::DataCollection::Pointer dc, std::string name, std::string origin)
{
  typedef itk::Image<unsigned char, 3> FeatureImage;
  typedef itk::Image<unsigned char, 3> LabelImage;

  mitk::DataCollectionImageIterator<unsigned char , 3> iter( dc, origin);
  while (!iter.IsAtEnd())
  {
    ++iter;
  }

  if (dc->HasElement(origin))
  {
    LabelImage::Pointer originImage = dynamic_cast<LabelImage*>(dc->GetData(origin).GetPointer());
    //    = dynamic_cast<LabelImage*>(dc->GetItkImage<LabelImage>(origin).GetPointer());
    if (!dc->HasElement(name) && originImage.IsNotNull())
    {
      MITK_INFO << "New unsigned char image necessary";
      FeatureImage::Pointer image = FeatureImage::New();
      image->SetRegions(originImage->GetLargestPossibleRegion());
      image->SetSpacing(originImage->GetSpacing());
      image->SetOrigin(originImage->GetOrigin());
      image->SetDirection(originImage->GetDirection());
      image->Allocate();
      image->FillBuffer(0);

      dc->AddData(dynamic_cast<itk::DataObject*>(image.GetPointer()),name,"");
    }
  }
  for (std::size_t i = 0; i < dc->Size();++i)
  {
    mitk::DataCollection* newCol = dynamic_cast<mitk::DataCollection*>(dc->GetData(i).GetPointer());
    if (newCol != nullptr)
    {
      EnsureUCharImageInDC(newCol, name, origin);
    }
  }
}

void mitk::DCUtilities::EnsureDoubleImageInDC(mitk::DataCollection::Pointer dc, std::string name, std::string origin)
{
  typedef itk::Image<double, 3> FeatureImage;
  typedef itk::Image<unsigned char, 3> LabelImage;

  mitk::DataCollectionImageIterator<unsigned char , 3> iter( dc, origin);
  while (!iter.IsAtEnd())
  {
    ++iter;
  }

  if (dc->HasElement(origin))
  {
    LabelImage::Pointer originImage = dynamic_cast<LabelImage*>(dc->GetData(origin).GetPointer());
    if (!dc->HasElement(name) && originImage.IsNotNull())
    {
      MITK_INFO << "New double image necessary";
      FeatureImage::Pointer image = FeatureImage::New();
      image->SetRegions(originImage->GetLargestPossibleRegion());
      image->SetSpacing(originImage->GetSpacing());
      image->SetOrigin(originImage->GetOrigin());
      image->SetDirection(originImage->GetDirection());
      image->Allocate();

      dc->AddData(dynamic_cast<itk::DataObject*>(image.GetPointer()),name,"");
    }
  }
  for (std::size_t i = 0; i < dc->Size();++i)
  {
    mitk::DataCollection* newCol = dynamic_cast<mitk::DataCollection*>(dc->GetData(i).GetPointer());
    if (newCol != nullptr)
    {
      EnsureDoubleImageInDC(newCol, name, origin);
    }
  }
}