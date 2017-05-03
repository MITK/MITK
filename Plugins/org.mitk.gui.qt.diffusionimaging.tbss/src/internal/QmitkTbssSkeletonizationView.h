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

#ifndef QmitkTbssSkeletonizationView_h
#define QmitkTbssSkeletonizationView_h


#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

#include "ui_QmitkTbssSkeletonizationViewControls.h"
#include "itkImage.h"
#include "mitkPixelType.h"

namespace mitk {
  class Image;
}

typedef itk::Image<float, 3> FloatImageType;
typedef itk::Image<char, 3> CharImageType;
typedef itk::Image<float, 4> Float4DImageType;

typedef itk::CovariantVector<int,3> VectorType;
typedef itk::Image<VectorType, 3> DirectionImageType;


/*!
  * \brief Implementation of the core functionality of TBSS.
  * This plugin provides the core functionality of TBSS (see Smith et al., 2009. http://dx.doi.org/10.1016/j.neuroimage.2006.02.024)
  * It can skeletonize a mean FA image and calculate the projection of all individual subjects to this skeleton.
*/

class QmitkTbssSkeletonizationView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkTbssSkeletonizationView();
    virtual ~QmitkTbssSkeletonizationView();

    virtual void CreateQtPartControl(QWidget *parent) override;

    //Creation of the connections of main and control widget
    virtual void CreateConnections();

    ///
    /// Sets the focus to an internal widget.
    ///
    virtual void SetFocus() override;

    /// \brief Called when the view gets activated
    virtual void Activated() override;

    /// \brief Called when the view gets deactivated
    virtual void Deactivated() override;

    bool IsActivated() const;

    /// \brief Called when the view gets visible
    virtual void Visible() override;

    /// \brief Called when the view gets hidden
    virtual void Hidden() override;

  protected slots:


    /* \brief Perform skeletonization only */
    void Skeletonize();

    // Perform skeletonization and Projection of subject data to the skeleton
    void Project();



  protected:

    //brief called by QmitkAbstractView when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

    Ui::QmitkTbssSkeletonizationViewControls* m_Controls;

    void AddToDataStorage(mitk::Image* img, std::string name);

    template <class TPixel>
    void ConvertToItk(mitk::PixelType, mitk::Image* image, Float4DImageType::Pointer);

  private:

    bool m_Activated;

};



#endif // _QMITKTbssSkeletonizationVIEW_H_INCLUDED

