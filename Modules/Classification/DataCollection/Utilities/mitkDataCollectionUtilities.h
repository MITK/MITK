/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataCollectionUtilities_h
#define mitkDataCollectionUtilities_h

#include <MitkDataCollectionExports.h>

#include <mitkDataCollection.h>
#include <Eigen/Dense>

namespace mitk
{
  class MITKDATACOLLECTION_EXPORT DCUtilities
  {
  public:
    static int VoxelInMask(mitk::DataCollection::Pointer dc, std::string mask);

    static Eigen::MatrixXd DC3dDToMatrixXd(mitk::DataCollection::Pointer dc, std::string names, std::string mask);
    static Eigen::MatrixXd DC3dDToMatrixXd(mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask);
    static Eigen::MatrixXi DC3dDToMatrixXi(mitk::DataCollection::Pointer dc, std::string name, std::string mask);
    static Eigen::MatrixXi DC3dDToMatrixXi(mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask);

    static void MatrixToDC3d(const Eigen::MatrixXd &matrix, mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask);
    static void MatrixToDC3d(const Eigen::MatrixXi &matrix, mitk::DataCollection::Pointer dc, const std::vector<std::string> &names, std::string mask);
    static void MatrixToDC3d(const Eigen::MatrixXd &matrix, mitk::DataCollection::Pointer dc, const std::string &names, std::string mask);
    static void MatrixToDC3d(const Eigen::MatrixXi &matrix, mitk::DataCollection::Pointer dc, const std::string &names, std::string mask);

    static void EnsureUCharImageInDC(mitk::DataCollection::Pointer dc, std::string name, std::string origin);
    static void EnsureDoubleImageInDC(mitk::DataCollection::Pointer dc, std::string name, std::string origin);
  };
}

#endif
