/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkMainTemplate.h"

class QmitkDataManager;
class QmitkRegistration;
class ReggiPerfusionMatching;
class QmitkSimpleExampleFunctionality;
class QmitkSimpleTumorSegmentation;
class QmitkLevelSets;
class QmitkNoiseSuppression;
class QmitkSimplexMeshTool;

/*!
	\brief This application demonstrates how to code a new MITK application 

*/
class SampleApp : public QmitkMainTemplate {
	Q_OBJECT

public:
		/*!
			\brief Default constructor
		*/
    SampleApp( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    
		/*!
			\brief Default destructor
		*/
    ~SampleApp();

	/*!
		\brief Overrides initializeFunctionality() from QmitKMainTemplate

		In this method the user adds the application specific functionalities
		to the application template
	 */
	virtual void initializeFunctionality();

  virtual void initializeQfm();

public slots:

  void SetWidgetPlanesEnabled(bool enable);

protected:

		QmitkSimpleExampleFunctionality * simpleFunc;
		QmitkDataManager * m_DataManager;

		/*!
		 AppModule for image fusion
		 */
		ReggiPerfusionMatching * registrationFunc;

    QmitkSimpleTumorSegmentation * tumorSegFunc;

    /*!
		 functionality for noise filters
		 */
		QmitkNoiseSuppression * m_NoiseFilters;

    /*!
		 level set segmentation tool
		 */
		QmitkLevelSets * m_LevelSetTool;

    
    QmitkSimplexMeshTool * m_SimplexMeshTool;

    bool m_ControlsLeft;


};
