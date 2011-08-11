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

#ifndef __vtkMitkApplyLevelWindowToRGBFilter_h
#define __vtkMitkApplyLevelWindowToRGBFilter_h

class vtkScalarsToColors;
//class vtkImageToImageFilter;
//#include <vtkImageRGBToHSI.h>
#include <vtkImageData.h>
//#include <vtkImageRGBToHSI.h>
//#include <vtkImageIterator.h>
//#include <vtkImageProgressIterator.h>
//#include <vtkLookupTable.h>
#include <vtkImageToImageFilter.h>
//#include <vtkThreadedImageAlgorithm.h>

#include "mitkCommon.h"

//#include "vtkThreadedImageAlgorithm.h"

class MITK_CORE_EXPORT vtkMitkApplyLevelWindowToRGBFilter : public vtkImageToImageFilter
{
public:
  vtkScalarsToColors* GetLookupTable();

  void SetLookupTable(vtkScalarsToColors *lookupTable);

  vtkMitkApplyLevelWindowToRGBFilter();
protected:
  ~vtkMitkApplyLevelWindowToRGBFilter();

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int extent[6], int id)

  template <class T>
      void vtkCalculateIntensityFromLookupTable(
                                                vtkImageData *inData,
                                                vtkImageData *outData,
                                                int outExt[6], T *);

  void ExecuteInformation();

  void ExecuteInformation(vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData));

private:
  vtkScalarsToColors* m_LookupTable;
};

#endif
