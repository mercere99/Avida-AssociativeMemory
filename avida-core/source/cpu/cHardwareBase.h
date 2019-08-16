/*
 *  cHardwareBase.h
 *  Avida
 *
 *  Called "hardware_base.hh" prior to 11/17/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1999-2003 California Institute of Technology.
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

#ifndef cHardwareBase_h
#define cHardwareBase_h

#include "avida/core/Sequence.h"

#include <cassert>
#include <climits>
#include <iostream>

#ifndef cInstSet_h
#include "cInstSet.h"
#endif
#ifndef tBuffer_h
#include "tBuffer.h"
#endif
#ifndef tSmartArray_h
#include "tSmartArray.h"
#endif

class cAvidaContext;
class cBioUnit;
class cCodeLabel;
class cCPUMemory;
class cHardwareTracer;
class cHeadCPU;
class cInstruction;
class cMutation;
class cOrganism;
class cString;
class cWorld;

using namespace std;
using namespace Avida;


class cHardwareBase
{
protected:
  cWorld* m_world;
  cOrganism* m_organism;            // Organism using this hardware.
  cInstSet* m_inst_set;             // Instruction set being used.
  cHardwareTracer* m_tracer;        // Set this if you want execution traced.
  cHardwareTracer* m_minitracer;    // Set this if you want execution traced in a condensed and tractable format.
  cString& m_minitrace_file;
  tSmartArray<char> m_microtracer;
  tSmartArray<int> m_navtraceloc;
  tSmartArray<int> m_navtracefacing;
  tSmartArray<int> m_navtraceupdate;
  bool m_microtrace;
  bool m_topnavtrace;
  bool m_reprotrace;

  // --------  Instruction Costs  ---------
  int m_inst_cost;
  int m_female_cost;
  tArray<int> m_inst_ft_cost;
  tArray<double> m_inst_energy_cost;
  tArray<double> m_inst_res_cost; 
  tArray<double> m_inst_fem_res_cost; 
  tArray<int> m_thread_inst_cost;
  tArray<int> m_thread_inst_post_cost;
  tArray<int> m_active_thread_costs;
  tArray<int> m_active_thread_post_costs;
  bool m_has_any_costs;
  bool m_has_costs;
  bool m_has_ft_costs;
  bool m_has_energy_costs;
  bool m_has_res_costs; 
  bool m_has_fem_res_costs; 
  int m_task_switching_cost;
  bool m_has_female_costs;
  bool m_has_choosy_female_costs;
  bool m_has_post_costs;

  // --------  Base Hardware Feature Support  ---------
  tSmartArray<int> m_ext_mem;
  bool m_implicit_repro_active;
  
	// --------  Bit masks  ---------
	static const unsigned int MASK_SIGNBIT = 0x7FFFFFFF;	
	static const unsigned int MASK24       = 0xFFFFFF;

	static const unsigned int MASKOFF_LOWEST16       = 0xFFFF0000;
	static const unsigned int MASKOFF_LOWEST15       = 0xFFFF8000;
	static const unsigned int MASKOFF_LOWEST14       = 0xFFFFC000;
	static const unsigned int MASKOFF_LOWEST13       = 0xFFFFE000;
	static const unsigned int MASKOFF_LOWEST12       = 0xFFFFF000;
	static const unsigned int MASKOFF_LOWEST8        = 0xFFFFFF00;
	static const unsigned int MASKOFF_LOWEST4        = 0xFFFFFFF0;
	
  cHardwareBase(); // @not_implemented
  cHardwareBase(const cHardwareBase&); // @not_implemented
  cHardwareBase& operator=(const cHardwareBase&); // @not_implemented

private: 
  cString null_str;

public:
  cHardwareBase(cWorld* world, cOrganism* in_organism, cInstSet* inst_set);
  virtual ~cHardwareBase() { ; }
  
  // interrupt types
  enum interruptTypes {MSG_INTERRUPT = 0, MOVE_INTERRUPT};
  
  // --------  Organism  ---------
  cOrganism* GetOrganism() { return m_organism; }
  const cInstSet& GetInstSet() const { return *m_inst_set; }


  // --------  Core Functionality  --------
  void Reset(cAvidaContext& ctx);
  virtual bool SingleProcess(cAvidaContext& ctx, bool speculative = false) = 0;
  virtual void ProcessBonusInst(cAvidaContext& ctx, const cInstruction& inst) = 0;

  int Divide_DoMutations(cAvidaContext& ctx, double mut_multiplier = 1.0, const int maxmut = INT_MAX);
  bool Divide_TestFitnessMeasures(cAvidaContext& ctx);
  
  // --------  Helper methods  --------
  virtual int GetType() const = 0;
  virtual bool SupportsSpeculative() const = 0;
  virtual void PrintStatus(std::ostream& fp) = 0;
  virtual void PrintMiniTraceStatus(cAvidaContext& ctx, std::ostream& fp, const cString& next_name) = 0;
  virtual void PrintMiniTraceSuccess(std::ostream& fp, const int exec_success) = 0;
  void SetTrace(cHardwareTracer* tracer) { m_tracer = tracer; }
  void SetMiniTrace(const cString& filename, const int gen_id, const cString& genotype);
  void SetMicroTrace() { m_microtrace = true; } 
  void SetTopNavTrace(bool nav_trace) { m_topnavtrace = nav_trace; }
  bool IsTopNavTrace() { return m_topnavtrace; }
  void SetReproTrace(bool repro_trace) { m_reprotrace = repro_trace; }
  bool IsReproTrace() { return m_reprotrace; }
  void RecordMicroTrace(const cInstruction& cur_inst);
  void PrintMicroTrace(int gen_id);
  void RecordNavTrace(bool use_avatar);
  tSmartArray<char>& GetMicroTrace() { return m_microtracer; }
  tSmartArray<int>& GetNavTraceLoc() { return m_navtraceloc; }
  tSmartArray<int>& GetNavTraceFacing() { return m_navtracefacing; }
  tSmartArray<int>& GetNavTraceUpdate() { return m_navtraceupdate; }
  void DeleteMiniTrace(bool print_reacs);
  virtual void SetupMiniTraceFileHeader(const cString& filename, const int gen_id, const cString& genotype) = 0;
  void SetupExtendedMemory(const tArray<int>& ext_mem) { m_ext_mem = ext_mem; }
  void PrintMiniTraceReactions();
  
  // --------  Stack Manipulation...  --------
  virtual int GetStack(int depth = 0, int stack_id = -1, int in_thread = -1) const = 0;
  virtual int GetNumStacks() const = 0;
  
  
  // --------  Head Manipulation (including IP)  --------
  virtual const cHeadCPU& GetHead(int head_id) const = 0;
  virtual cHeadCPU& GetHead(int head_id) = 0;
  virtual const cHeadCPU& GetHead(int head_id, int thread) const = 0;
  virtual cHeadCPU& GetHead(int head_id, int thread) = 0;
  virtual int GetNumHeads() const = 0;
  
  virtual const cHeadCPU& IP() const = 0;
  virtual cHeadCPU& IP() = 0;
  virtual const cHeadCPU& IP(int thread) const = 0;
  virtual cHeadCPU& IP(int thread) = 0;
  
  cHeadCPU FindLabelFull(const cCodeLabel& label);
  
  
  // --------  Memory Manipulation  --------
  virtual const cCPUMemory& GetMemory() const = 0;
  virtual cCPUMemory& GetMemory() = 0;
  virtual int GetMemSize() const = 0;
  virtual const cCPUMemory& GetMemory(int value) const = 0;
  virtual cCPUMemory& GetMemory(int value) = 0;
  virtual int GetMemSize(int value) const = 0;
  virtual int GetNumMemSpaces() const = 0;
  
  const tSmartArray<int>& GetExtendedMemory() const { return m_ext_mem; }
  
  
  // --------  Register Manipulation  --------
  virtual int GetRegister(int reg_id) const = 0;
  virtual int GetNumRegisters() const = 0;
  
  
  // --------  Thread Manipulation  --------
  virtual bool ThreadSelect(const int thread_id) = 0;
  virtual bool ThreadSelect(const cCodeLabel& in_label) = 0;
  virtual void ThreadNext() = 0;
  virtual void ThreadPrev() = 0;
  virtual cBioUnit* ThreadGetOwner() = 0;

  virtual int GetNumThreads() const = 0;
  virtual int GetCurThread() const = 0;
  virtual int GetCurThreadID() const = 0;
  
  // interrupt current thread
  virtual bool InterruptThread(int interruptType) = 0; // only implemented in cHardwareCPU and cHardwareExperimental
  virtual int GetThreadMessageTriggerType(int _index) = 0;
  
  
  // --------  Parasite Stuff  --------
  virtual bool ParasiteInfectHost(cBioUnit* bu) = 0;
  
    
  // --------  Mutation  --------
  virtual int PointMutate(cAvidaContext& ctx, double override_mut_rate = 0.0);

  
  // --------  Input/Output Buffers  --------
  virtual tBuffer<int>& GetInputBuf();
  virtual tBuffer<int>& GetOutputBuf();
  
  
  // --------  State Transfer  --------
  virtual void InheritState(cHardwareBase& in_hardware) { ; }
  
  
  // --------  Alarm  --------
  virtual bool Jump_To_Alarm_Label(int jump_label) { return false; }
  

	// -------- Synchronization --------
  //! Called when the organism that owns this CPU has received a flash from a neighbor.
  virtual void ReceiveFlash();	
	
	// -------- HGT --------
	//! Retrieve a genome fragment extending downstream from the read head.
	virtual Sequence GetGenomeFragment(unsigned int downstream);
	//! Insert a genome fragment at the current write head.
	virtual void InsertGenomeFragment(const Sequence& fragment);
  
protected:
  // --------  Core Execution Methods  --------
  bool SingleProcess_PayPreCosts(cAvidaContext& ctx, const cInstruction& cur_inst, const int thread_id);
  bool IsPayingActiveCost(cAvidaContext& ctx, const int thread_id);
  void SingleProcess_PayPostResCosts(cAvidaContext& ctx, const cInstruction& cur_inst);
  void SingleProcess_SetPostCPUCosts(cAvidaContext& ctx, const cInstruction& cur_inst, const int thread_id);
  virtual void internalReset() = 0;
	virtual void internalResetOnFailedDivide() = 0;
  
  
  // --------  No-Operation Instruction  --------
  bool Inst_Nop(cAvidaContext& ctx);  // A no-operation instruction that does nothing! 
  
  
  // --------  Implicit Repro Check/Instruction  -------- @JEB
  inline void CheckImplicitRepro(cAvidaContext& ctx, bool exec_last_inst = false)
    { if (m_implicit_repro_active) checkImplicitRepro(ctx, exec_last_inst); }
  virtual bool Inst_Repro(cAvidaContext& ctx);

  
  // --------  Execution Speed Instruction  --------
  bool Inst_DoubleEnergyUsage(cAvidaContext& ctx);
  bool Inst_HalveEnergyUsage(cAvidaContext& ctx);
  bool Inst_DefaultEnergyUsage(cAvidaContext& ctx);
	

  
  // --------  Mutation Helper Methods  --------
  bool doUniformMutation(cAvidaContext& ctx, Sequence& genome);
  void doUniformCopyMutation(cAvidaContext& ctx, cHeadCPU& head);
  void doSlipMutation(cAvidaContext& ctx, Sequence& genome, int from = -1);
  void doTransMutation(cAvidaContext& ctx, Sequence& genome, int from = -1);
  void doLGTMutation(cAvidaContext& ctx, Sequence& genome);
  

  // --------  Organism Execution Property Calculation  --------
  virtual int calcExecutedSize(const int parent_size);
  virtual int calcCopiedSize(const int parent_size, const int child_size) = 0;  
  
  
  // --------  Division Support Methods  --------
  bool Divide_CheckViable(cAvidaContext& ctx, const int parent_size, const int child_size, bool using_repro = false);
  unsigned Divide_DoExactMutations(cAvidaContext& ctx, double mut_multiplier = 1.0, const int pointmut = INT_MAX);
  bool Divide_TestFitnessMeasures1(cAvidaContext& ctx);
  

private:
  void checkImplicitRepro(cAvidaContext& ctx, bool exec_last_inst = false);
};


#endif
