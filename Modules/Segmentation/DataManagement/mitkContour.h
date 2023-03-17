/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContour_h
#define mitkContour_h

#include "mitkBaseData.h"
#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include <vtkRenderWindow.h>

#include <itkPolyLineParametricPath.h>

namespace mitk
{
  /**
    \brief Stores vertices for drawing a contour.
    \deprecated Use class mitk::ContourModel instead.

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
    * sets whether the contour should be closed or open.
    * by default the contour is closed
    */
    itkSetMacro(Closed, bool);

    /**
    * returns if the contour is closed or opened
    */
    itkGetMacro(Closed, bool);

    itkSetMacro(Selected, bool);

    itkGetMacro(Selected, bool);

    itkSetMacro(Width, float);

    itkGetMacro(Width, float);

    /**
    * clean up the contour data
    */
    void Initialize() override;

    /**
    * add a new vertex to the contour
    */
    void AddVertex(mitk::Point3D newPoint);

    /**
    * return an itk parametric path of the contour
    */
    PathPointer GetContourPath() const;

    /**
    * set the current render window. This is helpful if one
    * wants to draw the contour in one special window only.
    */
    void SetCurrentWindow(vtkRenderWindow *rw);

    /**
    * returns the points to the current render window
    */
    vtkRenderWindow *GetCurrentWindow() const;

    /**
    * returns the number of points stored in the contour
    */
    unsigned int GetNumberOfPoints() const;

    /**
    * returns the container of the contour points
    */
    PointsContainerPointer GetPoints() const;

    /**
    * set the contour points container.
    */
    void SetPoints(PointsContainerPointer points);

    /**
    * intherited from parent
    */
    void UpdateOutputInformation() override;

    /**
    * intherited from parent
    */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
    * intherited from parent
    */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
    * intherited from parent
    */
    bool VerifyRequestedRegion() override;

    /**
    * intherited from parent
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
