/*
 *  cAnalyzeFlowCommandDef.h
 *  Avida
 *
 *  Called "analyze_flow_command_def.hh" prior to 12/2/05.
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

#ifndef cAnalyzeFlowCommandDef_h
#define cAnalyzeFlowCommandDef_h

#ifndef cAnalyzeCommand_h
#include "cAnalyzeCommand.h"
#endif
#ifndef cAnalyzeCommandDefBase_h
#include "cAnalyzeCommandDefBase.h"
#endif
#ifndef cString_h
#include "cString.h"
#endif
#ifndef tList_h
#include "tList.h"
#endif

class cAnalyze;


class cAnalyzeFlowCommandDef : public cAnalyzeCommandDefBase
{
private:
  void (cAnalyze::*CommandFunction)(cString, tList<cAnalyzeCommand>&);
  
public:
  cAnalyzeFlowCommandDef(const cString& name, void (cAnalyze::*cf)(cString, tList<cAnalyzeCommand>&))
    : cAnalyzeCommandDefBase(name), CommandFunction(cf) { ; }
  virtual ~cAnalyzeFlowCommandDef() { ; }
			 
  virtual void Run(cAnalyze* analyze, const cString& args, cAnalyzeCommand& command, Feedback& feedback) const
  {
    (analyze->*CommandFunction)(args, *(command.GetCommandList()));
  }

  virtual bool IsFlowCommand() { return true; }
};

#endif
