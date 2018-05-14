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

#include <mitkGeneralizedLinearModel.h>

#include <mitkDistModels.h>
#include <mitkLinkModels.h>

#include <v3p_netlib.h>
#include <vnl/algo/vnl_qr.h>
#include <mitkLogMacros.h>
#include <algorithm>
#include <limits>

static void _UpdateXMatrix(const vnl_matrix<double> &xData, bool addConstant, v3p_netlib_doublereal *x);
static void _UpdatePermXMatrix(const vnl_matrix<double> &xData, bool addConstant, const vnl_vector<unsigned int> &permutation, vnl_matrix<double> &x);
static void _InitMuEta(mitk::DistSimpleBinominal *dist, mitk::LogItLinking *link, const vnl_vector<double> &yData, vnl_vector<double> &mu, vnl_vector<double> &eta);
static void _FinalizeBVector(vnl_vector<double> &b, vnl_vector<unsigned int> &perm, int cols);


double mitk::GeneralizedLinearModel::Predict( const vnl_vector<double> &c)
{
  LogItLinking link;
  double mu = 0;
  int cols = m_B.size();
  for (int i = 0; i < cols; ++i)
  {
    if (!m_AddConstantColumn)
      mu += c(i)* m_B(i);
    else if ( i == 0)
      mu += 1* m_B(i);
    else
      mu += c(i-1)*m_B(i);
  }
  return link.InverseLink(mu);
}

vnl_vector<double> mitk::GeneralizedLinearModel::Predict(const vnl_matrix<double> &x)
{
  LogItLinking link;
  vnl_vector<double> mu(x.rows());
  int cols = m_B.size();
  for (unsigned int r = 0 ; r < mu.size(); ++r)
  {
    mu(r) = 0;
    for (int c = 0; c < cols; ++c)
    {
      if (!m_AddConstantColumn)
        mu(r) += x(r,c)*m_B(c);
      else if ( c == 0)
        mu(r) += m_B(c);
      else
        mu(r) += x(r,c-1)*m_B(c);
    }
    mu(r) = link.InverseLink(mu(r));
  }
  return mu;
}

vnl_vector<double> mitk::GeneralizedLinearModel::B()
{
  return m_B;
}

vnl_vector<double> mitk::GeneralizedLinearModel::ExpMu(const vnl_matrix<double> &x)
{
  LogItLinking link;
  vnl_vector<double> mu(x.rows());
  int cols = m_B.size();
  for (unsigned int r = 0 ; r < mu.size(); ++r)
  {
    mu(r) = 0;
    for (int c = 0; c < cols; ++c)
    {
      if (!m_AddConstantColumn)
        mu(r) += x(r,c)*m_B(c);
      else if ( c == 0)
        mu(r) += m_B(c);
      else
        mu(r) += x(r,c-1)*m_B(c);
    }
    mu(r) = exp(-mu(r));
  }
  return mu;
}

mitk::GeneralizedLinearModel::GeneralizedLinearModel(const vnl_matrix<double> &xData, const vnl_vector<double> &yData, bool addConstantColumn) :
  m_AddConstantColumn(addConstantColumn)
{
  EstimatePermutation(xData);

  DistSimpleBinominal dist;
  LogItLinking link;
  vnl_matrix<double> x;
  int rows = xData.rows();
  int cols = m_Permutation.size();
  vnl_vector<double> mu(rows);
  vnl_vector<double> eta(rows);
  vnl_vector<double> weightedY(rows);
  vnl_matrix<double> weightedX(rows, cols);
  vnl_vector<double> oldB(cols);

  _UpdatePermXMatrix(xData, m_AddConstantColumn, m_Permutation, x);
  _InitMuEta(&dist, &link, yData, mu, eta);

  int iter = 0;
  int iterLimit = 100;
  double sqrtEps = sqrt(std::numeric_limits<double>::epsilon());
  double convertCriterion =1e-6;

  m_B.set_size(m_Permutation.size());
  m_B.fill(0);

  while (iter <= iterLimit)
  {
    ++iter;

    oldB = m_B;
    // Do Row-wise operation. No Vector operation at this point.
    for (int r = 0; r<rows; ++r)
    {
      double deta = link.DLink(mu(r));
      double zBuffer = eta(r) + (yData(r) - mu(r))*deta;
      double sqrtWeight = 1 / (std::abs(deta) * dist.SqrtVariance(mu(r)));

      weightedY(r) = zBuffer * sqrtWeight;
      for (int c=0; c<cols; ++c)
      {
        weightedX(r,c) = x(r,c) * sqrtWeight;
      }
    }
    vnl_qr<double> qr(weightedX);
    m_B = qr.solve(weightedY);
    eta = x * m_B;
    for (int r = 0; r < rows; ++r)
    {
      mu(r) = link.InverseLink(eta(r));
    }

    bool stayInLoop = false;
    for(int c= 0; c < cols; ++c)
    {
      stayInLoop |= std::abs( m_B(c) - oldB(c)) > convertCriterion * std::max(sqrtEps, std::abs(oldB(c)));
    }
    if (!stayInLoop)
      break;
  }
  _FinalizeBVector(m_B, m_Permutation, xData.cols());
}

void mitk::GeneralizedLinearModel::EstimatePermutation(const vnl_matrix<double> &xData)
{
  v3p_netlib_integer rows = xData.rows();
  v3p_netlib_integer cols = xData.cols();

  if (m_AddConstantColumn)
    ++cols;

  v3p_netlib_doublereal *x = new v3p_netlib_doublereal[rows* cols];
  _UpdateXMatrix(xData, m_AddConstantColumn, x);
  v3p_netlib_doublereal *qraux = new v3p_netlib_doublereal[cols];
  v3p_netlib_integer *jpvt = new v3p_netlib_integer[cols];
  std::fill_n(jpvt,cols,0);
  v3p_netlib_doublereal *work = new v3p_netlib_doublereal[cols];
  std::fill_n(work,cols,0);
  v3p_netlib_integer job = 16;

  // Make a call to Lapack-DQRDC which does QR with permutation
  // Permutation is saved in JPVT.
  v3p_netlib_dqrdc_(x, &rows, &rows, &cols, qraux, jpvt, work, &job);

  double limit = std::abs(x[0]) * std::max(cols, rows) * std::numeric_limits<double>::epsilon();
  // Calculate the rank of the matrix
  int m_Rank = 0;
  for (int i = 0; i <cols; ++i)
  {
    m_Rank += (std::abs(x[i*rows + i]) > limit) ? 1 : 0;
  }
  // Create a permutation vector
  m_Permutation.set_size(m_Rank);
  for (int i = 0; i < m_Rank; ++i)
  {
    m_Permutation(i) = jpvt[i]-1;
  }

  delete[] x;
  delete[] qraux;
  delete[] jpvt;
  delete[] work;
}

// Copy a vnl-matrix to an c-array with row-wise representation.
// Adds a constant column if required.
static void _UpdateXMatrix(const vnl_matrix<double> &xData, bool addConstant, v3p_netlib_doublereal *x)
{
  v3p_netlib_integer rows = xData.rows();
  v3p_netlib_integer cols = xData.cols();
  if (addConstant)
    ++cols;

  for (int r=0; r < rows; ++r)
  {
    for (int c=0; c <cols; ++c)
    {
      if (!addConstant)
      {
        x[c*rows + r] = xData(r,c);
      } else if (c == 0)
      {
        x[c*rows + r] = 1.0;
      } else
      {
        x[c*rows + r] = xData(r, c-1);
      }
    }
  }
}

// Fills the value of the xData-matrix into the x-matrix. Adds a constant
// column if required. Permutes the rows corresponding to the permutation vector.
static void _UpdatePermXMatrix(const vnl_matrix<double> &xData, bool addConstant, const vnl_vector<unsigned int> &permutation, vnl_matrix<double> &x)
{
  int rows = xData.rows();
  int cols = permutation.size();
  x.set_size(rows, cols);
  for (int r=0; r < rows; ++r)
  {
    for (int c=0; c<cols; ++c)
    {
      unsigned int newCol = permutation(c);
      if (!addConstant)
      {
        x(r, c) = xData(r,newCol);
      } else if (newCol == 0)
      {
        x(r, c) = 1.0;
      } else
      {
        x(r, c) = xData(r, newCol-1);
      }
    }
  }
}

// Initialize mu and eta by calling the corresponding
// link and distribution functions on each row
static void _InitMuEta(mitk::DistSimpleBinominal *dist, mitk::LogItLinking *link, const vnl_vector<double> &yData, vnl_vector<double> &mu, vnl_vector<double> &eta)
{
  int rows = yData.size();
  mu.set_size(rows);
  eta.set_size(rows);
  for (int r = 0; r < rows; ++r)
  {
    mu(r) = dist->Init(yData(r));
    eta(r) = link->Link(mu(r));
  }
}

// Inverts the permutation on a given b-vector.
// Necessary to get a b-vector that match the original data
static void _FinalizeBVector(vnl_vector<double> &b, vnl_vector<unsigned int> &perm, int cols)
{
  vnl_vector<double> tempB(cols+1);
  tempB.fill(0);
  for (unsigned int c = 0; c < perm.size(); ++c)
  {
    tempB(perm(c)) = b(c);
  }
  b = tempB;
}
