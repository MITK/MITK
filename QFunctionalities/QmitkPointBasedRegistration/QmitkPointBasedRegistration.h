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
\brief The PointBasedRegistration functionality is used to perform point based registration.

This functionality allows you to register 2D as well as 3D images in a rigid and deformable manner via corresponding 
PointSets. Register means to align two images, so that they become as similar as possible. 
Therefore you have to set corresponding points in both images, which will be matched. The movement, which has to be 
performed on the points to align them will be performed on the moving image as well. The result is shown in the multi-widget. 

For more informations see: \ref QmitkPointBasedRegistrationUserManual

\sa QmitkFunctionality
\ingroup Functionalities
\ingroup PointBasedRegistration
*/
class QmitkPointBasedRegistration : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  

    typedef std::set<mitk::DataTreeNode*> invisibleNodesList;

    /*!  
    \brief Default constructor  
    */  
    QmitkPointBasedRegistration(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

    /*!  
    \brief Default destructor  
    */  
    virtual ~QmitkPointBasedRegistration();

    /*!  
    \brief Method for creating the widget containing the application   controls, like sliders, buttons etc.  
    */  
    virtual QWidget * CreateControlWidget(QWidget *parent);

    /*!  
    \brief Method for creating the applications main widget  
    */  
    virtual QWidget * CreateMainWidget(QWidget * parent);

    /*!  
    \brief Method for creating the connections of main and control widget  
    */  
    virtual void CreateConnections();

    /*!  
    \brief Method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
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
       \brief Helper method for testing
    */
    bool TestAllTools();
    
  protected slots:
    /**
       \brief Helper method for testing
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
    
    /*!
    \brief Called whenever the data tree has changed. 
    */
    void TreeChanged();
    
    /*!  
    \brief Sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataTreeIteratorClone imageIt);
    
    /*!  
    \brief Sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataTreeIteratorClone imageIt);

    /*!  
    \brief Calculates registration with vtkLandmarkTransform
    */
    void calculateLandmarkbased(PointBasedRegistrationControlParameters* params);

    /*!  
    \brief Calculates registration with itkLandmarkWarping
    */
    void calculateLandmarkWarping(PointBasedRegistrationControlParameters* params);
    
    /*!  
    \brief Calculates registration with ICP and vtkLandmarkTransform
    */
    void calculateLandmarkbasedWithICP(PointBasedRegistrationControlParameters* params);
    
    /*!  
    \brief Adds mitk::PointSetInteractor for the moving image and removes mitk::PointSetInteractor from the fixed image, 
    lets the fixed image become invisible and the moving image visible
    */
    void addMovingInteractor();
    
    /*!  
    \brief Adds mitk::PointSetInteractor for the fixed image  and removes mitk::PointSetInteractor from the fixed image, 
    lets the moving image become invisible and the fixed image visible
    */
    void addFixedInteractor();

    /*!  
    \brief Checks if registration is possible
    */
    bool CheckCalculate();

    /*!  
    \brief Saves the registration result.
    */
    void SaveModel();
    
    /*!  
    \brief Performs an undo for the last transform.
    */
    void UndoTransformation();
    
    /*!  
    \brief Performs a redo for the last undo transform.
    */
    void RedoTransformation();

    /*!  
    \brief Removes all points from fixed and moving pointset
    */
    void ResetPointsets();

    /*!  
    \brief Stores whether the image will be shown in grayvalues or in red for fixed image and green for moving image

    @param if true, then images will be shown in red and green
    */
    void showRedGreen(bool show);

    /*!  
    \brief Removes mitk::PointSetInteractor from both images and let both images become visible
    */
    void bothSelected();

    /*!  
    \brief Sets the selected opacity for moving image
    
    @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!  
    \brief Updates the moving landmarks in QmitkPointBasedRegistrationControls widget 
    */
    void updateMovingLandmarksList();

    /*!  
    \brief Updates the fixed landmarks in QmitkPointBasedRegistrationControls widget 
    */
    void updateFixedLandmarksList();

    /*!  
    \brief Sets the point to be selected and jumps to the position in moving image according to this point
    
    @param pointID the point ID of the selected point set point
    */
    void movingLandmarkSelected(int pointID);
    
    /*!  
    \brief Sets the point to be selected and jumps to the position in fixed image according to this point
    
    @param pointID the point ID of the selected point set point
    */
    void fixedLandmarkSelected(int pointID);

    /*!  
    \brief Sets the images to gray values or fixed image to red and moving image to green
    
    @param if true, then images will be shown in red and green
    */
    void setImageColor(bool redGreen);

    /*!  
    \brief Removes the fixed mitk::PointSetInteractor
    */
    void removeFixedInteractor();

    /*!  
    \brief Removes the fixed PointSetObserver
    */
    void removeFixedObserver();

    /*!  
    \brief Removes the moving mitk::PointSetInteractor
    */
    void removeMovingInteractor();

    /*!  
    \brief Removes the moving PointSetObserver
    */
    void removeMovingObserver();

    /*!  
    \brief Clears the undo and redo transformation lists.
    */
    void clearTransformationLists();

    /*!  
    \brief Calculates the landmark error for the selected transformation.
    */
    void checkLandmarkError();
    
    /*!  
    \brief Changes the transformation type and calls checkLandmarkError().
    */
    void transformationChanged(int transform);

    /*!  
    \brief Sets whether all other data tree nodes except the two selected should be invisible or not.

    True = all nodes invisible, false = all nodes visible.
    */
    void setInvisible(bool invisible);

    /*!  
    \brief Opens a dialog box to select a file containing a point set for the fixed image.
    */
    void loadFixedPointSet();

    /*!  
    \brief Opens a dialog box to select a file containing a point set for the moving image.
    */
    void loadMovingPointSet();

    /*!  
    \brief Opens a dialog box to save the fixed image point set.
    */
    void saveFixedPointSet();

    /*!  
    \brief Opens a dialog box to save the moving image point set.
    */
    void saveMovingPointSet();

  protected:

    // observer class to react on changes on point set nodes
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
    
      // if landmarks were added or removed, then update landmark lists
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

      // @param fixed if true, this observer belongs to the fixed point set, otherwise to the moving point set
      void SetFixed(bool fixed)
      {
        m_FixedImage = fixed;
      }

    private: 
      
      // holds the QmitkPointBasedRegistration class to call function for updates on landmarks
      QmitkPointBasedRegistration*  m_Parent;

      // stores the information whether this observer belongs to the fixed point set or to the moving point set
      bool m_FixedImage;
    };

    /*!  
    * default main widget containing 4 windows showing 3   
    * orthogonal slices of the volume and a 3d render window  
    */  
    QmitkStdMultiWidget * m_MultiWidget;

    /*!  
    * control widget to make all changes for point based registration 
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
