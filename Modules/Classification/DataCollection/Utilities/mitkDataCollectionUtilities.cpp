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

int mitk::DCUtilities::VoxelInMask(mitk::DataCollection::Pointer dc, std::string mask)
{
  mitk::DataCollectionImageIterator<int, 3> maskIter(dc, mask);
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

  mitk::DataCollectionImageIterator<int, 3> maskIter(dc, mask);
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
  typedef mitk::DataCollectionImageIterator<int, 3> DataIterType;

  int numberOfVoxels = DCUtilities::VoxelInMask(dc,mask);
  int numberOfNames = names.size();

  mitk::DataCollectionImageIterator<int, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
    DataIterType iter(dc, names[i]);
    dataIter.push_back(iter);
  }

  Eigen::MatrixXi result(numberOfVoxels, names.size());
  int row = 0;
  while ( ! maskIter.IsAtEnd() )
  {
    if (maskIter.GetVoxel() > 0)
    {
      for (int col = 0; col < numberOfNames; ++col)
      {
        result(row,col) = dataIter[col].GetVoxel();
      }
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

  mitk::DataCollectionImageIterator<int, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
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
  typedef mitk::DataCollectionImageIterator<int, 3> DataIterType;

  int numberOfNames = names.size();

  mitk::DataCollectionImageIterator<int, 3> maskIter(dc, mask);
  std::vector<DataIterType> dataIter;

  for (int i = 0; i < numberOfNames; ++i)
  {
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