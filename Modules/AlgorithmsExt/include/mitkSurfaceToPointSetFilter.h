/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceToPointSetFilter_h
#define mitkSurfaceToPointSetFilter_h

// exports
#include <MitkAlgorithmsExtExports.h>

// mitk headers
#include <mitkSurface.h>
#include <mitkPointSetSource.h>
//#include <itkExtendedDoublyLinkedFaceList.h>

namespace mitk
{
  /**
   * \brief Convert a Surface into a PointSet without duplicate points.
   *
   * This filter extracts all unique vertices from the input surface and
   * produces a PointSet where each point appears exactly once (unlike the
   * STL format which may contain duplicate vertices).
   */

  class MITKALGORITHMSEXT_EXPORT SurfaceToPointSetFilter : public mitk::PointSetSource
  {
  public:
    mitkClassMacro(SurfaceToPointSetFilter, mitk::PointSetSource);
    itkNewMacro(Self);
    using itk::ProcessObject::SetInput;
    void SetInput(mitk::Surface::Pointer m_InputSurface);

    std::string GetErrorMessage();

  protected:
    SurfaceToPointSetFilter();
    ~SurfaceToPointSetFilter() override;

    /** \brief Generate the output of this filter. Called in the update process of the pipeline. */
    void GenerateData() override;

    //############### members ########################
    mitk::Surface::Pointer m_InputSurface;
    std::string m_ErrorMessage;
  };

} // namespace mitk
#endif
