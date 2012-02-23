/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkTbssSkeletonizationView_h
#define QmitkTbssSkeletonizationView_h

// Blueberry
#include <berryISelectionListener.h>
#include <berryIPartListener.h>
#include <berryIStructuredSelection.h>

// itk
#include "itkImage.h"
#include "itkVectorImage.h"

#include <QmitkFunctionality.h>

#include "ui_QmitkTbssSkeletonizationViewControls.h"


typedef itk::Image<float, 3> FloatImageType;
typedef itk::Image<char, 3> CharImageType;
typedef itk::Image<float, 4> Float4DImageType;

typedef itk::CovariantVector<int,3> VectorType;
typedef itk::Image<VectorType, 3> DirectionImageType;

struct TbssSkeletonizationSelListener;



/*!
  \brief QmitkTbssSkeletonizationView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionalitymitkTbssWorkspaceManager
  \ingroup Functionalities
*/
class QmitkTbssSkeletonizationView : public QmitkFunctionality
{


  friend struct TbssSkeletonizationSelListener;



  // this is needed for all Qt objesetupUicts that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkTbssSkeletonizationView();
    virtual ~QmitkTbssSkeletonizationView();

    virtual void CreateQtPartControl(QWidget *parent);

    /// \brief Creation of the connections of main and control widget
    virtual void CreateConnections();

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    /// \brief Called when the functionality is activated
    virtual void Activated();

    virtual void Deactivated();


  protected slots:


    void Skeletonize();
    void Project();



  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    void InitPointsets();

    void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

    berry::ISelectionListener::Pointer m_SelListener;
    berry::IStructuredSelection::ConstPointer m_CurrentSelection;

    bool m_IsInitialized;

    Ui::QmitkTbssSkeletonizationViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    void AddToDataStorage(mitk::Image* img, std::string name);

    Float4DImageType::Pointer ConvertToItk(mitk::Image::Pointer image);


};



#endif // _QMITKTbssSkeletonizationVIEW_H_INCLUDED

