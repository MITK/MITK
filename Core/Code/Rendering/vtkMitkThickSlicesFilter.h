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

// .NAME vtkMitkThickSlicesFilter - Computes the gradient vector.
// .SECTION Description
// vtkMitkThickSlicesFilter computes the gradient vector of an image.  The
// vector results are stored as scalar components. The Dimensionality
// determines whether to perform a 2d or 3d gradient. The default is
// two dimensional XY gradient.  OutputScalarType is always
// double. Gradient is computed using central differences.

#ifndef __vtkMitkThickSlicesFilter_h
#define __vtkMitkThickSlicesFilter_h

#include "mitkCommon.h"

#include "vtkThreadedImageAlgorithm.h"

class MITK_CORE_EXPORT vtkMitkThickSlicesFilter : public vtkThreadedImageAlgorithm
{
public:
  static vtkMitkThickSlicesFilter *New();
  vtkTypeRevisionMacro(vtkMitkThickSlicesFilter,vtkThreadedImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Determines how the input is interpreted (set of 2d slices ...)
  vtkSetClampMacro(Dimensionality,int,2,3);
  vtkGetMacro(Dimensionality,int);

  // Description:
  // Get/Set whether to handle boundaries.  If enabled, boundary
  // pixels are treated as duplicated so that central differencing
  // works for the boundary pixels.  If disabled, the output whole
  // extent of the image is reduced by one pixel.
  vtkSetMacro(HandleBoundaries, int);
  vtkGetMacro(HandleBoundaries, int);
  vtkBooleanMacro(HandleBoundaries, int);
  
  enum {
    MIP=0,
    SUM
  };

protected:
  vtkMitkThickSlicesFilter();
  ~vtkMitkThickSlicesFilter() {};

  int HandleBoundaries;
  int Dimensionality;

  virtual int RequestInformation (vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

  void ThreadedRequestData(vtkInformation*,
                           vtkInformationVector**,
                           vtkInformationVector*,
                           vtkImageData*** inData,
                           vtkImageData** outData,
                           int outExt[6],
                           int threadId);
  
  
  int m_CurrentMode;
                           
private:
  vtkMitkThickSlicesFilter(const vtkMitkThickSlicesFilter&);  // Not implemented.
  void operator=(const vtkMitkThickSlicesFilter&);  // Not implemented.
  
  public:
  
  void SetThickSliceMode( int mode)
  {
    m_CurrentMode = mode;
  }
 
  int GetThickSliceMode()
  {
    return m_CurrentMode;
  }

};

#endif



