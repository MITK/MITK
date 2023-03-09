/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceToSurfaceFilter_h
#define mitkSurfaceToSurfaceFilter_h

#include "mitkSurfaceSource.h"

namespace mitk
{
  class Surface;
  //##Documentation
  //## @brief Superclass of all classes getting surfaces (instances of class
  //## Surface) as input and generating surfaces as output.
  //##
  //## In itk and vtk the generated result of a ProcessObject is only guaranteed
  //## to be up-to-date, when Update() of the ProcessObject or the generated
  //## DataObject is called immediately before access of the data stored in the
  //## DataObject. This is also true for subclasses of mitk::BaseProcess and thus
  //## for mitk::mitkSurfaceToSurfaceFilter.
  //## @ingroup Process
  class MITKCORE_EXPORT SurfaceToSurfaceFilter : public mitk::SurfaceSource
  {
  public:
    mitkClassMacro(SurfaceToSurfaceFilter, mitk::SurfaceSource);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef itk::DataObject::Pointer DataObjectPointer;

    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::Surface *surface);

    /**
      * @brief Add a new input at the given index (idx)
      * Calls mitk::Surface::CreateOutputForInput(idx)
      * @note The inputs must be added sequentially
      * @param idx the index of the input, which must be incremental
      * @param surface the input which should be added
      */
    virtual void SetInput(unsigned int idx, const mitk::Surface *surface);

    virtual const mitk::Surface *GetInput();

    virtual const mitk::Surface *GetInput(unsigned int idx);

    /**
      * @brief Create a new output for the input at idx
      * @param idx the index of the input for which the output should be created
      */
    virtual void CreateOutputForInput(unsigned int idx);

    /**
      * @brief Creates outputs for all existing inputs
      * @note For each existing input a new output will be allocated
      */
    virtual void CreateOutputsForAllInputs();

    virtual void RemoveInputs(mitk::Surface *surface);

  protected:
    SurfaceToSurfaceFilter();

    ~SurfaceToSurfaceFilter() override;
  };

} // namespace mitk

#endif
