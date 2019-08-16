/*
 *  cHardwareStatusPrinter.cc
 *  Avida
 *
 *  Called "hardware_status_printer.cc" prior to 11/30/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1999-2004 California Institute of Technology.
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

#include "cHardwareStatusPrinter.h"

#include "cAvidaContext.h"
#include "cHardwareBase.h"
#include "cHeadCPU.h"
#include "cInstSet.h"
#include "cOrganism.h"
#include "cString.h"
#include "cStringUtil.h"

void cHardwareStatusPrinter::TraceHardware(cAvidaContext& ctx, cHardwareBase& hardware, bool bonus, bool minitrace, const int exec_success)
{
  cOrganism* organism = hardware.GetOrganism();
  if (exec_success == -2) {cString next_name(hardware.GetInstSet().GetName(hardware.IP().GetInst()));
    if (bonus) next_name = cStringUtil::Stringf("%s (bonus instruction)", static_cast<const char*>(next_name));
    
    if (organism) {
      if (!minitrace) organism->PrintStatus(m_trace_fp, next_name);
      else if (minitrace && exec_success == -2) organism->PrintMiniTraceStatus(ctx, m_trace_fp, next_name);
    }
  }
  else if (minitrace && exec_success != -2 && organism) organism->PrintMiniTraceSuccess(m_trace_fp, exec_success);
}

void cHardwareStatusPrinter::TraceTestCPU(int time_used, int time_allocated, const cOrganism& organism)
{
  organism.PrintFinalStatus(m_trace_fp, time_used, time_allocated);
}
