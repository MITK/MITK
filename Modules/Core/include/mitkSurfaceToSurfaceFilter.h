/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceToSurfaceFilter_h
#define mitkSurfaceToSurfaceFilter_h

#include <mitkSurfaceSource.h>

namespace mitk
{
  class Surface;
  /**
   * \brief Superclass of all classes getting surfaces (instances of class
   * Surface) as input and generating surfaces as output.
   *
   * In ITK and VTK the generated result of a ProcessObject is only guaranteed
   * to be up-to-date when Update() of the ProcessObject or the generated
   * DataObject is called immediately before access of the data stored in the
   * DataObject. This is also true for subclasses of mitk::BaseProcess and thus
   * for mitk::SurfaceToSurfaceFilter.
   *
   * \ingroup Process
   */
  class MITKCORE_EXPORT SurfaceToSurfaceFilter : public mitk::SurfaceSource
  {
  public:
    mitkClassMacro(SurfaceToSurfaceFilter, mitk::SurfaceSource);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef itk::DataObject::Pointer DataObjectPointer;

    using itk::ProcessObject::SetInput;

    /**
     * \brief Set the input surface at index 0.
     *
     * \param[in] surface the input surface.
     */
    virtual void SetInput(const mitk::Surface *surface);

    /**
     * \brief Set the input surface at the given index.
     *
     * Also calls CreateOutputForInput(idx) to create a corresponding output.
     *
     * \note The inputs must be added sequentially.
     *
     * \param[in] idx the index of the input, which must be incremental.
     * \param[in] surface the input surface to add.
     */
    virtual void SetInput(unsigned int idx, const mitk::Surface *surface);

    /**
     * \brief Get the input surface at index 0.
     * \return Pointer to the input surface, or nullptr if none is set.
     */
    virtual const mitk::Surface *GetInput();

    /**
     * \brief Get the input surface at the given index.
     *
     * \param[in] idx the index of the input.
     * \return Pointer to the input surface, or nullptr if none is set.
     */
    virtual const mitk::Surface *GetInput(unsigned int idx);

    /**
     * \brief Create a new output for the input at the given index.
     *
     * \param[in] idx the index of the input for which the output should be created.
     * \throw mitk::Exception if the input at idx does not exist.
     */
    virtual void CreateOutputForInput(unsigned int idx);

    /**
     * \brief Create outputs for all existing inputs.
     *
     * For each existing input, a new output will be allocated if one does
     * not already exist.
     */
    virtual void CreateOutputsForAllInputs();

    /**
     * \brief Remove all inputs matching the given surface.
     *
     * \param[in] surface the surface to remove from the inputs.
     */
    virtual void RemoveInputs(mitk::Surface *surface);

  protected:
    SurfaceToSurfaceFilter();

    ~SurfaceToSurfaceFilter() override;
  };

} // namespace mitk

#endif
