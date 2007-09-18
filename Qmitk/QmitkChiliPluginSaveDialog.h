/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2007-07-11 10:05:39 +0200 (Mi, 11 Jul 2007) $
Version:   $Revision: 11237 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkChiliPluginSaveDialog_h_Included
#define QmitkChiliPluginSaveDialog_h_Included

#include <qdialog.h>
#include <mitkDataTreeNode.h>

class QListView;
class QLineEdit;
class QRadioButton;
class QGridLayout;
class QBoxLayout;

class QmitkChiliPluginSaveDialog : public QDialog
{
  Q_OBJECT

  public:

    typedef enum { OverrideAll, AddOnlyNew, CreateNew } DataType;

    /** This struct is the basic ReturnValue. Which Series- and StudyOID select the user and what should happen with the nodes.  */
    struct ReturnValue
    {
      std::string StudyOID;
      std::string SeriesOID;
      DataType UserDecision;
    };

    /** This struct contains the information if a new series should created. */
    struct NewSeriesInformation
    {
      std::string SeriesDescription;
      int SeriesNumber;
    };

    /** constructor and destructor */
    QmitkChiliPluginSaveDialog( QWidget* parent = 0, const char* name = 0 );
    virtual ~QmitkChiliPluginSaveDialog();

    /** This function adds a study to the dialog. Use this function if the user selected a study only. */
    void AddStudy( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription );

    /** This function adds a study and series. Use this function if the user select a study and series. */
    void AddStudyAndSeries( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription );

    /** This function adds a mitk::DataTreeNode which was loaded from chili. We have to know from which study and series the node
    was loaded. Therefore you have to set all. Use this function for a mitk::DataTreeNode with SeriesOID-Property. */
    void AddStudySeriesAndNode( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription, mitk::DataTreeNode::Pointer node );

    /** This function add the node only. This node wasnt saved in chili bevor. So there are no series- and study-relationships.
    Use this function for a mitk::DataTreeNode without SeriesOID-Property. */
    void AddNode( mitk::DataTreeNode::Pointer node );

    /** This function return the user-selction. */
    ReturnValue GetSelection();
    /** If the user want to create a new series, this function return the SeriesNumber and -Description. */
    NewSeriesInformation GetSeriesInformation();

  public slots:

    /** This slot ensure that a study is selected, show all series to the selected study and call SetSeries().
    IMPORTANT: This slot have to call one time bevor exec the dialog. */
    void UpdateView();
    /** This slot ensure that a series is selected. If no series exist, the option to create a new series is set to the one and only. Call SetNodesByButtonGroup(). */
    void SetSeries();

  protected slots:

    /** This slot enable and strikeout the nodes in subject to the selected radiobutton. The lineedit for the SeriesDescription and -Number 
    get hide and show. */
    void SetNodesByButtonGroup();

    /** This slot check the ouput. Thats include all node-names. They get set via the lineedits whereas they should not be empty. If the user want to create a new series, the seriesDescription get checked too. */
    void CheckOutputs();

    void ShowHelp();

  protected:

  /** Internal struct to handle the input. */
  struct DifferentInputs
  {
    std::string StudyOID;
    std::string SeriesOID;
    std::string PatientName;
    std::string PatientID;
    std::string StudyDescription;
    std::string SeriesNumber;
    std::string SeriesDescription;
    mitk::DataTreeNode::Pointer Node;
    QLineEdit* NodeDescriptionField;
  };
  /** All inputs. */
  std::list< DifferentInputs > m_Inputs;

  /** Show the different Studies. */
  QListView* m_StudyListView;
  /** Show the different Series. */
  QListView* m_SeriesListView;
  /** Container to show the Nodes. */
  QBoxLayout* m_NodeLayout;
  /** The RadioButton to select whats happens with the nodes. */
  QRadioButton* m_Override;
  QRadioButton* m_Add;
  QRadioButton* m_New;
  /** The LineEdits for the SeriesAttributes if a new one created. */
  QLineEdit* m_SeriesNumber;
  QLineEdit* m_SeriesDescription;
};

#endif

