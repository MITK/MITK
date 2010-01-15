/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYFLAGS_H_
#define CHERRYFLAGS_H_

#include "cherryOSGiDll.h"

namespace cherry {

class CHERRY_OSGI Flag
{
    int i;
public:
    inline Flag(int i);
    inline operator int() const { return i; }
};

inline Flag::Flag(int ai) : i(ai) {}

class CHERRY_OSGI IncompatibleFlag
{
    int i;
public:
    inline explicit IncompatibleFlag(int i);
    inline operator int() const { return i; }
};

inline IncompatibleFlag::IncompatibleFlag(int ai) : i(ai) {}


template<typename Enum>
class Flags
{
    typedef void **Zero;
    int i;
public:
    typedef Enum enum_type;
    inline Flags(const Flags &f) : i(f.i) {}
    inline Flags(Enum f) : i(f) {}
    inline Flags(Zero = 0) : i(0) {}
    inline Flags(Flag f) : i(f) {}

    inline Flags &operator=(const Flags &f) { i = f.i; return *this; }
    inline Flags &operator&=(int mask) { i &= mask; return *this; }
    inline Flags &operator&=(unsigned int mask) { i &= mask; return *this; }
    inline Flags &operator|=(Flags f) { i |= f.i; return *this; }
    inline Flags &operator|=(Enum f) { i |= f; return *this; }
    inline Flags &operator^=(Flags f) { i ^= f.i; return *this; }
    inline Flags &operator^=(Enum f) { i ^= f; return *this; }

    inline operator int() const { return i; }

    inline Flags operator|(Flags f) const { Flags g; g.i = i | f.i; return g; }
    inline Flags operator|(Enum f) const { Flags g; g.i = i | f; return g; }
    inline Flags operator^(Flags f) const { Flags g; g.i = i ^ f.i; return g; }
    inline Flags operator^(Enum f) const { Flags g; g.i = i ^ f; return g; }
    inline Flags operator&(int mask) const { Flags g; g.i = i & mask; return g; }
    inline Flags operator&(unsigned int mask) const { Flags g; g.i = i & mask; return g; }
    inline Flags operator&(Enum f) const { Flags g; g.i = i & f; return g; }
    inline Flags operator~() const { Flags g; g.i = ~i; return g; }

    inline bool operator!() const { return !i; }

    inline bool TestFlag(Enum f) const { return (i & f) == f; }

};

} // namespace cherry

#endif /* CHERRYFLAGS_H_ */
