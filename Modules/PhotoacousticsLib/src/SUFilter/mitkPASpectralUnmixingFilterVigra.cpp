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

#include "mitkPASpectralUnmixingFilterVigra.h"

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// Testing algorithms
#include <eigen3\Eigen\src\SVD\JacobiSVD.h>

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

//vigra
/*
The VIGRA License
=================
(identical to the MIT X11 License)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
#include <vigra/matrix.hxx>
#include <vigra/regression.hxx>
#include <vigra/quadprog.hxx>

mitk::pa::SpectralUnmixingFilterVigra::SpectralUnmixingFilterVigra()
{
 
}

mitk::pa::SpectralUnmixingFilterVigra::~SpectralUnmixingFilterVigra()
{

}

void mitk::pa::SpectralUnmixingFilterVigra::SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType SetAlgorithmIndex)
{
  algorithmIndex = SetAlgorithmIndex;
}


Eigen::VectorXf mitk::pa::SpectralUnmixingFilterVigra::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{
  int numberOfWavelengths = EndmemberMatrix.rows();
  int numberOfChromophores = EndmemberMatrix.cols();
  Eigen::VectorXf resultVector(numberOfChromophores);

  using namespace vigra;
  using namespace vigra::linalg;

  std::vector<double> A_data;
  std::vector<double> B_data;


  for (int i = 0; i < numberOfWavelengths; ++i)
  {
    B_data.push_back(inputVector(i));
    for (int j = 0; j < numberOfChromophores; ++j)
    {
      A_data.push_back(EndmemberMatrix(i, j));
    }
  }

  const double* Adat = A_data.data();
  const double* Bdat = B_data.data();

  vigra::Matrix<double> A(Shape2(numberOfWavelengths, numberOfChromophores), Adat);
  vigra::Matrix<double> b(Shape2(numberOfWavelengths, 1), Bdat);
  vigra::Matrix<double> x(Shape2(numberOfChromophores, 1));

  // minimize (A*x-b)^2  s.t. x>=0 using least angle regression (LARS algorithm)
  if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS == algorithmIndex)
    nonnegativeLeastSquares(A, b, x);

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::GOLDFARB == algorithmIndex)
  {
    vigra::Matrix<double> eye(identityMatrix<double>(numberOfChromophores)),
      zeros(Shape2(numberOfChromophores, 1)),
      empty,
      U = transpose(A)*A,
      v = -transpose(A)*b;
    x = 0;

    // minimize (A*x-b)^2  s.t. x>=0 using the Goldfarb-Idnani algorithm
    quadraticProgramming(U, v, empty, empty, eye, zeros, x);
  }

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED == algorithmIndex)
  {
    std::vector<double> weightsvec;
    for (int i = 0; i < m_Weight.size(); ++i)
    {
      double value = double (m_Weight[i]) / 100.0;
      weightsvec.push_back(value);
    }

    const double* weightsdat = weightsvec.data();
    vigra::Matrix<double> weigths(Shape2(numberOfWavelengths, 1), weightsdat);

    vigra::linalg::weightedLeastSquares(A, b, weigths, x);
  }

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LS == algorithmIndex)
    linearSolve(A, b, x);

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::vigratest == algorithmIndex)
    linearSolve(A, b, x);
    //mitkThrow() << "nothing implemented";

  else
    mitkThrow() << "404 VIGRA ALGORITHM NOT FOUND";

  for (int k = 0; k < numberOfChromophores; ++k)
  {
    float test = x(k, 0);
    resultVector[k] = test;
  }

  return resultVector;
}
