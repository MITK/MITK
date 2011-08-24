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
#include <vtkImageData.h>
#include <vtkImageToImageFilter.h>

#include "mitkCommon.h"

class MITK_CORE_EXPORT vtkMitkApplyLevelWindowToRGBFilter : public vtkImageToImageFilter
{
public:
  vtkScalarsToColors* GetLookupTable();

  void SetLookupTable(vtkScalarsToColors *lookupTable);

  void SetMinOpacity(double minOpacity);
  inline double GetMinOpacity() const;

  void SetMaxOpacity(double maxOpacity);
  inline double GetMaxOpacity() const;

  vtkMitkApplyLevelWindowToRGBFilter();
protected:
  ~vtkMitkApplyLevelWindowToRGBFilter();

  /** \brief Method for threaded execution of the filter.
   * \param *inData: The input.
   * \param *outData: The output of the filter.
   * \param extent[6]: Specefies the region of the image to be updated inside this thread.
   * It is a six-component array of the form (xmin, xmax, ymin, ymax, zmin, zmax).
   * \param id: The thread id.
   */
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int extent[6], int id);

  void ExecuteInformation();

  void ExecuteInformation(vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData));

private:
  vtkScalarsToColors* m_LookupTable;
  double m_MinOqacity;
  double m_MaxOpacity;
};

#endif
