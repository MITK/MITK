/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED
#define MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED

#include <itkObject.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <itkRGBPixel.h>

#include <vector>

#include "mitkHistogramGenerator.h"
#include "mitkCommon.h"
#include "mitkImage.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <algorithm>
#include <set>

#include <itkHistogram.h>


namespace mitk {
  
  class TransferFunction : public itk::Object
{
public:
  
  mitkClassMacro(TransferFunction, itk::DataObject);
  itkSetMacro(Min,int);
  itkSetMacro(Max,int);
  itkGetMacro(Min,int);
  itkGetMacro(Max,int);
  itkGetMacro(ScalarOpacityFunction,vtkPiecewiseFunction*); 
  itkGetMacro(GradientOpacityFunction,vtkPiecewiseFunction*); 
  itkGetMacro(ColorTransferFunction,vtkColorTransferFunction*); 
  itkGetConstObjectMacro(Histogram,HistogramGenerator::HistogramType);
  void InitializeByMitkImage(const mitk::Image* image);
  void InitializeByItkHistogram(const itk::Statistics::Histogram<double>* histogram);
  
  itkNewMacro(Self);
  
  typedef std::vector<std::pair<double, double> > ControlPoints;
  typedef std::vector<std::pair<double, itk::RGBPixel<double> > > RGBControlPoints;
  
  /*!  
  \brief Insert controlpoints and values into the piecewise transfer function of a channel  (scalar / gradient)
  */  
  void SetPoints(int channel, TransferFunction::ControlPoints points);
  
  /*!  
  \brief Insert RGB controlpoints and values 
  */  
  void SetRGBPoints(TransferFunction::RGBControlPoints rgbpoints);
      
  /*!  
  \brief Add a single controlpoint to the transfer function of a channel  (scalar / gradient)
  */  
  void AddPoint(int channel, double x, double value);
  
  /*!  
  \brief Add a single RGB controlpoint
  */  
  void AddRGBPoint(double x, double r, double g, double b);
  
  /*!
  \brief Get all controlpoints of a channel
  */
  TransferFunction::ControlPoints GetPoints(int channel);
  
  /*!
  \brief Get all RGB controlpoints
  */
  TransferFunction::RGBControlPoints GetRGBPoints();
  
  /*!
  \brief Remove one controlpoint of a channel
  */
  void RemovePoint(int channel, double x);
  
  /*!
  \brief Remove one rgb controlpoint
  */
  void RemoveRGBPoint(double x);
  
  /*!
  \brief Removes all controlpoints of a channel
  */
  void ClearPoints(int channel);
  
  /*!
  \brief Removes all rgb controlpoints
  */
  void ClearRGBPoints();
  
protected:
  TransferFunction();
  virtual ~TransferFunction();
  
  /*!
  * controlpoints of the piecewise linear opacity functions (scalar and gradient)
  */
  TransferFunction::ControlPoints m_ScalarOpacityPoints;
  
  TransferFunction::ControlPoints m_GradientOpacityPoints;
  
  TransferFunction::RGBControlPoints m_RGBPoints;
  
  
  
  /*!
  * the RGB color function controlpoints
  */
  TransferFunction::RGBControlPoints m_RGBControlPoints;
  
  vtkPiecewiseFunction* m_ScalarOpacityFunction;
  vtkColorTransferFunction* m_ColorTransferFunction;
  vtkPiecewiseFunction* m_GradientOpacityFunction;
  
  
  int m_Min;
  int m_Max;
  
  mitk::HistogramGenerator::HistogramType::ConstPointer m_Histogram; 

};

}

#endif /* MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED */
