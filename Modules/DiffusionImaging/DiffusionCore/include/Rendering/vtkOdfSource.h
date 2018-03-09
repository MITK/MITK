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

#ifndef __vtkOdfSource_h
#define __vtkOdfSource_h

#include <MitkDiffusionCoreExports.h>

#include "vtkPolyDataAlgorithm.h"
#include "mitkCommon.h"
#include <vtkFloatArray.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkOdfNormalizationMethodProperty.h>

class MITKDIFFUSIONCORE_EXPORT vtkOdfSource : public vtkPolyDataAlgorithm
{
public:

  vtkTypeMacro(vtkOdfSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef itk::OrientationDistributionFunction<float, ODF_SAMPLING_SIZE> OdfType;
  // Description:
  // Construct sphere with radius=0.5 and default resolution 8 in both Phi
  // and Theta directions. Theta ranges from (0,360) and phi (0,180) degrees.
  static vtkOdfSource *New();

  vtkSetMacro(Scale,double)
  vtkGetMacro(Scale,double)

  vtkSetMacro(AdditionalScale,double)
  vtkGetMacro(AdditionalScale,double)

  vtkSetMacro(Normalization,int)
  vtkGetMacro(Normalization,int)

  vtkSetMacro(Odf,OdfType)
  vtkGetMacro(Odf,OdfType)

  vtkSetMacro(UseCustomColor,bool)
  vtkGetMacro(UseCustomColor,bool)

  void SetColor(int r, int g, int b)
  {
    this->r = r;
    this->g = g;
    this->b = b;
  }

protected:
  vtkOdfSource();
  ~vtkOdfSource() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  OdfType Odf;
  double  Scale;
  double  AdditionalScale;
  int     Normalization;
  int r;
  int g;
  int b;
  bool    UseCustomColor;
  vtkSmartPointer<vtkLookupTable> lut;

private:
  vtkOdfSource(const vtkOdfSource&);  // Not implemented.
  void operator=(const vtkOdfSource&);  // Not implemented.
};

#endif //__vtkOdfSource_h
