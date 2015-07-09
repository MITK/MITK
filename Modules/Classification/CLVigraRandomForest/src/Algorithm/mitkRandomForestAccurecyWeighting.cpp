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

#include <mitkRandomForestAccurecyWeighting.h>

#include <mitkVigraRandomForestClassifier.h>

namespace mitk {
void RandomForestAccuracyWeighting::SetTestPointMatrix(const Eigen::MatrixXd & point_matrix)
{
  this->m_TestPointMatrix = point_matrix;
}

void RandomForestAccuracyWeighting::SetTestPointLabelVector(const Eigen::VectorXi & label_vector)
{
  this->m_TestPointLabelVector = label_vector;
}

void RandomForestAccuracyWeighting::SetRandomForest(const vigra::RandomForest<int> & rf)
{
  this->m_RandomForest = rf;
}

Eigen::VectorXd RandomForestAccuracyWeighting::GetWeightVector() const
{
  return this->m_WeightVector;
}

void RandomForestAccuracyWeighting::Update()
{

  for(const auto & tree : m_RandomForest.trees_ )
  {
    vigra::RandomForest<int> rf = m_RandomForest;
    rf.set_options().tree_count(1);
    rf.trees_.push_back(tree);

    Eigen::VectorXi Y_eigen_result(m_TestPointLabelVector);

    vigra::MultiArrayView<2, double> X_in(vigra::Shape2(m_TestPointMatrix.rows(),m_TestPointMatrix.cols()),m_TestPointMatrix.data());
    vigra::MultiArrayView<2, int> Y_out(vigra::Shape2(Y_eigen_result.rows(),Y_eigen_result.cols()),Y_eigen_result.data());

    rf.predictLabels(X_in,Y_out);

    unsigned int positives = 0;
    for(unsigned int i = 0 ; i < Y_eigen_result.rows(); i++)
    {
      MITK_INFO << m_TestPointLabelVector[i] << " " << Y_eigen_result[i];
      if(Y_eigen_result[i] == m_TestPointLabelVector[i])
      {
        positives++;
      }
    }

    MITK_INFO << "Ratio for tree "<< positives/double(Y_eigen_result.rows());
  }
}

}


