/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtrudePlanarFigureFilter_h
#define mitkExtrudePlanarFigureFilter_h

#include <MitkPlanarFigureExports.h>
#include <itkProcessObject.h>
#include <mitkCommon.h>
#include <mitkVector.h>

namespace mitk
{
  class PlanarFigure;
  class Surface;

  class MITKPLANARFIGURE_EXPORT ExtrudePlanarFigureFilter : public itk::ProcessObject
  {
  public:
    mitkClassMacroItkParent(ExtrudePlanarFigureFilter, itk::ProcessObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkGetMacro(Length, ScalarType);
    itkSetMacro(Length, ScalarType);

    itkGetMacro(NumberOfSegments, unsigned int);
    itkSetMacro(NumberOfSegments, unsigned int);

    itkGetMacro(TwistAngle, ScalarType);
    itkSetMacro(TwistAngle, ScalarType);

    itkGetMacro(BendAngle, ScalarType);
    itkSetClampMacro(BendAngle, ScalarType, -360, 360);

    itkGetMacro(BendDirection, Vector2D);
    mitkSetConstReferenceMacro(BendDirection, Vector2D);

    itkGetMacro(FlipDirection, bool);
    itkSetMacro(FlipDirection, bool);

    itkGetMacro(FlipNormals, bool);
    itkSetMacro(FlipNormals, bool);

    DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx) override;
    DataObjectPointer MakeOutput(const DataObjectIdentifierType &name) override;

    using Superclass::SetInput;
    void SetInput(mitk::PlanarFigure *planarFigure);

    using Superclass::GetOutput;
    mitk::Surface *GetOutput();

  protected:
    ExtrudePlanarFigureFilter();
    ~ExtrudePlanarFigureFilter() override;

    void GenerateData() override;
    void GenerateOutputInformation() override;
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
    ScalarType m_Length;
    unsigned int m_NumberOfSegments;
    ScalarType m_TwistAngle;
    ScalarType m_BendAngle;
    Vector2D m_BendDirection;
    bool m_FlipDirection;
    bool m_FlipNormals;
  };
}

#endif
