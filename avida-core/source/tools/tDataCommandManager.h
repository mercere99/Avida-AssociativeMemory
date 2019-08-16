/*
 *  tDataCommandManager.h
 *  Avida
 *
 *  Created by David on 10/31/08.
 *  Copyright 2008-2011 Michigan State University. All rights reserved.
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

#ifndef tDataCommandManager_h
#define tDataCommandManager_h

#include "cStringUtil.h"
#include "cUserFeedback.h"
#include "tDataEntry.h"
#include "tDataEntryCommand.h"
#include "tDictionary.h"
#include "tList.h"


template <class TargetType> class tDataCommandManager
{
private:
  tDictionary<tDataEntry<TargetType>*> m_entry_dict;  
  tArray<cString> m_entry_names;
  
public:
  tDataCommandManager() { ; }
  ~tDataCommandManager()
  {
    tArray<tDataEntry<TargetType>*> entries;
    m_entry_dict.GetValues(entries);
    for (int i = 0; i < entries.GetSize(); i++) delete entries[i];
  }
  
  void Add(const cString& name, tDataEntry<TargetType>* entry)
  {
    m_entry_dict.Set(name, entry);
    m_entry_names.Push(name);
  }
  
  const tArray<cString>& GetEntryNames() const { return m_entry_names; }

  tDataEntryCommand<TargetType>* GetDataCommand(const cString& cmd, cString* error_str = NULL) const
  {
    cString arg_list = cmd;
    cString idx = arg_list.Pop(':');
    cString entry_name = idx.Pop('.');
    
    tDataEntry<TargetType>* data_entry;
    if (m_entry_dict.Find(entry_name, data_entry)) {
      return new tDataEntryCommand<TargetType>(data_entry, idx, arg_list);
    }
    
    if (error_str) {
      cString nm = m_entry_dict.NearMatch(entry_name);
      if (nm.GetSize()) {
        (*error_str) = cStringUtil::Stringf("data entry '%s' not found, best match is '%s'", (const char*)entry_name, (const char*)nm);
      } else {
        (*error_str) = cStringUtil::Stringf("date entry '%s' not found", (const char*)entry_name);
      }
    }
    
    return NULL;
  }
  
  void LoadCommandList(cStringList arg_list, tList<tDataEntryCommand<TargetType> >& output_list,
                       cUserFeedback* feedback = NULL) const
  {
    if (arg_list.GetSize() == 0) {
      // If no args were given, load all of the stats.
      // @TODO - handle indexed items...  under this scheme only the first task and env_input value will be output    
      
      tArray<tDataEntry<TargetType>*> data_entries;
      m_entry_dict.GetValues(data_entries);
      for (int i = 0; i < data_entries.GetSize(); i++)
        output_list.PushRear(new tDataEntryCommand<TargetType>(data_entries[i]));
    } else {
      while (arg_list.GetSize() != 0) {
        cString error_str;
        tDataEntryCommand<TargetType>* cur_command = GetDataCommand(arg_list.Pop(), &error_str);
        if (cur_command) output_list.PushRear(cur_command);
        if (feedback && error_str != "") feedback->Error(error_str);
      }
    }
  }
};


#endif
