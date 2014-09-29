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

#ifndef __vtkMitkLevelWindowFilter_h
#define __vtkMitkLevelWindowFilter_h

class vtkScalarsToColors;
class vtkPiecewiseFunction;
#include <vtkImageData.h>
#include <vtkThreadedImageAlgorithm.h>

#include <MitkCoreExports.h>
/** Documentation
* \brief Applies the grayvalue or color/opacity level window to scalar or RGB(A) images.
*
* This filter is used to apply the color level window to RGB images (e.g.
* diffusion tensor images). Therefore, the RGB channels are converted to
* the HSI color space, where the level window can be applied. Afterwards,
* the HSI values transformed back to the RGB space.
*
* The filter is also able to apply an opacity level window to RGBA images.
*
* \ingroup Renderer
*/
class MITK_CORE_EXPORT vtkMitkLevelWindowFilter : public vtkThreadedImageAlgorithm
{


public:
  vtkTypeMacro(vtkMitkLevelWindowFilter,vtkThreadedImageAlgorithm);

  static vtkMitkLevelWindowFilter *New();

  virtual unsigned long int GetMTime();
  /** \brief Get the lookup table for the RGB level window */
  vtkScalarsToColors* GetLookupTable();
  /** \brief Set the lookup table for the RGB level window */
  void SetLookupTable(vtkScalarsToColors *lookupTable);

  /** \brief Get the piecewise function used to map scalar to alpha component value (only
   *  used when the lookupTable is a vtkColorTransferFunction) */
  vtkPiecewiseFunction* GetOpacityPiecewiseFunction()
  { return m_OpacityFunction; }
  /** \brief Set the piecewise function used to map scalar to alpha component value (only
   *  used when the lookupTable is a vtkColorTransferFunction) */
  void SetOpacityPiecewiseFunction(vtkPiecewiseFunction *opacityFunction);

  /** \brief Get/Set the lower window opacity for the alpha level window */
  void SetMinOpacity(double minOpacity);
  inline double GetMinOpacity() const;

  /** \brief Get/Set the upper window opacity for the alpha level window */
  void SetMaxOpacity(double maxOpacity);
  inline double GetMaxOpacity() const;

  /** \brief Set clipping bounds for the opaque part of the resliced 2d image */
  void SetClippingBounds(double*);

protected:

  /** Default constructor. */
  vtkMitkLevelWindowFilter();
  /** Default deconstructor. */
  ~vtkMitkLevelWindowFilter();
  /** \brief Method for threaded execution of the filter.
   * \param *inData: The input.
   * \param *outData: The output of the filter.
   * \param extent: Specifies the region of the image to be updated inside this thread.
   * It is a six-component array of the form (xmin, xmax, ymin, ymax, zmin, zmax).
   * \param id: The thread id.
   */
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int extent[6], int id);

//  /** Standard VTK filter method to apply the filter. See VTK documentation.*/
  int RequestInformation(vtkInformation* request,vtkInformationVector** inputVector, vtkInformationVector* outputVector);
//  /** Standard VTK filter method to apply the filter. See VTK documentation. Not used at the moment.*/
//  void ExecuteInformation(vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData));

private:
  /** m_LookupTable contains the lookup table for the RGB level window.*/
  vtkScalarsToColors* m_LookupTable;
  /** The transfer function to map the scalar to alpha (4th component of the RGBA output value) */
  vtkPiecewiseFunction* m_OpacityFunction;
  /** m_MinOpacity contains the lower bound for the alpha level window.*/
  double m_MinOpacity;
  /** m_MaxOpacity contains the upper bound for the alpha level window.*/
  double m_MaxOpacity;

  double m_ClippingBounds[4];
};
#endif
