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
#include "mitkKalmanFilter.h"

mitk::KalmanFilter::KalmanFilter():
m_IsInitialized( false )
{
  m_cvKalman = cvCreateKalman( 2, 1, 0 );
  m_StateVector = cvCreateMat( 2, 1, CV_32FC1);

}

mitk::KalmanFilter::~KalmanFilter()
{
}

float mitk::KalmanFilter::ProcessValue( float input )
{
  if( m_IsInitialized )
  {
    //// predicted value
    //const CvMat* y_k = cvKalmanPredict( m_cvKalman, 0 );

    // measured value
    CvMat* z_k = cvCreateMat( 1, 1, CV_32FC1 );
    float Z[] ={ (float)input };
    cvInitMatHeader( z_k, 1, 1, CV_32FC1, Z );

      float test123 = m_cvKalman->measurement_matrix->data.fl[0];
  float test234 = m_cvKalman->measurement_matrix->data.fl[1];

    // zk=H•xk+vk,
    cvMatMulAdd( m_cvKalman->measurement_matrix, m_StateVector /* = x_k */, z_k, z_k );

    // adjust Kalman filter state
    cvKalmanCorrect( m_cvKalman, z_k );

    // apply the transition matrix A and apply "process noise" w_k
    // xk=A•xk-1+wk
    cvMatMulAdd( m_cvKalman->transition_matrix
      , m_StateVector /* = x_k */
      , m_cvKalman->process_noise_cov
      , m_StateVector /* = x_k */ );
  }

  return 0;
}

void mitk::KalmanFilter::SetMeasurementNoise( float measurementNoise )
{
  // R - measurement noise covariance matrix
  //   , represents position variance
  float r = measurementNoise;
  float R[]= { (float)r };
  cvInitMatHeader( m_cvKalman->measurement_noise_cov, 1, 1, CV_32FC1, R );
}

void mitk::KalmanFilter::SetProcessNoise( float processNoise )
{
  ///// SYSTEM NOISE ////
  // Q - process noise covariance matrix Q
  float q  = processNoise;
  float Q[] = { (float)q, (float)0 };
  cvInitMatHeader( m_cvKalman->process_noise_cov, 2, 1, CV_32FC1, Q );
}

void mitk::KalmanFilter::ResetFilter()
{
  // Initialize Standard Kalman Filter Variables: A,B,H,Q and R

  // A - (or F) relates the state at previous time step k-1 to the state at current time step k
  // B - control matrix, it is not used if there is no control
  // H - measurement matrix,
  // Q - process noise covariance matrix, constant or variable,
  // R - measurement noise covariance matrix, constant or variable

  // xk=A•xk-1+B•uk+wk
  // zk=H•xk+vk,
  // where:
  // xk (xk-1) - state of the system at the moment k (k-1)
  // zk - measurement of the system state at the moment k
  // uk - external control applied at the moment k
  // wk and vk are normally-distributed process and measurement noise, respectively:
  // p(w) ~ N(0,Q)
  // p(v) ~ N(0,R),

  // B - Optional: initialize control matrix B here

  /*CvMat* cvInitMatHeader( CvMat* mat
  , int rows
  , int cols
  , int type
  , void* data=NULL
  , int step=CV_AUTOSTEP );*/

  // reset state
  cvSetIdentity( m_StateVector );

  //// OBSERVATIONS ////
  // measurement matrix H
  float H[] = { (float)1.0, (float)12.0};
  cvInitMatHeader(m_cvKalman->measurement_matrix, 1, 2, CV_32FC1, H );

    float test11 = m_cvKalman->measurement_matrix->data.fl[0];
  float test22 = m_cvKalman->measurement_matrix->data.fl[1];

  memcpy(m_cvKalman->measurement_matrix->data.fl, H, sizeof(H));

  float test1 = m_cvKalman->measurement_matrix->data.fl[0];
  float test2 = m_cvKalman->measurement_matrix->data.fl[1];

  //// PREDICTION MATRICES ////

  // P - initial prediction covariance matrix (posteriori error estimate covariance matrix)
  float P[] = { (float)1, (float)0 };
  cvInitMatHeader(m_cvKalman->error_cov_post, 2, 1, CV_32FC1, P );

  // A - relates the time states
  double dt = 1;
  double A[] = { (float)1,(float)dt,
                 (float)0, (float)1, };
  cvInitMatHeader(m_cvKalman->transition_matrix, 2, 2, CV_32FC1, A);

  memcpy(m_cvKalman->transition_matrix->data.fl, A, sizeof(A));
  // what about kalman->state_post -> corrected state (x(k)=x'(k)+K(k)*(z(k)-H*x'(k))
  CvRNG rng = cvRNG(-1);

  //// init random prediction state
  //cvRandArr( &rng
  //  , m_cvKalman->state_post
  //  , CV_RAND_NORMAL
  //  , cvRealScalar(0)
  //  , cvRealScalar(0.1) );

  m_IsInitialized = true;

}
