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

#include <mitkAnisotropicRegistrationCommon.h>
#include <vtkPoints.h>

mitk::AnisotropicRegistrationCommon::WeightMatrix
mitk::AnisotropicRegistrationCommon::CalculateWeightMatrix( const WeightMatrix &sigma_X,
                                                            const WeightMatrix &sigma_Y )
{
  WeightMatrix returnValue;

  WeightMatrix sum =  sigma_X + sigma_Y;
  vnl_svd<double> svd(sum.GetVnlMatrix());

  WeightMatrix diag; diag.Fill(0.0);
  diag[0][0] = 1.0 / sqrt(svd.W(0));
  diag[1][1] = 1.0 / sqrt(svd.W(1));
  diag[2][2] = 1.0 / sqrt(svd.W(2));

  WeightMatrix V; //convert vnl matrix to itk matrix...
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
        V[i][j] = svd.V()[i][j];

  //add weighting matrix for point j1 (corresponding to identity transform)
  returnValue = V * diag * V.GetTranspose();

  return returnValue;
}

void mitk::AnisotropicRegistrationCommon::TransformPoints( vtkPoints *src,
                                                           vtkPoints *dst,
                                                           const Rotation &rotation,
                                                           const Translation &translation )
{
#pragma omp parallel for
  for ( vtkIdType i = 0; i < src->GetNumberOfPoints(); ++i )
  {
    double p_in[3];
    double p_out[3];
    src->GetPoint(i,p_in);

    for ( unsigned int j = 0; j < 3; ++j )
    {
      p_out[j] = p_in[0] * rotation[j][0] +
                 p_in[1] * rotation[j][1] +
                 p_in[2] * rotation[j][2] +
                 translation[j];
    }

    dst->SetPoint(i,p_out);
  }
}

void mitk::AnisotropicRegistrationCommon::PropagateMatrices( const MatrixList &src,
                                                             MatrixList &dst,
                                                             const Rotation &rotation )
{

 const vnl_matrix_fixed < double, 3, 3 > rotationT = rotation.GetTranspose();

#pragma omp parallel for
  for ( size_t i = 0; i < src.size(); ++i )
  {
    dst[i] = rotation * src[i] * rotationT;
  }
}

vtkSmartPointer<vtkPoints> mitk::AnisotropicRegistrationCommon::GetVTKPoints(mitk::PointSet *p)
{
  vtkSmartPointer<vtkPoints> out = vtkSmartPointer<vtkPoints>::New();
  out->SetNumberOfPoints(p->GetSize());

  for ( vtkIdType i = 0; i < p->GetSize(); ++i )
  {
    out->SetPoint(i,p->GetPoint(i)[0],p->GetPoint(i)[1],p->GetPoint(i)[2]);
  }

  return out;
}

mitk::PointSet::Pointer mitk::AnisotropicRegistrationCommon::GetMITKPoints(vtkPoints *p)
{
  mitk::PointSet::Pointer out = mitk::PointSet::New();

  for ( int i = 0; i < p->GetNumberOfPoints(); ++i )
  {
    mitk::Point3D point;
    double vtkPoint[3];
    p->GetPoint(i,vtkPoint);

    point[0] = vtkPoint[0];
    point[1] = vtkPoint[1];
    point[2] = vtkPoint[2];

    out->InsertPoint(i,point);
  }
  return out;
}
