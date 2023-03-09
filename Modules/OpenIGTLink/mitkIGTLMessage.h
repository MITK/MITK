/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIGTLMessage_h
#define mitkIGTLMessage_h

#include <itkDataObject.h>
#include "MitkOpenIGTLinkExports.h"
#include <mitkNumericTypes.h>
#include <mitkCommon.h>

#include "igtlMessageBase.h"

namespace mitk {

    /**Documentation
    * \brief A wrapper for the OpenIGTLink message type
    *
    * This class represents the data object that is passed through the
    * MITK-OpenIGTLink filter pipeline. It wraps the OpenIGTLink message type.
    * Additionally, it contains a data structure that contains error/plausibility
    * information.
    *
    */
  class MITKOPENIGTLINK_EXPORT IGTLMessage : public itk::DataObject
    {
    public:
      mitkClassMacroItkParent(IGTLMessage, itk::DataObject);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);
      mitkNewMacro1Param(Self, igtl::MessageBase::Pointer);

      /**
      * \brief type that holds the time at which the data was recorded in milliseconds
      */
      typedef double TimeStampType;

      /**
      * \brief Sets the OpenIGTLink message
      */
      void SetMessage(igtl::MessageBase::Pointer msg);
      /**
      * \brief returns the OpenIGTLink message
      */
      itkGetConstMacro(Message, igtl::MessageBase::Pointer);
      /**
      * \brief returns true if the object contains valid data
      */
      virtual bool IsDataValid() const;
      /**
      * \brief sets the dataValid flag of the IGTLMessage object indicating if
      * the object contains valid data
      */
      itkSetMacro(DataValid, bool);
      /**
      * \brief gets the IGT timestamp of the IGTLMessage object
      */
      itkGetConstMacro(IGTTimeStamp, TimeStampType);
      /**
      * \brief set the name of the IGTLMessage object
      */
      itkSetStringMacro(Name);
      /**
      * \brief returns the name of the IGTLMessage object
      */
      itkGetStringMacro(Name);

      /**
      * \brief Graft the data and information from one IGTLMessage to another.
      *
      * Copies the content of data into this object.
      * This is a convenience method to setup a second IGTLMessage object with
      * all the meta information of another IGTLMessage object.
      * Note that this method is different than just using two
      * SmartPointers to the same IGTLMessage object since separate DataObjects
      * are still maintained.
      */
      void Graft(const DataObject *data) override;

      /**
      * \brief copy meta data of a IGTLMessage object
      *
      * copies all meta data from IGTLMessage data to this object
      */
      void CopyInformation(const DataObject* data) override;

      /**
      * \brief Prints the object information to the given stream os.
      * \param os The stream which is used to print the output.
      * \param indent Defines the indentation of the output.
      */
      void PrintSelf(std::ostream& os, itk::Indent indent) const override;

      std::string ToString() const;

      /** Compose with another IGTLMessage
       *
       * This method composes self with another IGTLMessage of the
       * same dimension, modifying self to be the composition of self
       * and other.  If the argument pre is true, then other is
       * precomposed with self; that is, the resulting transformation
       * consists of first applying other to the source, followed by
       * self.  If pre is false or omitted, then other is post-composed
       * with self; that is the resulting transformation consists of
       * first applying self to the source, followed by other. */
      void Compose(const mitk::IGTLMessage::Pointer n, const bool pre = false);

      /** Returns the OpenIGTL Message type
       **/
      const char* GetIGTLMessageType() const;

      template < typename IGTLMessageType > IGTLMessageType* GetMessage() const;

    protected:
      mitkCloneMacro(Self);

      IGTLMessage();

      /**
       * Copy constructor internally used.
       */
      IGTLMessage(const mitk::IGTLMessage& toCopy);

      /**
       * Creates a IGTLMessage object from an igtl::MessageBase and a given name.
       */
      IGTLMessage(igtl::MessageBase::Pointer message);

      ~IGTLMessage() override;

      /**
      * \brief holds the actual OpenIGTLink message
      */
      igtl::MessageBase::Pointer m_Message;

      /**
      * \brief defines if the object contains valid values
      */
      bool m_DataValid;
      /**
      * \brief contains the time at which the tracking data was recorded
      */
      TimeStampType m_IGTTimeStamp;
      /**
      * \brief name of the navigation data
      */
      std::string m_Name;

    private:

      // pre = false
      static mitk::IGTLMessage::Pointer getComposition(
          const mitk::IGTLMessage::Pointer nd1,
          const mitk::IGTLMessage::Pointer nd2);

      /**
      * \brief sets the IGT timestamp of the IGTLMessage object
      */
      itkSetMacro(IGTTimeStamp, TimeStampType);

    };

    /**
      * @brief Equal A function comparing two OpenIGTLink message objects for
      * being equal in meta- and imagedata
      *
      * @ingroup MITKTestingAPI
      *
      * Following aspects are tested for equality:
      *  - TBD
      *
      * @param rightHandSide An IGTLMessage to be compared
      * @param leftHandSide An IGTLMessage to be compared
      * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
      * @param verbose Flag indicating if the user wants detailed console output
      * or not.
      * @return true, if all subsequent comparisons are true, false otherwise
      */
      MITKOPENIGTLINK_EXPORT bool Equal( const mitk::IGTLMessage& leftHandSide,
                                          const mitk::IGTLMessage& rightHandSide,
                                          ScalarType eps = mitk::eps,
                                          bool verbose = false );

} // namespace mitk
#endif
