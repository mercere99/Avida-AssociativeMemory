/*
 *  cPhenPlastUtil.h
 *  Avida
 *
 *  Created by David on 6/18/10.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
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

#ifndef cPhenPlastUtil_h
#define cPhenPlastUtil_h

namespace Avida {
  class Genome;
};

class cAvidaContext;
class cBioGroup;
class cPhenPlastSummary;
class cWorld;

template<class T> class tArray;

using namespace Avida;


class cPhenPlastUtil
{
private:
  cPhenPlastUtil(); // @not_implemented
  
public:
  static int GetNumPhenotypes(cAvidaContext& ctx, cWorld* world, cBioGroup* bg);
  static double GetPhenotypicEntropy(cAvidaContext& ctx, cWorld* world, cBioGroup* bg);
  static double GetTaskProbability(cAvidaContext& ctx, cWorld* world, cBioGroup* bg, int task_id);
  static const tArray<double>& GetTaskProbabilities(cAvidaContext& ctx, cWorld* world, cBioGroup* bg);
  static cPhenPlastSummary* TestPlasticity(cAvidaContext& ctx, cWorld* world, const Genome& mg);
};  


#endif
