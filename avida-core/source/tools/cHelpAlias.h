/*
 *  cHelpAlias.h
 *  Avida
 *
 *  Called "help_alias.hh" prior to 12/7/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef cHelpAlias_h
#define cHelpAlias_h

#ifndef cHelpEntry_h
#include "cHelpEntry.h"
#endif

class cHelpFullEntry;
class cHelpType;
class cString;

class cHelpAlias : public cHelpEntry {
private:
  cHelpFullEntry * full_entry;
public:
  cHelpAlias(const cString & _alias, cHelpFullEntry * _full_entry)
    : cHelpEntry(_alias), full_entry(_full_entry) { ; }
  ~cHelpAlias() { ; }

  const cString & GetKeyword() const;
  const cString & GetDesc() const;
  cHelpType * GetType() const;

  bool IsAlias() const { return true; }
};

#endif
