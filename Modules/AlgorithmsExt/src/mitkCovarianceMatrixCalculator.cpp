/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCovarianceMatrixCalculator.h"
#include <mitkExceptionMacro.h>
#include <mitkSurface.h>
#include <vtkCell.h>
#include <vtkCellLinks.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>

// forward declarations of private functions
static vtkIdList *GetNeighboursOfPoint(unsigned int index, vtkPolyData *polydata);

static vtkIdList *CalculatePCAonPointNeighboursForNormalVector(int index,
                                                               double normal[3],
                                                               itk::Matrix<double, 3, 3> &mat,
                                                               double curVertex[3],
                                                               std::vector<mitk::Point3D> &pointList,
                                                               vtkPolyData *polyData);

static itk::Matrix<double, 3, 3> ComputeCovarianceMatrix(itk::Matrix<double, 3, 3> &axes,
                                                         double sigma[3],
                                                         double normalizationValue);
namespace mitk
{
  /** \brief Pimpl to hold the private data in the CovarianceMatrixCalculator.*/
  struct CovarianceMatrixCalculatorData
  {
    vtkPolyDataNormals *m_PolyDataNormals;
    vtkPolyData *m_PolyData;
    Surface *m_Input;
    double m_VoronoiScalingFactor;
    bool m_EnableNormalization;
    double m_MeanVariance;

    CovarianceMatrixCalculatorData()
      : m_PolyDataNormals(vtkPolyDataNormals::New()),
        m_PolyData(nullptr),
        m_Input(nullptr),
        m_VoronoiScalingFactor(1.0),
        m_EnableNormalization(false),
        m_MeanVariance(0.0)
    {
      m_PolyDataNormals->SplittingOff();
    }

    ~CovarianceMatrixCalculatorData()
    {
      if (m_PolyDataNormals)
        m_PolyDataNormals->Delete();
    }
  };
}

mitk::CovarianceMatrixCalculator::CovarianceMatrixCalculator() : d(new CovarianceMatrixCalculatorData())
{
}

mitk::CovarianceMatrixCalculator::~CovarianceMatrixCalculator()
{
  delete d;
}

void mitk::CovarianceMatrixCalculator::SetVoronoiScalingFator(const double factor)
{
  d->m_VoronoiScalingFactor = factor;
}

void mitk::CovarianceMatrixCalculator::EnableNormalization(bool state)
{
  d->m_EnableNormalization = state;
}

double mitk::CovarianceMatrixCalculator::GetMeanVariance() const
{
  return d->m_MeanVariance;
}

const mitk::CovarianceMatrixCalculator::CovarianceMatrixList &mitk::CovarianceMatrixCalculator::GetCovarianceMatrices()
  const
{
  return m_CovarianceMatrixList;
}

void mitk::CovarianceMatrixCalculator::SetInputSurface(Surface *input)
{
  d->m_Input = input;
}

void mitk::CovarianceMatrixCalculator::ComputeCovarianceMatrices()
{
  double normalizationValue = -1.0;
  vtkDataArray *normals = nullptr;
  d->m_MeanVariance = 0.0;

  if (!d->m_Input)
    mitkThrow() << "No input surface was set in mitk::CovarianceMatrixCalculator";

  d->m_PolyData = d->m_Input->GetVtkPolyData();

  // Optional normal calculation can be disabled to use the normals
  // of the surface:
  // normals = d->m_PolyData->GetPointData()->GetNormals();
  //// compute surface normals if the surface has no normals
  // if ( normals == nullptr )
  //{
  d->m_PolyDataNormals->SetInputData(d->m_PolyData);
  d->m_PolyDataNormals->Update();
  normals = d->m_PolyDataNormals->GetOutput()->GetPointData()->GetNormals();
  //}

  if (d->m_EnableNormalization)
    normalizationValue = 1.5;

  // clear the matrixlist
  m_CovarianceMatrixList.clear();
  // allocate memory if required
  if (d->m_PolyData->GetNumberOfPoints() > (vtkIdType)m_CovarianceMatrixList.capacity())
    m_CovarianceMatrixList.reserve(d->m_PolyData->GetNumberOfPoints());

  for (vtkIdType i = 0; i < d->m_PolyData->GetNumberOfPoints(); ++i)
  {
    Vertex normal;
    Vertex currentVertex;
    Vertex variances = {0.0, 0.0, 0.0};
    CovarianceMatrix mat;
    mat.Fill(0.0);

    normals->GetTuple(i, normal);
    d->m_PolyData->GetPoint(i, currentVertex);

    ComputeOrthonormalCoordinateSystem(i, normal, mat, variances, currentVertex);

    // use prefactor for sigma along surface
    variances[0] = (d->m_VoronoiScalingFactor * variances[0]);
    variances[1] = (d->m_VoronoiScalingFactor * variances[1]);
    variances[2] = (d->m_VoronoiScalingFactor * variances[2]);

    d->m_MeanVariance += (variances[0] + variances[1] + variances[2]);
    // compute the covariance matrix and save it
    const CovarianceMatrix covarianceMatrix = ComputeCovarianceMatrix(mat, variances, normalizationValue);
    m_CovarianceMatrixList.push_back(covarianceMatrix);
  }

  if (d->m_EnableNormalization)
    d->m_MeanVariance = normalizationValue / 3.0;
  else
    d->m_MeanVariance /= (3.0 * (double)d->m_PolyData->GetNumberOfPoints());

  // reset input
  d->m_PolyData = nullptr;
  d->m_Input = nullptr;
}

// Get a list with the id's of all surrounding conected vertices
// to the current vertex at the given index in the polydata
vtkIdList *GetNeighboursOfPoint(unsigned int index, vtkPolyData *polydata)
{
  vtkIdList *cellIds = vtkIdList::New();
  vtkIdList *result = vtkIdList::New();
  polydata->GetPointCells(index, cellIds);

  for (vtkIdType j = 0; j < cellIds->GetNumberOfIds(); j++)
  {
    vtkIdList *newPoints = polydata->GetCell(cellIds->GetId(j))->GetPointIds();
    for (vtkIdType k = 0; k < newPoints->GetNumberOfIds(); k++)
    {
      // if point has not yet been inserted add id
      if (result->IsId(newPoints->GetId(k)) == -1)
      {
        result->InsertNextId(newPoints->GetId(k));
      }
    }
  }
  cellIds->Delete();
  return result;
}

// Computes a primary component analysis of the surounding vertices
// of the verex at the current index.
vtkIdList *CalculatePCAonPointNeighboursForNormalVector(int index,
                                                        double normal[3],
                                                        itk::Matrix<double, 3, 3> &mat,
                                                        double curVertex[3],
                                                        std::vector<mitk::Point3D> &pointList,
                                                        vtkPolyData *polyData)
{
  typedef std::vector<mitk::Point3D> VectorType;
  typedef VectorType::const_iterator ConstPointIterator;
  typedef double Vertex[3];

  Vertex mean = {0.0, 0.0, 0.0};
  Vertex tmp = {0.0, 0.0, 0.0};
  vtkIdList *neighbourPoints = GetNeighboursOfPoint(index, polyData);
  const vtkIdType size = neighbourPoints->GetNumberOfIds();

  // reserve memory for all neighbours
  pointList.reserve(size);

  // project neighbours on plane given by normal
  // and compute mean
  for (vtkIdType i = 0; i < size; ++i)
  {
    mitk::Point3D p;
    Vertex resultPoint;

    polyData->GetPoint((neighbourPoints->GetId(i)), tmp);

    vtkPlane::GeneralizedProjectPoint(tmp, curVertex, normal, resultPoint);

    p[0] = resultPoint[0];
    p[1] = resultPoint[1];
    p[2] = resultPoint[2];

    mean[0] += p[0];
    mean[1] += p[1];
    mean[2] += p[2];

    pointList.push_back(p);
  }

  mean[0] /= (double)size;
  mean[1] /= (double)size;
  mean[2] /= (double)size;

  // compute the covariances with matrix multiplication
  for (ConstPointIterator it = pointList.begin(); it != pointList.end(); ++it)
  {
    tmp[0] = ((*it)[0] - mean[0]);
    tmp[1] = ((*it)[1] - mean[1]);
    tmp[2] = ((*it)[2] - mean[2]);

    // on diagonal elements
    mat[0][0] += tmp[0] * tmp[0];
    mat[1][1] += tmp[1] * tmp[1];
    mat[2][2] += tmp[2] * tmp[2];

    // of diagonal elements
    mat[1][0] += tmp[0] * tmp[1];
    mat[2][0] += tmp[0] * tmp[2];
    mat[2][1] += tmp[1] * tmp[2];
  }

  // copy upper triangle to lower triangle,
  // we got a symetric matrix
  mat[0][1] = mat[1][0];
  mat[0][2] = mat[2][0];
  mat[1][2] = mat[2][1];

  // variance
  mat /= (size - 1);

  vnl_svd<double> svd(mat.GetVnlMatrix());

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      mat[i][j] = svd.U()[j][i];

  return neighbourPoints;
}

// Computes an orthonormal system for a vertex with it's surrounding neighbours.
void mitk::CovarianceMatrixCalculator::ComputeOrthonormalCoordinateSystem(
  const int index, Vertex normal, CovarianceMatrix &axes, Vertex variances, Vertex curVertex)
{
  typedef std::vector<mitk::Point3D> VectorType;
  typedef VectorType::const_iterator ConstPointIterator;
  VectorType projectedPoints;

  Vertex meanValues = {0.0, 0.0, 0.0};
  // project neighbours to new coordinate system and get principal axes
  vtkIdList *neighbourPoints =
    CalculatePCAonPointNeighboursForNormalVector(index, normal, axes, curVertex, projectedPoints, d->m_PolyData);

  // Set the normal as the third principal axis
  axes[2][0] = normal[0];
  axes[2][1] = normal[1];
  axes[2][2] = normal[2];

  for (vtkIdType i = 0; i < neighbourPoints->GetNumberOfIds(); ++i)
  {
    mitk::Point3D projectedPoint;
    Vertex curNeighbour;
    d->m_PolyData->GetPoint(neighbourPoints->GetId(i), curNeighbour);

    curNeighbour[0] = curNeighbour[0] - curVertex[0];
    curNeighbour[1] = curNeighbour[1] - curVertex[1];
    curNeighbour[2] = curNeighbour[2] - curVertex[2];

    for (int k = 0; k < 3; ++k)
    {
      projectedPoint[k] = axes[k][0] * curNeighbour[0] + axes[k][1] * curNeighbour[1] + axes[k][2] * curNeighbour[2];

      meanValues[k] += projectedPoint[k];
    }
    // reuse the allocated vector from the PCA on the point neighbours
    projectedPoints[i] = projectedPoint;
  }

  meanValues[0] /= (double)projectedPoints.size();
  meanValues[1] /= (double)projectedPoints.size();
  meanValues[2] /= (double)projectedPoints.size();

  // compute variances along new axes
  for (ConstPointIterator it = projectedPoints.begin(); it != projectedPoints.end(); ++it)
  {
    const mitk::Point3D &p = *it;

    variances[0] += (p[0] - meanValues[0]) * (p[0] - meanValues[0]);
    variances[1] += (p[1] - meanValues[1]) * (p[1] - meanValues[1]);
    variances[2] += (p[2] - meanValues[2]) * (p[2] - meanValues[2]);
  }

  variances[0] /= (double)(projectedPoints.size() - 1);
  variances[1] /= (double)(projectedPoints.size() - 1);
  variances[2] /= (double)(projectedPoints.size() - 1);

  // clean up
  neighbourPoints->Delete();
}

// Sorts the axes of the computed orthonormal system based on
// the eigenvalues in a descending order
itk::Matrix<double, 3, 3> ComputeCovarianceMatrix(itk::Matrix<double, 3, 3> &axes,
                                                  double sigma[3],
                                                  double normalizationValue)
{
  unsigned int idxMax, idxMin, idxBetween;
  itk::Matrix<double, 3, 3> returnValue;
  itk::Matrix<double, 3, 3> V;
  itk::Matrix<double, 3, 3> diagMatrix;
  diagMatrix.Fill(0.0);

  if (sigma[0] >= sigma[1] && sigma[0] >= sigma[2])
  {
    idxMax = 0;
    if (sigma[1] >= sigma[2])
    {
      idxBetween = 1;
      idxMin = 2;
    }
    else
    {
      idxBetween = 2;
      idxMin = 1;
    }
  }
  else if (sigma[1] >= sigma[0] && sigma[1] >= sigma[2])
  {
    idxMax = 1;
    if (sigma[0] >= sigma[2])
    {
      idxBetween = 0;
      idxMin = 2;
    }
    else
    {
      idxBetween = 2;
      idxMin = 0;
    }
  }
  else // index 2 corresponds to largest sigma
  {
    idxMax = 2;
    if (sigma[0] >= sigma[1])
    {
      idxBetween = 0;
      idxMin = 1;
    }
    else
    {
      idxBetween = 1;
      idxMin = 0;
    }
  }

  V[0][0] = axes[idxMax][0];
  V[1][0] = axes[idxMax][1];
  V[2][0] = axes[idxMax][2];
  V[0][1] = axes[idxBetween][0];
  V[1][1] = axes[idxBetween][1];
  V[2][1] = axes[idxBetween][2];
  V[0][2] = axes[idxMin][0];
  V[1][2] = axes[idxMin][1];
  V[2][2] = axes[idxMin][2];

  diagMatrix[0][0] = sigma[idxMax];
  diagMatrix[1][1] = sigma[idxBetween];
  diagMatrix[2][2] = sigma[idxMin];

  returnValue = V * diagMatrix * V.GetTranspose();

  if (normalizationValue > 0.0)
  {
    double trace = returnValue[0][0] + returnValue[1][1] + returnValue[2][2];
    returnValue *= (normalizationValue / trace);
  }

  return returnValue;
}
