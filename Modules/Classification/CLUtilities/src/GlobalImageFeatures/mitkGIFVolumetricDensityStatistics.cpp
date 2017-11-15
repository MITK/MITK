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

#include <mitkGIFVolumetricDensityStatistics.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkLabelGeometryImageFilter.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageMarchingCubes.h>
#include <vtkMassProperties.h>
#include <vtkDelaunay3D.h>
#include <vtkOBBTree.h>
#include <vtkTessellatorFilter.h>
#include <vtkTriangleFilter.h>

#include <vtkXMLPolyDataWriter.h>
#include <vtkSTLWriter.h>

#include <vtkGeometryFilter.h>

// STL
#include <sstream>
#include <vnl/vnl_math.h>

// OpenCV
#include <opencv2/opencv.hpp>

// Eigen
#include <Eigen/Dense>

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateVolumeDensityStatistic(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, double volume, mitk::GIFVolumetricDensityStatistics::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelGeometryImageFilter<MaskType, ImageType> FilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  itk::ImageRegionConstIteratorWithIndex<ImageType> imgA(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<ImageType> imgB(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<MaskType> maskA(maskImage, maskImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<MaskType> maskB(maskImage, maskImage->GetLargestPossibleRegion());

  double moranA = 0;
  double moranB = 0;
  double geary = 0;
  double Nv = 0;
  double w_ij = 0;
  double mean = 0;

  ImageType::PointType pointA;
  ImageType::PointType pointB;

  while (!imgA.IsAtEnd())
  {
    if (maskA.Get() > 0)
    {
      Nv += 1;
      mean += imgA.Get();
    }
    ++imgA;
    ++maskA;
  }
  mean /= Nv;
  imgA.GoToBegin();
  maskA.GoToBegin();
  while (!imgA.IsAtEnd())
  {
    if (maskA.Get() > 0)
    {
      imgB.GoToBegin();
      maskB.GoToBegin();
      while (!imgB.IsAtEnd())
      {
        if ((imgA.GetIndex() == imgB.GetIndex()) ||
          (maskB.Get() < 1))
        {
          ++imgB;
          ++maskB;
          continue;
        }
        itkImage->TransformIndexToPhysicalPoint(maskA.GetIndex(), pointA);
        itkImage->TransformIndexToPhysicalPoint(maskB.GetIndex(), pointB);

        double w = 1 / pointA.EuclideanDistanceTo(pointB);
        moranA += w*(imgA.Get() - mean)* (imgB.Get() - mean);
        geary += w * (imgA.Get() - imgB.Get()) * (imgA.Get() - imgB.Get());

        w_ij += w;

        ++imgB;
        ++maskB;
      }
      moranB += (imgA.Get() - mean)* (imgA.Get() - mean);
    }
    ++imgA;
    ++maskA;
  }

  featureList.push_back(std::make_pair("Morphological Density Volume Integrated Intensity", volume* mean));
  featureList.push_back(std::make_pair("Morphological Density Volume Moran's I Index", Nv / w_ij * moranA / moranB));
  featureList.push_back(std::make_pair("Morphological Density Volume Geary's C measure", ( Nv -1 ) / 2 / w_ij * geary/ moranB));
}

void calculateMOBB(vtkPointSet *pointset, double &volume, double &surface)
{
  auto cell = pointset->GetCell(0);
  volume = 10000000000000000000;

  for (int cellID = 0; cellID < pointset->GetNumberOfCells(); ++cellID)
  {
    auto cell = pointset->GetCell(cellID);

    for (int edgeID = 0; edgeID < 3; ++edgeID)
    {
      auto edge = cell->GetEdge(edgeID);

      double pA[3], pB[3];
      double pAA[3], pBB[3];

      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      transform->PostMultiply();
      pointset->GetPoint(edge->GetPointId(0), pA);
      pointset->GetPoint(edge->GetPointId(1), pB);

      double angleZ = std::atan2((- pA[2] + pB[2]) ,(pA[1] - pB[1]));
      angleZ *= 180 / vnl_math::pi;
      if (pA[2] == pB[2])
        angleZ = 0;

      transform->RotateX(angleZ);
      transform->TransformPoint(pA, pAA);
      transform->TransformPoint(pB, pBB);

      double angleY = std::atan2((pAA[1] -pBB[1]) ,-(pAA[0] - pBB[0]));
      angleY *= 180 / vnl_math::pi;
      if (pAA[1] == pBB[1])
        angleY = 0;
      transform->RotateZ(angleY);

      double p0[3];
      pointset->GetPoint(edge->GetPointId(0), p0);

      double curMinX = 1000000000000000;
      double curMaxX = -10000000000000000;
      double curMinY = 1000000000000000;
      double curMaxY = -10000000000000000;
      double curMinZ = 1000000000000000;
      double curMaxZ = -10000000000000000;
      for (int pointID = 0; pointID < pointset->GetNumberOfPoints(); ++pointID)
      {
        double p[3];
        double p2[3];
        pointset->GetPoint(pointID, p);
        p[0] -= p0[0]; p[1] -= p0[1]; p[2] -= p0[2];
        transform->TransformPoint(p, p2);

        curMinX = std::min<double>(p2[0], curMinX);
        curMaxX = std::max<double>(p2[0], curMaxX);
        curMinY = std::min<double>(p2[1], curMinY);
        curMaxY = std::max<double>(p2[1], curMaxY);
        curMinZ = std::min<double>(p2[2], curMinZ);
        curMaxZ = std::max<double>(p2[2], curMaxZ);

        //std::cout << pointID << " (" << p[0] << "|" << p[1] << "|" << p[2] << ") (" << p2[0] << "|" << p2[1] << "|" << p2[2] << ")" << std::endl;
      }

      if ((curMaxX - curMinX)*(curMaxY - curMinY)*(curMaxZ - curMinZ) < volume)
      {
        volume = (curMaxX - curMinX)*(curMaxY - curMinY)*(curMaxZ - curMinZ);
        surface = (curMaxX - curMinX)*(curMaxX - curMinX) + (curMaxY - curMinY)*(curMaxY - curMinY) + (curMaxZ - curMinZ)*(curMaxZ - curMinZ);
        surface *= 2;
      }


    }
  }
}

void calculateMEE(vtkPointSet *pointset, double &vol, double &surf, double tolerance=0.0000001)
{
  // Inspired by https://github.com/smdabdoub/ProkaryMetrics/blob/master/calc/fitting.py

  int numberOfPoints = pointset->GetNumberOfPoints();
  int dimension = 3;
  Eigen::MatrixXd points(3, numberOfPoints);
  Eigen::MatrixXd Q(3+1, numberOfPoints);
  double p[3];
  for (int i = 0; i < numberOfPoints; ++i)
  {
    pointset->GetPoint(i, p);
    points(0, i) = p[0];
    points(1, i) = p[1];
    points(2, i) = p[2];
    Q(0, i) = p[0];
    Q(1, i) = p[1];
    Q(2, i) = p[2];
    Q(3, i) = p[3];
  }

  int count = 1;
  double error = 1;
  Eigen::VectorXd u_vector(numberOfPoints);
  u_vector.fill(1.0 / numberOfPoints);
  Eigen::DiagonalMatrix<double, Eigen::Dynamic> u = u_vector.asDiagonal();
  Eigen::VectorXd ones(dimension + 1);
  ones.fill(1);
  Eigen::MatrixXd Ones = ones.asDiagonal();

  // Khachiyan Algorithm
  while (error > tolerance)
  {
    auto Qt = Q.transpose();
    Eigen::MatrixXd X = Q*u*Qt;
    Eigen::FullPivHouseholderQR<Eigen::MatrixXd> qr(X);
    Eigen::MatrixXd Xi = qr.solve(Ones);
    Eigen::MatrixXd M = Qt * Xi * Q;

    double maximumValue = M(0, 0);
    int maximumPosition = 0;
    for (int i = 0; i < numberOfPoints; ++i)
    {
      if (maximumValue < M(i, i))
      {
        maximumValue = M(i, i);
        maximumPosition = i;
      }
    }
    double stepsize = (maximumValue - dimension - 1) / ((dimension + 1) * (maximumValue - 1));
    Eigen::DiagonalMatrix<double, Eigen::Dynamic> new_u = (1.0 - stepsize) * u;
    new_u.diagonal()[maximumPosition] = (new_u.diagonal())(maximumPosition) + stepsize;
    ++count;
    error = (new_u.diagonal() - u.diagonal()).norm();
    u.diagonal() = new_u.diagonal();
  }

   // U = u
  Eigen::MatrixXd Ai = points * u * points.transpose() - points * u *(points * u).transpose();
  Eigen::FullPivHouseholderQR<Eigen::MatrixXd> qr(Ai);
  Eigen::VectorXd ones2(dimension);
  ones2.fill(1);
  Eigen::MatrixXd Ones2 = ones2.asDiagonal();
  Eigen::MatrixXd A = qr.solve(Ones2)*1.0/dimension;

  Eigen::JacobiSVD<Eigen::MatrixXd> svd(A);
  double c = 1 / sqrt(svd.singularValues()[0]);
  double b = 1 / sqrt(svd.singularValues()[1]);
  double a = 1 / sqrt(svd.singularValues()[2]);
  double V = 4 * vnl_math::pi*a*b*c / 3;

  double ad_mvee= 0;
  double alpha = std::sqrt(1 - b*b / a / a);
  double beta = std::sqrt(1 - c*c / a / a);
  for (int i = 0; i < 20; ++i)
  {
    ad_mvee += 4 * vnl_math::pi*a*b*(alpha*alpha + beta*beta) / (2 * alpha*beta) * (std::pow(alpha*beta, i)) / (1 - 4 * i*i);
  }
  vol = V;
  surf = ad_mvee;
}

mitk::GIFVolumetricDensityStatistics::FeatureListType mitk::GIFVolumetricDensityStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;
  if (image->GetDimension() < 3)
  {
    return featureList;
  }


  //AccessByItk_2(mask, CalculateLargestDiameter, image, featureList);

  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  vtkSmartPointer<vtkMassProperties> stats = vtkSmartPointer<vtkMassProperties>::New();
  vtkSmartPointer<vtkMassProperties> stats2 = vtkSmartPointer<vtkMassProperties>::New();
  mesher->SetInputData(mask->GetVtkImageData());
  mesher->SetValue(0, 0.5);
  stats->SetInputConnection(mesher->GetOutputPort());
  stats->Update();

  vtkSmartPointer<vtkDelaunay3D> delaunay =
    vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetInputConnection(mesher->GetOutputPort());
  //delaunay->SetInputData(mask->GetVtkImageData());
  delaunay->SetAlpha(0);
  delaunay->Update();
  vtkSmartPointer<vtkGeometryFilter> geometryFilter =
    vtkSmartPointer<vtkGeometryFilter>::New();
  geometryFilter->SetInputConnection(delaunay->GetOutputPort());
  geometryFilter->Update();
  stats2->SetInputConnection(geometryFilter->GetOutputPort());
  stats2->Update();

  auto data = mesher->GetOutput()->GetPointData();

  double vol_mvee;
  double surf_mvee;
  calculateMEE(mesher->GetOutput(), vol_mvee, surf_mvee);

  double vol_mobb;
  double surf_mobb;
  calculateMOBB(geometryFilter->GetOutput(), vol_mobb, surf_mobb);

  double pi = vnl_math::pi;

  double meshVolume = stats->GetVolume();
  double meshSurf = stats->GetSurfaceArea();


  AccessByItk_3(image, CalculateVolumeDensityStatistic, mask, meshVolume, featureList);

  //Calculate center of mass shift
  int xx = mask->GetDimensions()[0];
  int yy = mask->GetDimensions()[1];
  int zz = mask->GetDimensions()[2];

  double xd = mask->GetGeometry()->GetSpacing()[0];
  double yd = mask->GetGeometry()->GetSpacing()[1];
  double zd = mask->GetGeometry()->GetSpacing()[2];

  std::vector<cv::Point3d> pointsForPCA;
  std::vector<cv::Point3d> pointsForPCAUncorrected;

  int minimumX=xx;
  int maximumX=0;
  int minimumY=yy;
  int maximumY=0;
  int minimumZ=zz;
  int maximumZ=0;

  vtkSmartPointer<vtkPoints> points =
    vtkSmartPointer< vtkPoints >::New();

  for (int x = 0; x < xx; x++)
  {
    for (int y = 0; y < yy; y++)
    {
      for (int z = 0; z < zz; z++)
      {
        itk::Image<int,3>::IndexType index;

        index[0] = x;
        index[1] = y;
        index[2] = z;

        mitk::ScalarType pxImage;
        mitk::ScalarType pxMask;

        mitkPixelTypeMultiplex5(
              mitk::FastSinglePixelAccess,
              image->GetChannelDescriptor().GetPixelType(),
              image,
              image->GetVolumeData(),
              index,
              pxImage,
              0);

        mitkPixelTypeMultiplex5(
              mitk::FastSinglePixelAccess,
              mask->GetChannelDescriptor().GetPixelType(),
              mask,
              mask->GetVolumeData(),
              index,
              pxMask,
              0);

        //Check if voxel is contained in segmentation
        if (pxMask > 0)
        {
          minimumX = std::min<int>(x, minimumX);
          minimumY = std::min<int>(y, minimumY);
          minimumZ = std::min<int>(z, minimumZ);
          maximumX = std::max<int>(x, maximumX);
          maximumY = std::max<int>(y, maximumY);
          maximumZ = std::max<int>(z, maximumZ);
          points->InsertNextPoint(x*xd, y*yd, z*zd);
          cv::Point3d tmp;
          tmp.x = x * xd;
          tmp.y = y * yd;
          tmp.z = z * zd;
          pointsForPCAUncorrected.push_back(tmp);

          if (pxImage == pxImage)
          {
            pointsForPCA.push_back(tmp);
          }
        }
      }
    }
  }


  //Calculate PCA Features
  int sz = pointsForPCA.size();
  cv::Mat data_pts = cv::Mat(sz, 3, CV_64FC1);
  for (int i = 0; i < data_pts.rows; ++i)
  {
      data_pts.at<double>(i, 0) = pointsForPCA[i].x;
      data_pts.at<double>(i, 1) = pointsForPCA[i].y;
      data_pts.at<double>(i, 2) = pointsForPCA[i].z;
  }

  //Calculate PCA Features
  int szUC = pointsForPCAUncorrected.size();
  cv::Mat data_ptsUC = cv::Mat(szUC, 3, CV_64FC1);
  for (int i = 0; i < data_ptsUC.rows; ++i)
  {
    data_ptsUC.at<double>(i, 0) = pointsForPCAUncorrected[i].x;
    data_ptsUC.at<double>(i, 1) = pointsForPCAUncorrected[i].y;
    data_ptsUC.at<double>(i, 2) = pointsForPCAUncorrected[i].z;
  }

  //Perform PCA analysis
  cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
  cv::PCA pca_analysisUC(data_ptsUC, cv::Mat(), CV_PCA_DATA_AS_ROW);

  //Store the eigenvalues
  std::vector<double> eigen_val(3);
  std::vector<double> eigen_valUC(3);
  for (int i = 0; i < 3; ++i)
  {
    eigen_val[i] = pca_analysis.eigenvalues.at<double>(0, i);
    eigen_valUC[i] = pca_analysisUC.eigenvalues.at<double>(0, i);
  }

  std::sort(eigen_val.begin(), eigen_val.end());
  std::sort(eigen_valUC.begin(), eigen_valUC.end());

  double major = 2*sqrt(eigen_val[2]);
  double minor = 2*sqrt(eigen_val[1]);
  double least = 2*sqrt(eigen_val[0]);

  double alpha = std::sqrt(1 - minor*minor / major / major);
  double beta = std::sqrt(1 - least*least / major / major);

  double a = (maximumX - minimumX+1) * xd;
  double b = (maximumY - minimumY+1) * yd;
  double c = (maximumZ - minimumZ+1) * zd;

  double vd_aabb = meshVolume / (a*b*c);
  double ad_aabb = meshSurf / (2 * a*b + 2 * a*c + 2 * b*c);

  double vd_aee = 3 * meshVolume / (4.0*pi*major*minor*least);
  double ad_aee = 0;
  for (int i = 0; i < 20; ++i)
  {
    ad_aee += 4 * pi*major*minor*(alpha*alpha + beta*beta) / (2 * alpha*beta) * (std::pow(alpha*beta, i)) / (1 - 4 * i*i);
  }
  ad_aee = meshSurf / ad_aee;

  double vd_ch = meshVolume / stats2->GetVolume();
  double ad_ch = meshSurf / stats2->GetSurfaceArea();

  featureList.push_back(std::make_pair("Morphological Density Volume Density axis-aligned bounding box", vd_aabb));
  featureList.push_back(std::make_pair("Morphological Density Surface Density axis-aligned bounding box", ad_aabb));
  featureList.push_back(std::make_pair("Morphological Density Volume Density oriented bounding box", meshVolume / vol_mobb));
  featureList.push_back(std::make_pair("Morphological Density Surface Density oriented bounding box", meshSurf / surf_mobb));
  featureList.push_back(std::make_pair("Morphological Density Volume Density Approx. enclosing ellipsoid", vd_aee));
  featureList.push_back(std::make_pair("Morphological Density Surface Density Approx. enclosing ellipsoid", ad_aee));
  featureList.push_back(std::make_pair("Morphological Density Volume Density Approx. minimum enclosing ellipsoid", meshVolume / vol_mvee));
  featureList.push_back(std::make_pair("Morphological Density Surface Density Approx. minimum enclosing ellipsoid", meshSurf / surf_mvee));
  featureList.push_back(std::make_pair("Morphological Density Volume Density Convex Hull", vd_ch));
  featureList.push_back(std::make_pair("Morphological Density Surface Density Convex Hull", ad_ch));

  return featureList;
}

mitk::GIFVolumetricDensityStatistics::GIFVolumetricDensityStatistics()
{
  SetLongName("volume-density");
  SetShortName("volden");
}

mitk::GIFVolumetricDensityStatistics::FeatureNameListType mitk::GIFVolumetricDensityStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFVolumetricDensityStatistics::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Volume-Density Statistic", "calculates volume density based features", us::Any());
}

void
mitk::GIFVolumetricDensityStatistics::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating volumetric density features ....";
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating volumetric density features....";
  }
}

