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


#include <QmitkFunctionality.h>
#include "ui_QmitkTbssSkeletonizationViewControls.h"
#include "itkImage.h"

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

class QmitkTbssSkeletonizationView : public QmitkFunctionality
{

  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkTbssSkeletonizationView();
    virtual ~QmitkTbssSkeletonizationView();

    virtual void CreateQtPartControl(QWidget *parent);

    //Creation of the connections of main and control widget
    virtual void CreateConnections();

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    /// \brief Called when the functionality is activated
    virtual void Activated();

    virtual void Deactivated();


  protected slots:


    /* \brief Perform skeletonization only */
    void Skeletonize();

    // Perform skeletonization and Projection of subject data to the skeleton
    void Project();



  protected:

    //brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::QmitkTbssSkeletonizationViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    void AddToDataStorage(mitk::Image* img, std::string name);

    Float4DImageType::Pointer ConvertToItk(itk::SmartPointer<mitk::Image> image);


};



#endif // _QMITKTbssSkeletonizationVIEW_H_INCLUDED

