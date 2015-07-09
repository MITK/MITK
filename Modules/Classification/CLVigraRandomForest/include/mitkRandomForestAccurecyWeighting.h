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

#ifndef MITKRANDOMFORESTACCURACYWEIGHTING_H
#define MITKRANDOMFORESTACCURACYWEIGHTING_H

#include <mitkBaseData.h>
#include <Eigen/Dense>
#include <vigra/random_forest.hxx>

namespace mitk {

class RandomForestAccuracyWeighting
{
public:
  mitkClassMacroNoParent(RandomForestAccuracyWeighting);

  void SetTestPointMatrix(const Eigen::MatrixXd & point_matrix);
  void SetTestPointLabelVector(const Eigen::VectorXi &label_vector);
  void SetRandomForest(const vigra::RandomForest<int> & rf);
  Eigen::VectorXd GetWeightVector() const;
  void Update();

private:

  RandomForestAccuracyWeighting(){}
  virtual ~RandomForestAccuracyWeighting(){}

  Eigen::MatrixXd m_TestPointMatrix;
  Eigen::VectorXi m_TestPointLabelVector;
  Eigen::VectorXd m_WeightVector;
  vigra::RandomForest<int> m_RandomForest;


};

}


#endif
