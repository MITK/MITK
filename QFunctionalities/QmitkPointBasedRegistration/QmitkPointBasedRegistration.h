/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
 
#if !defined(QMITK_POINTBASEDREGISTRATION_H__INCLUDED)
#define QMITK_POINTBASEDREGISTRATION_H__INCLUDED

#include "QmitkFunctionality.h"

#include "mitkTestingConfig.h" // IMPORTANT: this defines or undefines BUILD_TESTING !


#include "PointBasedRegistrationControlParameters.h"
#include <mitkPointSetInteractor.h>
#include <mitkGlobalInteraction.h>
#include <mitkAffineInteractor.h>
#include <mitkPointSet.h>
#include <vtkCellArray.h>
#include <vtkLandmarkTransform.h>
#include "QmitkMessageBoxHelper.h"

class QmitkStdMultiWidget;
class QmitkPointBasedRegistrationControls;

/*!
\brief PointBasedRegistration 

One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
and CreateAction(..) from QmitkFunctionality. 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkPointBasedRegistration : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  

    typedef std::set<mitk::DataTreeNode*> invisibleNodesList;

    /*!  
    \brief default constructor  
    */  
    QmitkPointBasedRegistration(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    /*!  
    \brief default destructor  
    */  
    virtual ~QmitkPointBasedRegistration();

    /*!  
    \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
    */  
    virtual QWidget * CreateControlWidget(QWidget *parent);

    /*!  
    \brief method for creating the applications main widget  
    */  
    virtual QWidget * CreateMainWidget(QWidget * parent);

    /*!  
    \brief method for creating the connections of main and control widget  
    */  
    virtual void CreateConnections();

    /*!  
    \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
    */  
    virtual QAction * CreateAction(QActionGroup *parent);

    virtual void Activated();
    virtual void Deactivated();

#ifdef BUILD_TESTING
    /**
      \brief Testing entry point
    */
    virtual int TestYourself();

    /**
       \brief helper method for testing
    */
    bool TestAllTools();
    
  protected slots:
    /**
       \brief helper method for testing
    */
    void RegistrationErrorDialogFound( QWidget* widget );

  private:
    bool m_MessageBox;


  public:
#else
    // slot function is needed, because moc ignores our #ifdefs
    void RegistrationErrorDialogFound( QWidget* widget ) {}
#endif

  protected slots:  
    
    void TreeChanged();
    
    /*
    * sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataTreeIteratorClone imageIt);
    
    /*
    * sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataTreeIteratorClone imageIt);

    /*
    * calculates registration with vtkLandmarkTransform
    */
    void calculateLandmarkbased(PointBasedRegistrationControlParameters* params);

    /*
    * calculates registration with itkLandmarkWarping
    */
    void calculateLandmarkWarping(PointBasedRegistrationControlParameters* params);
    
    /*
    * calculates registration with ICP and vtkLandmarkTransform
    */
    void calculateLandmarkbasedWithICP(PointBasedRegistrationControlParameters* params);
    
    /*
    * adds mitk::PointSetInteractor for the moving image and removes mitk::PointSetInteractor from the fixed image,
    * lets the fixed image become invisible and the moving image visible
    */
    void addMovingInteractor();
    
    /*
    * adds mitk::PointSetInteractor for the fixed image  and removes mitk::PointSetInteractor from the fixed image,
    * lets the moving image become invisible and the fixed image visible
    */
    void addFixedInteractor();

    /*
    * checks if registration is possible
    */
    bool CheckCalculate();

    void SaveModel();
    void UndoTransformation();
    void RedoTransformation();

    /*
    * removes all points from fixed and moving pointset
    */
    void ResetPointsets();

    /*
    * stores whether the image will be shown in grayvalues or in red for fixed image and green for moving image
    * @param if true, then images will be shown in red and green
    */
    void showRedGreen(bool show);

    /*
    * removes mitk::PointSetInteractor from both images and let both images become visible
    */
    void bothSelected();

    /*
    * set the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*
    * updates the moving landmarks in QmitkPointBasedRegistrationControls widget 
    */
    void updateMovingLandmarksList();

    /*
    * updates the fixed landmarks in QmitkPointBasedRegistrationControls widget 
    */
    void updateFixedLandmarksList();

    /*
    * sets the point to be selected and jumps to the position in moving image according to this point
    * @param pointID the point ID of the selected Pointsetpoint
    */
    void movingLandmarkSelected(int pointID);
    
    /*
    * sets the point to be selected and jumps to the position in fixed image according to this point
    * @param pointID the point ID of the selected Pointsetpoint
    */
    void fixedLandmarkSelected(int pointID);

    /*
    * sets the images to grayvalues or fixed image to red and moving image to green
    * @param if true, then images will be shown in red and green
    */
    void setImageColor(bool redGreen);

    /*
    * removes the fixed mitk::PointSetInteractor
    */
    void removeFixedInteractor();

    /*
    * removes the fixed PointSetObserver
    */
    void removeFixedObserver();

    /*
    * removes the moving mitk::PointSetInteractor
    */
    void removeMovingInteractor();

    /*
    * removes the moving PointSetObserver
    */
    void removeMovingObserver();

    void clearTransformationLists();

    void checkLandmarkError();
    
    void transformationChanged(int transform);

    void setInvisible(bool invisible);

    void loadFixedPointSet();

    void loadMovingPointSet();

    void saveFixedPointSet();

    void saveMovingPointSet();

  protected:

    // observerclass to react on changes on pointsetnodes
    class PointSetObserver : public itk::Command 
    {
    public:
      typedef  PointSetObserver         Self;
      typedef  itk::Command             Superclass;
      typedef  itk::SmartPointer<Self>  Pointer;

      itkNewMacro( Self );

    protected:
      PointSetObserver()  { };

    public:

      void Execute(itk::Object *object, const itk::EventObject & event)
      {
        Execute( (const itk::Object*) object, event );
      }
    
      // if landmarks were added or removed, then update landmarklists
      void Execute(const itk::Object * /*object*/, const itk::EventObject & /*event*/)
      {
        if (m_FixedImage)
        {
          m_Parent->updateFixedLandmarksList();
        }
        else
        {
          m_Parent->updateMovingLandmarksList();
        }
      }

      // @param parent the parent QmitkPointBasedRegistration class
      void SetParent(QmitkPointBasedRegistration* parent)
      {
        m_Parent = parent;
      }

      // @param fixed if true, this obsever belongs to the fixed pointset, otherwise to the moving pointset
      void SetFixed(bool fixed)
      {
        m_FixedImage = fixed;
      }

    private: 
      
      // holds the QmitkPointBasedRegistration class to call function for updates on landmarks
      QmitkPointBasedRegistration*  m_Parent;

      // stores the information whether this observer belongs to the fixed pointset or to the moving pointset
      bool m_FixedImage;
    };

    /*!  
    * default main widget containing 4 windows showing 3   
    * orthogonal slices of the volume and a 3d render window  
    */  
    QmitkStdMultiWidget * m_MultiWidget;

    /*!  
    * control widget to make all changes for pointbased registration 
    */  
    QmitkPointBasedRegistrationControls * m_Controls;
    mitk::PointSet::Pointer m_FixedLandmarks;
    mitk::PointSet::Pointer m_MovingLandmarks;
    PointSetObserver::Pointer m_FixedLandmarkObserver;
    PointSetObserver::Pointer m_MovingLandmarkObserver;
    mitk::PointSetInteractor::Pointer m_FixedInteractor;
    mitk::PointSetInteractor::Pointer m_MovingInteractor;
    mitk::GlobalInteraction::Pointer m_GlobalInteraction;
    mitk::DataTreeNode::Pointer m_MovingPointSetNode;
    mitk::DataTreeNode::Pointer m_FixedPointSetNode;
    mitk::DataTreeNode* m_MovingNode;
    mitk::DataTreeNode* m_FixedNode;
    mitk::AffineInteractor::Pointer m_AffineInteractor;
    std::list<mitk::Geometry3D::Pointer> m_UndoGeometryList;
    std::list<mitk::Geometry3D::Pointer> m_UndoPointsGeometryList;
    std::list<mitk::Geometry3D::Pointer> m_RedoGeometryList;
    std::list<mitk::Geometry3D::Pointer> m_RedoPointsGeometryList;
    bool m_SetInvisible;
    bool m_ShowRedGreen;
    bool m_WorkWithFixed;
    bool m_WorkWithMoving;
    bool m_WorkWithBoth;
    float m_Opacity;
    float m_OriginalOpacity;
    int m_OldMovingLayer;
    int m_NewMovingLayer;
    bool m_FixedLandmarkObserverSet;
    bool m_MovingLandmarkObserverSet;
    bool m_OldMovingLayerSet;
    bool m_NewMovingLayerSet;
    unsigned long m_FixedLandmarkAddedObserver;
    unsigned long m_FixedLandmarkRemovedObserver;
    unsigned long m_MovingLandmarkAddedObserver;
    unsigned long m_MovingLandmarkRemovedObserver;
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    invisibleNodesList m_InvisibleNodesList;
    int m_Transformation;
};
#endif // !defined(QMITK_POINTBASEDREGISTRATION_H__INCLUDED)
