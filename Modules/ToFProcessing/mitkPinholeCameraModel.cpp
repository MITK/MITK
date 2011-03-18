/*=========================================================================
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-07-17 15:44:24 +0200 (Fr, 17 Jul 2009) $
Version:   $Revision: 18261 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPinholeCameraModel.h"

mitk::PinholeCameraModel::PinholeCameraModel()
  : m_FocalLength(12.8), m_MaxDistance(2000.0), m_PrincipalPoint(), m_CameraToWorldTransform_R(), m_ImageArea(),
  m_InterPixelDistance(), m_DistanceImageDimension(2), m_DistanceImageDimensions(), m_CameraToWorldTransform_T(), m_ImageShift()
{
  this->InitializeWithDefaultSettings();
}

mitk::PinholeCameraModel::~PinholeCameraModel()
{}

void mitk::PinholeCameraModel::GetCameraToWorldTransform(itk::Matrix<ToFScalarType,3,3>& TransformR,itk::Vector<ToFScalarType,3>& TransformT)
{
  TransformR = m_CameraToWorldTransform_R;
  TransformT = m_CameraToWorldTransform_T;
}

void mitk::PinholeCameraModel::SetCameraToWorldTransform(itk::Matrix<ToFScalarType,3,3> TransformR,itk::Vector<ToFScalarType,3> TransformT)
{
  m_CameraToWorldTransform_R = TransformR;
  m_CameraToWorldTransform_T = TransformT;
}

void mitk::PinholeCameraModel::CalculateImageArea()
{
  m_ImageArea[0] = m_DistanceImageDimensions[0]*m_InterPixelDistance[0];
  m_ImageArea[1] = m_DistanceImageDimensions[1]*m_InterPixelDistance[1];
}

void mitk::PinholeCameraModel::SetDistanceImageDimensions(unsigned int* dimensions)
{
  m_DistanceImageDimensions = dimensions;
}

void mitk::PinholeCameraModel::CalculateImageDimensions()
{
  m_DistanceImageDimensions[0] = (unsigned int)(m_ImageArea[0]/m_InterPixelDistance[0] + 0.5);
  m_DistanceImageDimensions[1] = (unsigned int)(m_ImageArea[1]/m_InterPixelDistance[1] + 0.5);
}

mitk::ToFProcessingCommon::ToFPoint3D mitk::PinholeCameraModel::GetPixelMidPointInWorldCoordinates(unsigned int indexX, unsigned int indexY)
{
  ToFPoint3D returnValue;
  //calculate the mid-point of a pixel in world coordinates
  //[1] pinhole is in the middle, so we subtract half of the size of the image area
  //[2] then we multiplicate with the pixel size to get the coordinates in mm
  //[3] next we take account of the image shift (if the pinhole is NOT exactly in the middle)
  //[4] last we add half of the size of a pixel to get the middle of each pixel
  //                          [1]                   [2]               [3]                  [4]
  returnValue[0] =  ((indexX - m_PrincipalPoint[0])*m_InterPixelDistance[0] + (m_InterPixelDistance[0]/2));
  returnValue[1] =  ((indexY - m_PrincipalPoint[1])*m_InterPixelDistance[1] + (m_InterPixelDistance[1]/2));
  returnValue[2] = -m_FocalLength;
  return returnValue;
}

mitk::ToFProcessingCommon::ToFPoint3D mitk::PinholeCameraModel::GetCameraPositionInWorldCoordinates()
{
  ToFPoint3D returnValue;
  ToFPoint3D origin; origin[0] = 0;  origin[1] = 0;  origin[2] = 0;
  returnValue = (this->m_CameraToWorldTransform_R * origin) + this->m_CameraToWorldTransform_T;
  return returnValue;
}

void mitk::PinholeCameraModel::InitializeWithDefaultSettings()
{
  ToFPoint2D PrincipalPoint;
  PrincipalPoint[0] = 101.5f;
  PrincipalPoint[1] = 101.5f;

  ToFPoint2D ImageArea;
  ImageArea[0] = 9.18f;
  ImageArea[1] = 9.18f;

  ToFPoint2D InterPixelDistance;
  InterPixelDistance[0] = 0.045f;
  InterPixelDistance[1] = 0.045f;

  ToFPoint2D ImageShift;
  ImageShift[0] = 0.0;
  ImageShift[1] = 0.0;

  unsigned int* dimensions = new unsigned int[3];
  dimensions[0] = 204;
  dimensions[1] = 204;
//  dimensions[2] = 1;

  itk::Matrix<ToFScalarType,3,3> transR;
  transR.Fill(0); transR[0][0] = 1; transR[1][1] = 1; transR[2][2] = 1;

  itk::Vector<ToFScalarType,3> transT;
  transT.Fill(0); 
  
  SetDistanceImageDimensions(dimensions);
  SetPrincipalPoint(PrincipalPoint);
  SetImageArea(ImageArea);
  SetInterPixelDistance(InterPixelDistance);
  SetImageShift(ImageShift);
  SetCameraToWorldTransform(transR,transT);
}
