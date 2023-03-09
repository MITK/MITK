/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceToPointSetFilter_h
#define mitkSurfaceToPointSetFilter_h

// mitk headers
#include "mitkSurface.h"
#include <mitkPointSetSource.h>

namespace mitk
{
  /** Documentation
   *  @brief This filter converts the input surface into a point set. The output point set contains every point exactly
   * one time
   *         (no dublicated points like in the stl-format).
   */

  class SurfaceToPointSetFilter : public mitk::PointSetSource
  {
  public:
    mitkClassMacro(SurfaceToPointSetFilter, mitk::PointSetSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    using Superclass::SetInput;
    void SetInput(mitk::Surface::Pointer m_InputSurface);

    std::string GetErrorMessage();

  protected:
    SurfaceToPointSetFilter();
    ~SurfaceToPointSetFilter() override;

    /** @brief method generating the output of this filter. Called in the updated process of the pipeline. */
    void GenerateData() override;

    //############### members ########################
    mitk::Surface::Pointer m_InputSurface;
    std::string m_ErrorMessage;
  };

} // namespace mitk
#endif /* MITKSURFACETODISTANCEIMAGEFILTER_H_HEADER_INCLUDED_ */
