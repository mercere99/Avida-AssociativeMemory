/*
 *  Map.cc
 *  Avida
 *
 *  Created by Charles on 7-9-07
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  http://avida.devosoft.org/
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
 *  Authors: David M. Bryson <david@programerror.com>, Charles Ofria <ofria@msu.edu>
 *
 */

#include "avida/viewer-core/Map.h"

#include "cBioGroup.h"
#include "cEnvironment.h"
#include "cOrganism.h"
#include "cPopulation.h"
#include "cPopulationCell.h"
#include "cStringUtil.h"
#include "cWorld.h"

#include "avida/private/viewer-core/ClassificationInfo.h"

#include <cmath>
#include <iostream>


Avida::CoreView::MapMode::~MapMode() { ; }
Avida::CoreView::DiscreteScale::~DiscreteScale() { ; }


class cFitnessMapMode : public Avida::CoreView::MapMode, public Avida::CoreView::DiscreteScale
{
private:
  static const int SCALE_MAX = 201;
  static const int SCALE_LABELS = 4;
  static const int RESCALE_TIME_CONSTANT = 40;
  static const double RESCALE_TOLERANCE;
  static const double MAX_RESCALE_FACTOR;
private:
  Apto::Array<int> m_color_grid;
  Apto::Array<int> m_color_count;
  Apto::Array<DiscreteScale::Entry> m_scale_labels;
  
  double m_cur_min;
  double m_cur_max;
  double m_target_min;
  double m_target_max;
  double m_rescale_rate_min;
  double m_rescale_rate_max;
  
public:
  cFitnessMapMode(cWorld* world)
    : m_color_count(SCALE_MAX + Avida::CoreView::MAP_RESERVED_COLORS), m_scale_labels(SCALE_LABELS)
    , m_cur_min(0.0), m_cur_max(0.0), m_target_min(0.0), m_target_max(0.0), m_rescale_rate_min(0.0), m_rescale_rate_max(0.0)
  { ; }
  ~cFitnessMapMode() { ; }
  
  // MapMode Interface
  const Apto::String& GetName() const { static const Apto::String name("Fitness"); return name; }
  const Apto::Array<int>& GetGridValues() const { return m_color_grid; }
  const Apto::Array<int>& GetValueCounts() const { return m_color_count; }
  
  const DiscreteScale& GetScale() const { return *this; }
  const Apto::String& GetScaleLabel() const;
  
  int GetSupportedTypes() const { return Avida::CoreView::MAP_GRID_VIEW_COLOR; }


  void Update(cPopulation& pop);
  
  
  // DiscreteScale Interface
  int GetScaleRange() const { return m_color_count.GetSize() - Avida::CoreView::MAP_RESERVED_COLORS; }
  int GetNumLabeledEntries() const { return m_scale_labels.GetSize(); }
  DiscreteScale::Entry GetEntry(int index) const { return m_scale_labels[index]; }  
};

const double cFitnessMapMode::RESCALE_TOLERANCE = 0.1;
const double cFitnessMapMode::MAX_RESCALE_FACTOR = 0.03;

void cFitnessMapMode::Update(cPopulation& pop)
{
  m_color_grid.Resize(pop.GetSize());
  
  // Keep track of how many times each color was assigned.
  m_color_count.SetAll(0);
  
  // Determine the max and min in the population.
  double max_fit = 0.0;
  double min_fit = 0.0;
  
  for (int i = 0; i < pop.GetSize(); i++) {
    cOrganism* org = pop.GetCell(i).GetOrganism();
    if (org == NULL) continue;
    double fit = org->GetPhenotype().GetFitness();
    if (fit == 0.0) continue;
//    fit = log2(fit);
    if (fit > max_fit) max_fit = fit;
    if (fit < min_fit) min_fit = fit;
  }

  if (m_cur_max == 0.0) {
    // Reset range
    m_cur_max = max_fit;
    m_target_max = max_fit;
    m_rescale_rate_min = 0.0;
    m_rescale_rate_max = 0.0;
    
    // Update scale labels
    for (int i = 0; i < m_scale_labels.GetSize(); i++) {
      m_scale_labels[i].index = (SCALE_MAX / (m_scale_labels.GetSize() - 1)) * i;
      m_scale_labels[i].label =
        static_cast<const char*>(cStringUtil::Stringf("%f", ((m_cur_max - m_cur_min) / (m_scale_labels.GetSize() - 1)) * i));
    }
  } else {
    if (max_fit < (1.0 - RESCALE_TOLERANCE) * m_target_max || m_target_max < max_fit) {
      m_target_max = max_fit * (1.0 + RESCALE_TOLERANCE);
      m_rescale_rate_max = (m_target_max - m_cur_max) / RESCALE_TIME_CONSTANT;
    }
    
    if (m_rescale_rate_max != 0.0) {
      if (min_fit <= m_cur_max) {
        m_cur_max += m_rescale_rate_max;
      } else {
        double max_rate = m_cur_max * MAX_RESCALE_FACTOR;
        m_cur_max += (m_rescale_rate_max < max_rate) ? m_rescale_rate_max : max_rate;
      }
      
      if (fabs(m_target_max - m_cur_max) <= fabs(m_rescale_rate_max)) {
        m_cur_max = m_target_max;
        m_rescale_rate_max = 0.0;
      }
      
      // Update scale labels
      for (int i = 0; i < m_scale_labels.GetSize(); i++) {
        m_scale_labels[i].index = (SCALE_MAX / (m_scale_labels.GetSize() - 1)) * i;
        m_scale_labels[i].label =
          static_cast<const char*>(cStringUtil::Stringf("%2.2f", ((m_cur_max - m_cur_min) / (m_scale_labels.GetSize() - 1)) * i));
      }
    }
  }
  
  // Now fill out the color grid.
  for (int i = 0; i < pop.GetSize(); i++) {
    cOrganism* org = pop.GetCell(i).GetOrganism();
    if (org == NULL) {
      m_color_grid[i] = Avida::CoreView::MAP_RESERVED_COLOR_BLACK;
      m_color_count[Avida::CoreView::MAP_RESERVED_COLORS - Avida::CoreView::MAP_RESERVED_COLOR_BLACK]++;
      continue;
    }

    double fit = org->GetPhenotype().GetFitness();
    if (fit == 0.0) {
      m_color_grid[i] = Avida::CoreView::MAP_RESERVED_COLOR_DARK_GRAY;
      m_color_count[Avida::CoreView::MAP_RESERVED_COLORS - Avida::CoreView::MAP_RESERVED_COLOR_DARK_GRAY]++;
      continue;
    }
    
//    fit = log2(fit);
    
    fit = (fit - m_cur_min) / (m_cur_max - m_cur_min);
    if (fit > 1.0) {
      m_color_grid[i] = Avida::CoreView::MAP_RESERVED_COLOR_WHITE;
      m_color_count[Avida::CoreView::MAP_RESERVED_COLORS - Avida::CoreView::MAP_RESERVED_COLOR_WHITE]++;
    } else {
      int color = fit * static_cast<double>(SCALE_MAX - 1);
      m_color_grid[i] = color;
      m_color_count[color + Avida::CoreView::MAP_RESERVED_COLORS]++;
    }
  }
}

const Apto::String& cFitnessMapMode::GetScaleLabel() const
{
  static const Apto::String normal("Fitness");
  static const Apto::String rescale("Fitness (rescaling)");
  
  if (m_rescale_rate_max != 0) return rescale;
  
  return normal;
}



class cGenotypeMapMode : public Avida::CoreView::MapMode, public Avida::CoreView::DiscreteScale
{
private:
  static const int NUM_COLORS = 10;
private:
  Avida::CoreView::ClassificationInfo* m_info;
  Apto::Array<int> m_color_grid;
  Apto::Array<int> m_color_count;
  Apto::Array<DiscreteScale::Entry> m_scale_labels;
  
public:
  cGenotypeMapMode(cWorld* world)
    : m_info(new Avida::CoreView::ClassificationInfo(world, "genotype", NUM_COLORS))
    , m_color_count(NUM_COLORS + Avida::CoreView::MAP_RESERVED_COLORS)
  { ; }
  virtual ~cGenotypeMapMode() { delete m_info; }
  
  // MapMode Interface
  const Apto::String& GetName() const { static const Apto::String name("Genotypes"); return name; }
  const Apto::Array<int>& GetGridValues() const { return m_color_grid; }
  const Apto::Array<int>& GetValueCounts() const { return m_color_count; }
  
  const DiscreteScale& GetScale() const { return *this; }
  const Apto::String& GetScaleLabel() const { static const Apto::String name("Genotypes"); return name; }
  
  int GetSupportedTypes() const { return Avida::CoreView::MAP_GRID_VIEW_COLOR; }
  
  void Update(cPopulation& pop);
  
  
  // DiscreteScale Interface
  int GetScaleRange() const { return m_color_count.GetSize() - Avida::CoreView::MAP_RESERVED_COLORS; }
  int GetNumLabeledEntries() const { return m_scale_labels.GetSize(); }
  DiscreteScale::Entry GetEntry(int index) const { return m_scale_labels[index]; }  
};


void cGenotypeMapMode::Update(cPopulation& pop)
{
  m_info->Update();
  m_color_grid.Resize(pop.GetSize());
  m_color_count.SetAll(0);            // reset all color counts
  for (int i = 0; i < pop.GetSize(); i++) {
    cOrganism* org = pop.GetCell(i).GetOrganism();
    if (org == NULL) {
      m_color_grid[i] = -4;
      m_color_count[0]++;
    } else {
      Avida::CoreView::ClassificationInfo::MapColor* mapcolor =
      org->GetBioGroup("genotype")->GetData<Avida::CoreView::ClassificationInfo::MapColor>();
      if (mapcolor) {
        m_color_grid[i] = mapcolor->color;
        m_color_count[mapcolor->color + 4];
      } else {
        m_color_grid[i] = -1;
        m_color_count[3]++;
      }
    }
  }
}





Avida::CoreView::Map::Map(cWorld* world)
  : m_width(world->GetPopulation().GetWorldX())
  , m_height(world->GetPopulation().GetWorldY())
  , m_num_viewer_colors(-1)
  , m_color_mode(0)
  , m_symbol_mode(-1)
  , m_tag_mode(-1)
{
  // Setup the available view modes...
  m_view_modes.Resize(2);
  m_view_modes[0] = new cFitnessMapMode(world);
  m_view_modes[1] = new cGenotypeMapMode(world);

  
//  AddViewMode("Genome Length",  &cCoreView_Map::SetColors_Length,   VIEW_COLOR, COLORS_SCALE);

//  AddViewMode("None",           &cCoreView_Map::TagCells_None,      VIEW_TAGS);
//
//  AddViewMode("Square",         &cCoreView_Map::SetSymbol_Square,   VIEW_SYMBOLS);
//  AddViewMode("Facing",         &cCoreView_Map::SetSymbol_Facing,   VIEW_SYMBOLS);

  // Load tasks...
//  const cEnvironment& env = world->GetEnvironment();
//  const int num_tasks = env.GetNumTasks();
//  for (int i = 0; i < num_tasks; i++) {
//    cString mode_name = env.GetTask(i).GetDesc();
//    mode_name.Insert("Task/");
//    AddViewMode(mode_name, &cCoreView_Map::TagCells_Task, VIEW_TAGS, i);
//  }
}

Avida::CoreView::Map::~Map()
{
  for (int i = 0; i < m_view_modes.GetSize(); i++) delete m_view_modes[i];
}



void Avida::CoreView::Map::UpdateMaps(cPopulation& pop)
{
  m_rw_lock.WriteLock();
  
  for (int i = 0; i < m_view_modes.GetSize(); i++) m_view_modes[i]->Update(pop);
  
  m_rw_lock.WriteUnlock();
}


void Avida::CoreView::Map::SetMode(int mode)
{
  int type = m_view_modes[mode]->GetSupportedTypes();
  if (type == MAP_GRID_VIEW_COLOR) m_color_mode = mode;
  else if (type == MAP_GRID_VIEW_SYMBOLS) m_symbol_mode = mode;
  else if (type == MAP_GRID_VIEW_TAGS) m_tag_mode = mode;
  else assert(false);
}


//
//void cCoreView_Map::TagCells_None(cPopulation& pop, int ignore)
//{
//  m_tag_grid.Resize(pop.GetSize());
//  m_tag_grid.SetAll(0);
//}
//
//
//void cCoreView_Map::TagCells_Task(cPopulation& pop, int task_id)
//{
//  m_tag_grid.Resize(pop.GetSize());
//  for (int i = 0; i < pop.GetSize(); i++) {
//    cOrganism * org = pop.GetCell(i).GetOrganism();
//    if (org == NULL) m_tag_grid[i] = 0;
//    else if (org->GetPhenotype().GetCurTaskCount()[task_id] > 0) m_tag_grid[i] = 1;
//    else if (org->GetPhenotype().GetLastTaskCount()[task_id] > 0) m_tag_grid[i] = 2;
//    else m_tag_grid[i] = 0;
//  }
//}
