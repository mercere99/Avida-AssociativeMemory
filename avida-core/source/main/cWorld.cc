/*
 *  cWorld.cc
 *  Avida
 *
 *  Created by David on 10/18/05.
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

#include "cWorld.h"

#include "avida/Avida.h"
#include "AvidaTools.h"

#include "avida/data/Manager.h"

#include "cAnalyze.h"
#include "cAnalyzeGenotype.h"
#include "cBioGroupManager.h"
#include "cClassificationManager.h"
#include "cEnvironment.h"
#include "cEventList.h"
#include "cFallbackWorldDriver.h"
#include "cHardwareManager.h"
#include "cMigrationMatrix.h"   // MIGRATION_MATRIX
#include "cInstSet.h"
#include "cPopulation.h"
#include "cStats.h"
#include "cTestCPU.h"
#include "cUserFeedback.h"

#include <cassert>

using namespace AvidaTools;


cWorld::cWorld(cAvidaConfig* cfg, const cString& wd)
  : m_working_dir(wd), m_analyze(NULL), m_conf(cfg), m_ctx(this, m_rng), m_class_mgr(NULL), m_datafile_mgr(NULL)
  , m_env(NULL), m_event_list(NULL), m_hw_mgr(NULL),m_mig_mat(NULL), m_pop(NULL), m_stats(NULL), m_driver(NULL), m_data_mgr(NULL)   // MIGRATION_MATRIX
{
}

cWorld* cWorld::Initialize(cAvidaConfig* cfg, const cString& working_dir, cUserFeedback* feedback)
{
  cWorld* world = new cWorld(cfg, working_dir);
  if (!world->setup(feedback)) { 
    delete world;
    world = NULL;
  }
  return world;
}

cWorld::~cWorld()
{
  // m_actlib is not owned by cWorld, DO NOT DELETE
  
  // These must be deleted first
  delete m_analyze; m_analyze = NULL;
  delete m_pop; m_pop = NULL;
  
  delete m_class_mgr; m_class_mgr = NULL;
  delete m_env; m_env = NULL;
  delete m_event_list; m_event_list = NULL;
  delete m_hw_mgr; m_hw_mgr = NULL;

  // Delete after all classes that may be logging items
  if (m_datafile_mgr) { m_datafile_mgr->FlushAll(); }
  delete m_datafile_mgr; m_datafile_mgr = NULL;
  
  delete m_data_mgr;

  delete m_mig_mat; // MIGRATION_MATRIX
  
  // Delete Last
  delete m_conf; m_conf = NULL;

  // cleanup driver object, if needed
  if (m_own_driver) { delete m_driver; m_driver = NULL; }
}


bool cWorld::setup(cUserFeedback* feedback)
{
  bool success = true;
  
  m_own_driver = true;
  m_driver = new cFallbackWorldDriver();
  
  // Setup Random Number Generator
  m_rng.ResetSeed(m_conf->RANDOM_SEED.Get());
  m_srng.ResetSeed(m_conf->RANDOM_SEED.Get());
  
  m_datafile_mgr = new cDataFileManager(cString(Apto::FileSystem::GetAbsolutePath(Apto::String(m_conf->DATA_DIR.Get()), Apto::String(m_working_dir))), (m_conf->VERBOSITY.Get() > VERBOSE_ON));
  
  m_data_mgr = new Avida::Data::Manager(this);
  
  m_class_mgr = new cClassificationManager(this);
  m_env = new cEnvironment(this);
    
  m_mig_mat = new cMigrationMatrix(); // MIGRATION_MATRIX
  
  
  // Initialize the default environment...
  // This must be after the HardwareManager in case REACTIONS that trigger instructions are used.
  if (!m_env->Load(m_conf->ENVIRONMENT_FILE.Get(), m_working_dir, *feedback)) {
    success = false;
  }
    
  if(m_conf->DEMES_MIGRATION_METHOD.Get() == 4){
    // MIGRATION_MATRIX   
    
    bool count_parasites,count_offspring = false;
    if(m_conf->DEMES_PARASITE_MIGRATION_RATE.Get() > 0.0)
      count_parasites = true;
    if(m_conf->DEMES_MIGRATION_RATE.Get() > 0.0)
      count_offspring = true;
    
    if(!m_mig_mat->Load(m_conf->NUM_DEMES.AsString().AsInt(), m_conf->MIGRATION_FILE.Get(), m_working_dir,count_parasites,count_offspring,false,*feedback))
      success = false;
  }
  
  
  // Setup Stats Object
  m_stats = Apto::SmartPtr<cStats, Apto::ThreadSafeRefCount>(new cStats(this));
  m_class_mgr->GetBioGroupManager("genotype")->AddListener(Apto::SmartPtr<cStats, Apto::ThreadSafeRefCount>::GetPointer(m_stats));

  
  // Initialize the hardware manager, loading all of the instruction sets
  m_hw_mgr = new cHardwareManager(this);
  if (m_conf->INST_SET_LOAD_LEGACY.Get()) {
    if (!m_hw_mgr->ConvertLegacyInstSetFile(m_conf->INST_SET.Get(), m_conf->INSTSETS.Get(), feedback)) success = false;
  }
  if (!m_hw_mgr->LoadInstSets(feedback)) success = false;
  if (m_hw_mgr->GetNumInstSets() == 0) {
    if (feedback) {
      feedback->Error("no instruction sets defined");
      if (!m_conf->INST_SET_LOAD_LEGACY.Get() && m_conf->INST_SET.Get() != "" && m_conf->INST_SET.Get() != "-") {
        feedback->Notify("It looks like you are attempting to load a legacy format instruction set file.  Try setting INST_SET_LOAD_LEGACY to 1.");
      }
    }
    success = false;
  }
  
  // If there were errors loading at this point, it is perilous to try to go further (pop depends on an instruction set)
  if (!success) return success;
  
  
  // @MRR CClade Tracking
//	if (m_conf->TRACK_CCLADES.Get() > 0)
//		m_class_mgr->LoadCCladeFounders(m_conf->TRACK_CCLADES_IDS.Get());
  
  const bool revert_fatal = m_conf->REVERT_FATAL.Get() > 0.0;
  const bool revert_neg = m_conf->REVERT_DETRIMENTAL.Get() > 0.0;
  const bool revert_neut = m_conf->REVERT_NEUTRAL.Get() > 0.0;
  const bool revert_pos = m_conf->REVERT_BENEFICIAL.Get() > 0.0;
  const bool revert_taskloss = m_conf->REVERT_TASKLOSS.Get() > 0.0;
  const bool revert_equals = m_conf->REVERT_EQUALS.Get() > 0.0;
  const bool sterilize_unstable = m_conf->STERILIZE_UNSTABLE.Get() > 0;
  m_test_on_div = (revert_fatal || revert_neg || revert_neut || revert_pos || revert_taskloss || revert_equals || sterilize_unstable);
  
  const bool sterilize_fatal = m_conf->STERILIZE_FATAL.Get() > 0.0;
  const bool sterilize_neg = m_conf->STERILIZE_DETRIMENTAL.Get() > 0.0;
  const bool sterilize_neut = m_conf->STERILIZE_NEUTRAL.Get() > 0.0;
  const bool sterilize_pos = m_conf->STERILIZE_BENEFICIAL.Get() > 0.0;
  const bool sterilize_taskloss = m_conf->STERILIZE_TASKLOSS.Get() > 0.0;
  m_test_sterilize = (sterilize_fatal || sterilize_neg || sterilize_neut || sterilize_pos || sterilize_taskloss);

  m_pop = new cPopulation(this);
  if (!m_pop->InitiatePop(feedback)) success = false;
  
  // Setup Event List
  m_event_list = new cEventList(this);
  if (!m_event_list->LoadEventFile(m_conf->EVENT_FILE.Get(), m_working_dir, *feedback)) {
    if (feedback) feedback->Error("unable to load event file");
    success = false;
  }
  
  return success;
}

Apto::SmartPtr<Data::Provider, Apto::ThreadSafeRefCount> cWorld::GetStatsProvider(cWorld*) { return m_stats; }


cAnalyze& cWorld::GetAnalyze()
{
  if (m_analyze == NULL) m_analyze = new cAnalyze(this);
  return *m_analyze;
}

void cWorld::GetEvents(cAvidaContext& ctx)
{  
  if (m_pop->GetSyncEvents() == true) {
    m_event_list->Sync();
    m_pop->SetSyncEvents(false);
  }
  m_event_list->Process(ctx);
}

int cWorld::GetNumResources()
{
  return m_env->GetResourceLib().GetSize();
}


void cWorld::SetDriver(WorldDriver* driver, bool take_ownership)
{
  // cleanup current driver, if needed
  if (m_own_driver) delete m_driver;
  
  // store new driver information
  m_driver = driver;
  m_own_driver = take_ownership;
}

/*! Calculate the size (in virtual CPU cycles) of the current update.
 */
int cWorld::CalculateUpdateSize()
{
	return GetConfig().AVE_TIME_SLICE.Get() * GetPopulation().GetNumOrganisms();
}
