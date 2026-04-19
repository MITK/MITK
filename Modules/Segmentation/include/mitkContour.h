/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContour_h
#define mitkContour_h

#include <mitkBaseData.h>
#include <mitkCommon.h>
#include <MitkSegmentationExports.h>
#include <vtkRenderWindow.h>

#include <itkPolyLineParametricPath.h>

namespace mitk
{
  /**
    \brief Stores vertices for drawing a contour.

    \note mitk::ContourModel is the preferred alternative for new code.
    \sa ContourModel
  */
  class MITKSEGMENTATION_EXPORT Contour : public BaseData
  {
  public:
    mitkClassMacro(Contour, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef itk::PolyLineParametricPath<3> PathType;
    typedef PathType::Pointer PathPointer;
    typedef PathType::ContinuousIndexType ContinuousIndexType;
    typedef PathType::InputType InputType;
    typedef PathType::OutputType OutputType;
    typedef PathType::OffsetType OffsetType;
    typedef itk::
      BoundingBox<unsigned long, 3, ScalarType, itk::VectorContainer<unsigned long, mitk::Point<ScalarType, 3>>>
        BoundingBoxType;
    typedef BoundingBoxType::PointsContainer PointsContainer;
    typedef BoundingBoxType::PointsContainer::Pointer PointsContainerPointer;
    typedef BoundingBoxType::PointsContainerIterator PointsContainerIterator;

    /**
     * \brief Sets whether the contour is closed or open.
     * \param[in] _arg If true, contour is closed (default). If false, open.
     */
    itkSetMacro(Closed, bool);

    /**
     * \brief Returns whether the contour is closed.
     * \return true if the contour is closed, false if open.
     */
    itkGetMacro(Closed, bool);

    /** \brief Sets the selection state of the contour. */
    itkSetMacro(Selected, bool);

    /** \brief Returns the selection state of the contour. */
    itkGetMacro(Selected, bool);

    /** \brief Sets the line width for rendering the contour. */
    itkSetMacro(Width, float);

    /** \brief Returns the line width for rendering the contour. */
    itkGetMacro(Width, float);

    /**
     * \brief Resets the contour, removing all vertices.
     */
    void Initialize() override;

    /**
     * \brief Adds a new vertex to the contour.
     * \param[in] newPoint The 3D point to add.
     */
    void AddVertex(mitk::Point3D newPoint);

    /**
     * \brief Returns an ITK parametric path representation of the contour.
     * \return Smart pointer to the parametric path.
     */
    PathPointer GetContourPath() const;

    /**
     * \brief Sets the render window in which this contour should be drawn.
     * \param[in] rw Pointer to the VTK render window.
     */
    void SetCurrentWindow(vtkRenderWindow *rw);

    /**
     * \brief Returns the render window associated with this contour.
     * \return Pointer to the VTK render window, or nullptr.
     */
    vtkRenderWindow *GetCurrentWindow() const;

    /**
     * \brief Returns the number of vertices stored in the contour.
     * \return The vertex count.
     */
    unsigned int GetNumberOfPoints() const;

    /**
     * \brief Returns the container of contour vertices.
     * \return Smart pointer to the points container.
     */
    PointsContainerPointer GetPoints() const;

    /**
     * \brief Sets the contour points container.
     * \param[in] points Smart pointer to the new points container.
     */
    void SetPoints(PointsContainerPointer points);

    /** \brief Updates the output information (bounding box, etc.). */
    void UpdateOutputInformation() override;

    /** \brief Sets the requested region to the largest possible region. */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /** \brief Checks whether the requested region is outside the buffered region. */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /** \brief Verifies that the requested region is valid. */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Sets the requested region from an itk::DataObject.
     * \param[in] data The data object from which to copy the requested region.
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

  protected:
    mitkCloneMacro(Self);

    Contour();
    Contour(const Contour &other);
    ~Contour() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
    /**
    * parametric path of a contour;
    */
    PathType::Pointer m_ContourPath;

    /**
    * the current render window
    */
    vtkRenderWindow *m_CurrentWindow;

    /**
    * the bounding box of the contour
    */
    BoundingBoxType::Pointer m_BoundingBox;

    /**
    * container for all contour points
    */
    BoundingBoxType::PointsContainer::Pointer m_Vertices;

    /**
    * decide whether th contour is open or closed
    */
    bool m_Closed;

    bool m_Selected;

    float m_Width;
  };

} // namespace mitk

#endif
