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

#include "itkBiExpFitFunctor.h"
#include <math.h>
#include <iostream>
#include <iomanip>

void itk::BiExpFitFunctor::operator()(vnl_matrix<double> & newSignal,const vnl_matrix<double> & SignalMatrix, const double & S0)
{

  vnl_vector<double> initalGuess(3);
  // initialize Least Squres Function
  // SignalMatrix.cols() defines the number of shells points
  lestSquaresFunction model(SignalMatrix.cols());
  model.set_bvalues(m_BValueList);// set BValue Vector e.g.: [1000, 2000, 3000] <- shell b Values

  // initialize Levenberg Marquardt
  vnl_levenberg_marquardt minimizer(model);
  minimizer.set_max_function_evals(1000);   // Iterations
  minimizer.set_f_tolerance(1e-10);        // Function tolerance

  // for each Direction calculate LSF Coeffs ADC & AKC
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); i++)
  {
    model.set_measurements(SignalMatrix.get_row(i));
    model.set_reference_measurement(S0);

    initalGuess.put(0, 0.f); // ADC_slow
    initalGuess.put(1, 0.009f); // ADC_fast
    initalGuess.put(2, 0.7f); // lambda

    // start Levenberg-Marquardt
    minimizer.minimize_without_gradient(initalGuess);

    const double & ADC_slow = initalGuess.get(0);
    const double & ADC_fast = initalGuess.get(1);
    const double & lambda = initalGuess(2);

    newSignal.put(i, 0, S0 * (lambda * std::exp(-m_TargetBvalue * ADC_slow) + (1-lambda)* std::exp(-m_TargetBvalue * ADC_fast)));
    newSignal.put(i, 1, minimizer.get_end_error()); // RMS Error

    //OUTPUT FOR EVALUATION
    /*std::cout << std::scientific << std::setprecision(5)
              << ADC_slow   << ","                        // lambda
              << ADC_fast   << ","                        // alpha
              << lambda     << ","                        // lambda
              << S0         << ","                        // S0 value
              << minimizer.get_end_error() << ",";      // End error
    for(unsigned int j = 0; j < SignalMatrix.get_row(i).size(); j++ ){
      std::cout << std::scientific << std::setprecision(5) << SignalMatrix.get_row(i)[j];    // S_n Values corresponding to shell 1 to shell n
      if(j != SignalMatrix.get_row(i).size()-1) std::cout << ",";
    }
    std::cout << std::endl;*/
  }

}
