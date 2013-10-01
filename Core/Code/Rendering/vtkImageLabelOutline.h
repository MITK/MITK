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

/*==========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageLabelOutline.h,v $
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkImageLabelOutline_h
#define __vtkImageLabelOutline_h

#include "vtkImageNeighborhoodFilter.h"

#include <MitkExports.h>

class vtkImageData;

/// \brief Display labelmap outlines.
///
/// Used  in slicer for the Label layer to outline the segmented
/// structures (instead of showing them filled-in).
class MITK_CORE_EXPORT vtkImageLabelOutline : public vtkImageNeighborhoodFilter
{
public:
  static vtkImageLabelOutline *New();
//  vtkTypeRevisionMacro(vtkImageLabelOutline,vtkImageNeighborhoodFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// background pixel value in the image (usually 0)
  vtkSetMacro(Background, float);
  vtkGetMacro(Background, float);

  ///
  /// show all labels outlined
  vtkSetMacro(OutlineAll, bool);
  vtkGetMacro(OutlineAll, bool);

  ///
  /// active label in the image
  vtkSetMacro(ActiveLabel, int);
  vtkGetMacro(ActiveLabel, int);

protected:
  vtkImageLabelOutline();
  ~vtkImageLabelOutline();

  float Background;
  float ActiveLabel;
  bool OutlineAll;

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
  int extent[6], int id);
};

#endif
