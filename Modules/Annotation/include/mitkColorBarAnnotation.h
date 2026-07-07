/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkColorBarAnnotation_h
#define mitkColorBarAnnotation_h

#include <MitkAnnotationExports.h>
#include <mitkLocalStorageHandler.h>
#include <mitkVtkAnnotation.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class vtkScalarBarActor;

namespace mitk
{
  /**
   * \brief Displays a configurable color bar (scalar bar) on the render window.
   *
   * Wraps a vtkScalarBarActor to show a color legend corresponding to a
   * vtkLookupTable. Orientation, number of colors, labels, and tick marks
   * are configurable.
   *
   * \sa VtkAnnotation, ScaleLegendAnnotation
   */
  class MITKANNOTATION_EXPORT ColorBarAnnotation : public mitk::VtkAnnotation
  {
  public:
    /** \brief Per-renderer local storage for the scalar bar actor. */
    class LocalStorage : public mitk::Annotation::BaseLocalStorage
    {
    public:
      /** \brief The vtkScalarBarActor used for rendering. */
      vtkSmartPointer<vtkScalarBarActor> m_ScalarBarActor;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default destructor of the local storage. */
      ~LocalStorage();
    };

    mitkClassMacro(ColorBarAnnotation, mitk::VtkAnnotation);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Enable or disable drawing of text annotations on the color bar.
     * \param[in] annotations true to enable.
     */
    void SetDrawAnnotations(bool annotations);

    /**
     * \brief Query whether text annotations are drawn.
     * \return true if enabled.
     */
    bool GetDrawAnnotations() const;

    /** \brief Set the color bar orientation to horizontal. */
    void SetOrientationToHorizontal();

    /** \brief Set the color bar orientation to vertical. */
    void SetOrientationToVertical();

    /**
     * \brief Set the color bar orientation.
     * \param[in] orientation 0 for horizontal, 1 for vertical.
     */
    void SetOrientation(int orientation);

    /**
     * \brief Get the current orientation.
     * \return 0 for horizontal, 1 for vertical.
     */
    int GetOrientation() const;

    /**
     * \brief Set the maximum number of colors in the bar.
     * \param[in] numberOfColors The maximum color count.
     */
    void SetMaxNumberOfColors(int numberOfColors);

    /**
     * \brief Get the maximum number of colors.
     * \return The color count.
     */
    int GetMaxNumberOfColors() const;

    /**
     * \brief Set the number of labels displayed on the bar.
     * \param[in] numberOfLabels The label count.
     */
    void SetNumberOfLabels(int numberOfLabels);

    /**
     * \brief Get the number of labels.
     * \return The label count.
     */
    int GetNumberOfLabels() const;

    /**
     * \brief Set the lookup table used for the color mapping.
     * \param[in] table The vtkLookupTable to display.
     */
    void SetLookupTable(vtkSmartPointer<vtkLookupTable> table);

    /**
     * \brief Get the current lookup table.
     * \return The vtkLookupTable.
     */
    vtkSmartPointer<vtkLookupTable> GetLookupTable() const;

    /**
     * \brief Enable or disable drawing of tick labels.
     * \param[in] ticks true to enable.
     */
    void SetDrawTickLabels(bool ticks);

    /**
     * \brief Query whether tick labels are drawn.
     * \return true if enabled.
     */
    bool GetDrawTickLabels() const;

    /**
     * \brief Enable or disable text scaling on the annotation.
     * \param[in] scale true to enable scaling.
     */
    void SetAnnotationTextScaling(bool scale);

    /**
     * \brief Query whether annotation text scaling is enabled.
     * \return true if enabled.
     */
    bool GetAnnotationTextScaling() const;

  protected:
    /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
    mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation(BaseRenderer *renderer) override;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit ColorBarAnnotation();

    /** \brief virtual destructor in order to derive from this class */
    ~ColorBarAnnotation() override;

  private:
    /** \brief copy constructor */
    ColorBarAnnotation(const ColorBarAnnotation &);

    /** \brief assignment operator */
    ColorBarAnnotation &operator=(const ColorBarAnnotation &);
  };

} // namespace mitk
#endif
