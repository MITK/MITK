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


#ifndef _mitkLabelSet_H_
#define _mitkLabelSet_H_

#include <MitkExports.h>
#include <mitkCommon.h>
#include <mitkMessage.h>
#include <mitkLookupTable.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkLabel.h>

namespace mitk
{
//
//Documentation
// @brief LabelSet containing the labels corresponding to a segmentation session.
// @ingroup Data
//
class MITK_CORE_EXPORT LabelSet : public itk::Object
{
public:

    mitkClassMacro( LabelSet, itk::Object );

    itkNewMacro( Self );

    mitkCloneMacro(LabelSet);

    typedef std::vector <mitk::Label::Pointer>            LabelContainerType;
    typedef LabelContainerType::const_iterator            LabelContainerConstIteratorType;
    typedef LabelContainerType::iterator                  LabelContainerIteratorType;

    //Documentation
    // @brief equality operator implementation
    //
    virtual bool operator==( const LabelSet& LabelSet ) const;

    /*!
    * \brief non equality operator implementation
    */
    virtual bool operator!=( const LabelSet& LabelSet ) const;

    /*!
    * \brief implementation necessary because operator made
    * private in itk::Object
    */
    virtual LabelSet& operator=( const LabelSet& LabelSet );

    /** \brief Returns a const LabelMap iterator at the begining.
    */
    LabelContainerConstIteratorType IteratorBegin();

    /** \brief Returns a const LabelMap iterator at the end.
    */
    LabelContainerConstIteratorType IteratorEnd();

    itkSetStringMacro(LastModified);
    itkGetStringMacro(LastModified);

    itkSetStringMacro(Owner);
    itkGetStringMacro(Owner);

    itkGetStringMacro(Name);
    itkSetStringMacro(Name);

    bool HasLabel(int) const;

    void SetActiveLabel(int);

    void RemoveLabel(int);

    static bool IsSelected(mitk::Label::Pointer label);

    void RemoveSelectedLabels();

    void RemoveLabels(int begin, int count);

    void AddLabel(const mitk::Label& label);

    void AddLabel(const std::string& name, const mitk::Color& color);

    void RenameLabel(int, const std::string&, const mitk::Color&);

    virtual int GetNumberOfLabels() const;

    virtual void SetAllLabelsVisible(bool);

    virtual void SetAllLabelsLocked(bool);

    virtual void RemoveAllLabels();

    virtual void SetLabelVisible(int index, bool value);
    virtual bool GetLabelVisible(int index);

    virtual void SetLabelLocked(int index, bool value);
    virtual bool GetLabelLocked(int index);

    virtual void SetLabelSelected(int index, bool value);
    virtual bool GetLabelSelected(int index);

    virtual void SetLabelOpacity(int index, float value);
    virtual float GetLabelOpacity(int index);

    virtual void SetLabelVolume(int index, float value);
    virtual float GetLabelVolume(int index);

    virtual void SetLabelName(int index, const std::string &name);
    virtual std::string GetLabelName(int index);

    virtual void SetLabelColor(int index, const mitk::Color &color);
    virtual const mitk::Color& GetLabelColor(int index);

    virtual const mitk::Label* GetActiveLabel() const { return m_ActiveLabel; };

    virtual mitk::Label::ConstPointer GetLabel(int index) const;

    virtual int GetActiveLabelIndex() const;

    virtual void ResetLabels();


    LabelSet();
    virtual ~LabelSet();

protected:

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    LabelSet(const LabelSet& other);

    LabelContainerType m_LabelContainer;

    mitk::Label* m_ActiveLabel;

    std::string m_Owner;

    std::string m_LastModified;

    std::string m_Name;

};

} // namespace mitk

#endif // _mitkLabelSet_H_
