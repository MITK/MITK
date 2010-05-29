// This file is part of QuadProg++: a C++ library implementing
// the algorithm of Goldfarb and Idnani for the solution of a (convex) 
// Quadratic Programming problem by means of an active-set dual method.
// Copyright (C) 2007-2009 Luca Di Gaspero.
// Copyright (C) 2009 Eric Moyer.  
//
// QuadProg++ is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QuadProg++ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with QuadProg++. If not, see <http://www.gnu.org/licenses/>.

#include "Array.h"

/**
  Index utilities
 */
namespace QuadProgPP{

std::set<unsigned int> seq(unsigned int s, unsigned int e)
{
	std::set<unsigned int> tmp;
	for (unsigned int i = s; i <= e; i++)
		tmp.insert(i);
	
	return tmp;
}

std::set<unsigned int> singleton(unsigned int i)
{
	std::set<unsigned int> tmp;
	tmp.insert(i);
	
	return tmp;
}

}
