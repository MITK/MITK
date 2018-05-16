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

#ifndef SCALELEGENDAnnotation_H
#define SCALELEGENDAnnotation_H

#include "MitkAnnotationExports.h"
#include <mitkLocalStorageHandler.h>
#include <mitkVtkAnnotation.h>
#include <vtkSmartPointer.h>

class vtkLegendScaleActor;

namespace mitk
{
  /** \brief Displays configurable scales on the renderwindow. The scale is determined by the image spacing. */
  class MITKANNOTATION_EXPORT ScaleLegendAnnotation : public mitk::VtkAnnotation
  {
  public:
    class LocalStorage : public mitk::Annotation::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      vtkSmartPointer<vtkLegendScaleActor> m_legendScaleActor;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();
    };

    mitkClassMacro(ScaleLegendAnnotation, mitk::VtkAnnotation);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    void SetRightAxisVisibility(bool visibility);
    bool GetRightAxisVisibility() const;

    void SetLeftAxisVisibility(bool visibility);
    bool GetLeftAxisVisibility() const;

    void SetTopAxisVisibility(bool visibility);
    bool GetTopAxisVisibility() const;

    void SetBottomAxisVisibility(bool visibility);
    bool GetBottomAxisVisibility() const;

    void SetLegendVisibility(bool visibility);
    bool GetLegendVisibility() const;

    void SetRightBorderOffset(int offset);
    int GetRightBorderOffset() const;

    void SetLeftBorderOffset(int offset);
    int GetLeftBorderOffset() const;

    void SetTopBorderOffset(int offset);
    int GetTopBorderOffset() const;

    void SetBottomBorderOffset(int offset);
    int GetBottomBorderOffset() const;

    void SetFontFactor(double fontFactor);
    double GetFontFactor() const;

    void SetCornerOffsetFactor(double offsetFactor);
    double GetCornerOffsetFactor() const;

  protected:
    /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
    mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation(BaseRenderer *renderer) override;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit ScaleLegendAnnotation();

    /** \brief virtual destructor in order to derive from this class */
    ~ScaleLegendAnnotation() override;

  private:
    /** \brief copy constructor */
    ScaleLegendAnnotation(const ScaleLegendAnnotation &);

    /** \brief assignment operator */
    ScaleLegendAnnotation &operator=(const ScaleLegendAnnotation &);
  };

} // namespace mitk
#endif // SCALELEGENDAnnotation_H
