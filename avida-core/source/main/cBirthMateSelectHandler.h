/*
 *  cBirthMateSelectHandler.h
 *  Avida
 *
 *  Created by David Bryson on 4/1/09.
 *  Copyright 2009-2011 Michigan State University. All rights reserved.
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

#ifndef cBirthMateSelectHandler_h
#define cBirthMateSelectHandler_h

#ifndef cBirthEntry_h
#include "cBirthEntry.h"
#endif
#ifndef cBirthSelectionHandler_h
#include "cBirthSelectionHandler.h"
#endif
#ifndef tArray_h
#include "tArray.h"
#endif

class cBirthChamber;


class cBirthMateSelectHandler : public cBirthSelectionHandler
{
private:
  cBirthChamber* m_bc;
  tArray<cBirthEntry> m_entries;
  cBirthEntry m_non_ms_entry;
  
  
public:
  cBirthMateSelectHandler(cBirthChamber* bc) : m_bc(bc) { ; }
  ~cBirthMateSelectHandler();
  
  cBirthEntry* SelectOffspring(cAvidaContext& ctx, const Genome& offspring, cOrganism* parent);
};

#endif

