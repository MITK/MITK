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