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

#include "MitkDiffusionImagingExports.h"

#include "vtkPolyDataAlgorithm.h"
#include "mitkCommon.h"
#include <vtkFloatArray.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkOdfNormalizationMethodProperty.h>

class MitkDiffusionImaging_EXPORT vtkOdfSource : public vtkPolyDataAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkOdfSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  typedef itk::OrientationDistributionFunction<float, QBALL_ODFSIZE> OdfType;
  // Description:
  // Construct sphere with radius=0.5 and default resolution 8 in both Phi
  // and Theta directions. Theta ranges from (0,360) and phi (0,180) degrees.
  static vtkOdfSource *New();

  vtkSetMacro(Scale,double);
  vtkGetMacro(Scale,double);

  vtkSetMacro(AdditionalScale,double);
  vtkGetMacro(AdditionalScale,double);

  vtkSetMacro(Normalization,int);
  vtkGetMacro(Normalization,int);

  vtkSetMacro(Odf,OdfType);
  vtkGetMacro(Odf,OdfType);

protected:
  vtkOdfSource();
  ~vtkOdfSource() {}

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  OdfType Odf;
  double  Scale;
  double  AdditionalScale;
  int     Normalization;

private:
  vtkOdfSource(const vtkOdfSource&);  // Not implemented.
  void operator=(const vtkOdfSource&);  // Not implemented.
};

#endif //__vtkOdfSource_h
