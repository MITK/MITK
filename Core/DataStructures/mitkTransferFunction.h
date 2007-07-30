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

#include <mitkHistogramGenerator.h>
#include "mitkCommon.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <algorithm>
#include <set>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <mitkImage.h>
#include <itkHistogram.h>

namespace mitk
{

  //##
  //##Documentation
  //## @brief
  //##
  //##
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
      static Pointer CreateForMitkImage(const mitk::Image* image);
      ~TransferFunction() {
        m_ColorTransferFunction->Delete();
        m_ScalarOpacityFunction->Delete();
        m_GradientOpacityFunction->Delete();
      }
      itkNewMacro(Self);
    protected:
      TransferFunction(int min, int max) : m_Min(min), m_Max(max) , m_ColorTransferFunction(vtkColorTransferFunction::New()), m_ScalarOpacityFunction(vtkPiecewiseFunction::New()), m_GradientOpacityFunction(vtkPiecewiseFunction::New()), m_Histogram(NULL)  {
        this->m_ScalarOpacityFunction->Initialize();
        this->m_GradientOpacityFunction->Initialize();
      }
      TransferFunction() : m_Min(0), m_Max(255) , m_ColorTransferFunction(vtkColorTransferFunction::New()), m_ScalarOpacityFunction(vtkPiecewiseFunction::New()),m_GradientOpacityFunction(vtkPiecewiseFunction::New()), m_Histogram(NULL) {
        this->m_ScalarOpacityFunction->Initialize();
        this->m_GradientOpacityFunction->Initialize();
      }
       int m_Min;
      int m_Max;

      vtkColorTransferFunction* m_ColorTransferFunction;
      vtkPiecewiseFunction* m_ScalarOpacityFunction;
      vtkPiecewiseFunction* m_GradientOpacityFunction;
      mitk::HistogramGenerator::HistogramType::ConstPointer m_Histogram; 

  };
}
#endif 
// MITK_TRANSFER_FUNCTION_H_HEADER_INCLUDED
