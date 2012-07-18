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

/**
* \brief This is a class for showing how to document your code using doxygen.
*
* The more detailed description is needed for some more elaborate description. Here you can describe
* anything anyone might ever want to know about your class. Of especial interest might be to mention
* what it can be used for or what its main purpose is. If you want you can even use pictures (you migth want
* take a look at the doxygen documentation for that).
*/
class DocumentationExample
{
public:

  /**
  * \brief A constructor.
  * A more elaborate description of the constructor.
  */
  DocumentationExample();
  /**
  * \brief A destructor.
  * A more elaborate description of the destructor.
  */
  ~DocumentationExample();

  /**
  * \brief Casts the char at position number to an int and returns it
  *
  * This function will take an int number and a char pointer name, then will try to access the int a position number
  * and cast it to an int and return it. No verification is done within the function to ensure, that it is a valid position.
  *
  * @param number The position of the char to be cast. This should never be below 0 and not above the length of the char array.
  * @param name A constant character pointer.
  * @return The char value of the charakter at position number casted to int
  */
  int ExampleCastCharToInt(int number,const char *name);

  /**
  * \brief Tests whether an integer is within the bounds of a string
  *
  * This will perform a check whether an int is bigger than an arbitrary zero and smaller than the length of the string.
  * @param itsAString The string to be checked as reference
  * @param theInt The position to be checked for
  * @exception std::out_of_range parameter is out of range.
  * @see zero()
  * @return True if character is in range, should never return false but throw an exception instead.
  */
  bool TestWithin(std::string itsAString,int theInt) throw(std::out_of_range);

  /**
  * \brief The public definition of zero
  *
  * This variable is used to determine what will be regarded as zero.
  */
  int zero;

};
