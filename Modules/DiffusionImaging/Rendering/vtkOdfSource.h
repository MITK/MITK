/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __vtkOdfSource_h
#define __vtkOdfSource_h

#include "MitkDiffusionImagingExports.h"

#include "vtkPolyDataAlgorithm.h"
#include "mitkCommon.h"

class MitkDiffusionImaging_EXPORT vtkOdfSource : public vtkPolyDataAlgorithm 
{
public:
  vtkTypeRevisionMacro(vtkOdfSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct sphere with radius=0.5 and default resolution 8 in both Phi
  // and Theta directions. Theta ranges from (0,360) and phi (0,180) degrees.
  static vtkOdfSource *New();

  vtkSetMacro(TemplateOdf,vtkPolyData*);
  vtkGetMacro(TemplateOdf,vtkPolyData*);

  vtkSetMacro(OdfVals,vtkDoubleArray*);
  vtkGetMacro(OdfVals,vtkDoubleArray*);
  
  vtkSetMacro(Scale,double);
  vtkGetMacro(Scale,double);

protected:
  vtkOdfSource();
  ~vtkOdfSource() {}

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkPolyData* TemplateOdf;
  vtkDoubleArray* OdfVals;

  double Scale;

private:
  vtkOdfSource(const vtkOdfSource&);  // Not implemented.
  void operator=(const vtkOdfSource&);  // Not implemented.
};

#endif //__vtkOdfSource_h
