/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
class QLabel;
class QVBoxLayout;

class QMITK_EXPORT QmitkChiliPluginSaveDialog : public QDialog
{
  Q_OBJECT

  public:

    typedef enum { OverrideAll, AddOnlyNew, CreateNew } DataType;

    /** This struct is the basic ReturnValue. Where ( Series- and StudyOID ) should all nodes save and how ( new, add, override ). */
    struct ReturnValue
    {
      std::string StudyOID;
      std::string SeriesOID;
      DataType UserDecision;
    };

    /** This struct contains the information to create a new series. */
    struct NewSeriesInformation
    {
      std::string SeriesDescription;
      int SeriesNumber;
    };

    /*!
    \brief Constructor.
    The MainWidget get created via the constuctor.
    */
    QmitkChiliPluginSaveDialog( QWidget* parent = 0, const char* name = 0 );

    /*! \brief Destructor. */
    virtual ~QmitkChiliPluginSaveDialog();

    /*!
    \brief This function add a study to the dialog.
    @param studyOID   The studyOID.
    @param patientName   The patientName.
    @param patientID   The patientID.
    @param studyDescription   The studyDesription.
    If you want to add the current selected study as target to save, then use this function.
    */
    void AddStudy( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription );

    /*!
    \brief This function add a study and series to the dialog.
    @param studyOID   The studyOID.
    @param patientName   The patientName.
    @param patientID   The patientID.
    @param studyDescription   The studyDesription.
    @param seriesOID   The seriesOID.
    @param seriesNumber   The seriesNumber.
    @param seriesDescription   The seriesDesription.
    If you want to add the current selected study and series as target to save, then use this function.
    */
    void AddStudyAndSeries( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription );

    /*!
    \brief This function add a study, series and node to the dialog.
    @param studyOID   The studyOID.
    @param patientName   The patientName.
    @param patientID   The patientID.
    @param studyDescription   The studyDesription.
    @param seriesOID   The seriesOID.
    @param seriesNumber   The seriesNumber.
    @param seriesDescription   The seriesDesription.
    @param node   The mitk::DataTreeNode.
    A node which was loaded from chili have a seriesOID as property. With this property you can get the series and study where the node load from. You can set this attributes to the dialog. Then the user can save to this series and study.
    */
    void AddStudySeriesAndNode( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription, mitk::DataTreeNode::Pointer node );

    /*!
    \brief This function add a node to the dialog.
    @param node   The mitk::DataTreeNode.
    There are nodes which was not loaded from chili. This one have no seriesOID property. But we want to save them too. So we add only the node.
    If you add only nodes without a series or study, its not possible to save them. Because the user have no study and series to select. Therefore add the current selected study or series!
    */
    void AddNode( mitk::DataTreeNode::Pointer node );

    /*!
    \brief This function return the user-selection.
    @returns The struct ReturnValue.
    */
    ReturnValue GetSelection();

    /*!
    \brief This function return the seriesNumber and -Description.
    @returns The struct NewSeriesInformation.
    If the user want to create a new series, he can enter a SeriesNumber and -Description. This function return them.
    */
    NewSeriesInformation GetSeriesInformation();

  public slots:

    /*!
    \brief This function refresh the dialog.
    This slot ensure that a study is selected, show all series to the selected study and call SetSeries().
    IMPORTANT: This slot have to call one time bevor exec the dialog.
    */
    void UpdateView();

  protected slots:

    /** This slot ensure that a series is selected. If no series exist, the option to create a new series is set to the one and only. Call SetNodesByButtonGroup(). */
    void SetSeries();

    /** This slot enable and strikeout the nodes in subject to the selected radiobutton. The lineedit for the SeriesDescription and -Number 
    get hide or show. */
    void SetNodesByButtonGroup();

    /** If the user want to create a new series, this slot check if the seriesDescription is not empty. */
    void CheckOutputs();

    /** This slot show a messagebox with helpinformation. */
    void ShowHelp();

  protected:

  /** Internal struct to handle the input. */
  struct SeriesInputs
  {
    std::string StudyOID;
    std::string SeriesOID;
    std::string SeriesNumber;
    std::string SeriesDescription;
  };
  /** All inputs. */
  std::list< SeriesInputs > m_SeriesInputs;

  /** Internal struct to handle the nodes. */
  struct NodeInputs
  {
    std::string SeriesOID;
    bool canBeOverride;
    QLabel* usedLabel;
  };
  /** All nodes. */
  std::list< NodeInputs > m_NodeInputs;

  /** Show the different Studies. */
  QListView* m_StudyListView;
  /** Show the different Series. */
  QListView* m_SeriesListView;
  /** The layoout for the nodes to save. */
  QVBoxLayout* m_NodeLayout;
  /** The RadioButton to select whats happens with the nodes. */
  QRadioButton* m_Override;
  QRadioButton* m_Add;
  QRadioButton* m_New;
  /** The LineEdits for the SeriesAttributes if a new one created. */
  QLineEdit* m_SeriesNumber;
  QLineEdit* m_SeriesDescription;
};

#endif

