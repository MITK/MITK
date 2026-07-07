/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureSource_h
#define mitkPlanarFigureSource_h

#include <mitkBaseDataSource.h>
#include <mitkCommon.h>
#include <mitkPlanarFigure.h>
#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Base class for all filters producing a PlanarFigure as output.
   *
   * Base class for all filters which have an object of type mitk::PlanarFigure
   * as output. PlanarFigureSources do not provide support for streaming,
   * that is, the requested region is always the largest possible region.
   *
   * \ingroup MitkPlanarFigureModule
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureSource : public mitk::BaseDataSource
  {
  public:
    mitkClassMacro(PlanarFigureSource, BaseDataSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef mitk::PlanarFigure OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef itk::DataObject::Pointer DataObjectPointer;

    mitkBaseDataSourceGetOutputDeclarations

      /**
       * \brief Allocate a new output object and return it.
       *
       * Currently the index is not evaluated.
       *
       * \param[in] idx The index of the output for which an object should be created.
       * \return The new output object.
       */
      itk::DataObject::Pointer
      MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * This is a default implementation to make sure we have something.
     * Once all the subclasses of ProcessObject provide an appropriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

    /**
     * Generates the input requested region simply by calling the equivalent
     * method of the superclass.
     */
    void GenerateInputRequestedRegion() override;

  protected:
    PlanarFigureSource();
    ~PlanarFigureSource() override;
  };
} // namespace mitk
#endif
