// Authors: Sascha Lange
// Copyright (c) 2004, Neuroinformatics Group, University of Osnabrueck
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, 
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the University of Osnabrueck nor the names of its 
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.

#include "registry.h"
#include "global.h"
#include "valueparser.h"
#include "string.h"
#include "stdlib.h"
#include <iostream>

#define ERROUT_REG( __x__ ) std::cerr << "#ERROR: " << __FILE__ << " in Line: " << __LINE__ << " in " << __PRETTY_FUNCTION__ << __x__ << "\n"

using namespace Tapir;

AbstractRegistry::AbstractRegistry()
{entries.clear();}

AbstractRegistry::~AbstractRegistry()
{
  for (unsigned int i=0; i < entries.size(); i++) {
    delete entries[i].factory;
  }
  entries.clear();
}

void AbstractRegistry::add(const char* name, const char* desc, Factory *factory) {
  Entry ne;
  ne.name = name;
  ne.desc = desc;
  ne.factory = factory;
  entries.push_back(ne);
}

AbstractRegistry::Factory* AbstractRegistry::lookup(const char* name) const
{
  for (unsigned int i=0; i < entries.size(); i++) {
    if (std::string(name) == entries[i].name) {
      return entries[i].factory;
    }
  }
  return 0;  // nicht gefunden
}

int AbstractRegistry::listEntries(char* buf, int buf_size) const 
{
  buf[0] = '\0';
  int available = buf_size - 1;
  for (unsigned int i=0; i < entries.size(); i++) {
    strncat(buf, entries[i].name.c_str(), available);
    available -= strlen(entries[i].name.c_str());
    strncat(buf, "\n\t", available);
    available -= 2;
    strncat(buf, entries[i].desc.c_str(), available);
    available -= strlen(entries[i].desc.c_str());
    strncat(buf, "\n", available);
    available -= 1;
    if (available < 0) {
      return 1;
    }
  }
  buf[buf_size-1] = '\0'; // safety first
  return 0;
}
      
void AbstractRegistry::listEntries(std::vector<std::string>* names , std::vector<std::string>* desc) const
{
    names->clear();
    if (desc!=0) desc->clear();
    std::cout << "Available modules: " << entries.size() << std::endl;
    for (unsigned int i=0; i < entries.size(); i++) {
        names->push_back( entries[i].name );
        std::cout << "  " << entries[i].name << " (" << entries[i].desc << ")" << std::endl;
        if (desc!=0) desc->push_back( entries[i].desc );
    }
}

// die Registry wird NICHT statisch erzeugt. Bei statischer Erzeugung wäre
// nicht sicher, dass sie vor den ebenfall statisch erzeugten Factoryklassen
// angelegt worden wäre (the_imagesourceFactory könnte beim Aufruf des Konstruktors 
// der Factory also noch ins Nirvana zeigen). Daher wird die Registry erst bei
// Bedarf von get erzeugt.
#define ADD_MODULE_IMPLEMENTATION(_type_) \
_type_* _type_##Factory::create(const char* name) const \
{ \
  Factory* factory = lookup(name); \
  if (! factory) { \
    return 0; \
  } \
  else { \
    return (_type_*) factory->create(); \
  } \
} \
_type_##Factory* _type_##Factory::get() \
{ \
  if (! the##_type_##Factory) { \
    the##_type_##Factory = new _type_##Factory(); \
  } \
  return the##_type_##Factory; \
} \
_type_##Factory* _type_##Factory::the##_type_##Factory = 0;

FOR_EACH_MODULE(ADD_MODULE_IMPLEMENTATION);

static KeyRegistry *_instance = 0;
KeyRegistry* KeyRegistry::instance ()
{
  if (_instance == 0) _instance = new KeyRegistry();
  return _instance;
}

bool KeyRegistry::signup (const long int cmd, TapirModule* module, const char* entry, const char* name, const char* desc, bool print)
{
  // determine substitute
  ValueParser vp(fname, "Keymap");

  std::stringstream ss;
  ss << name << "_" << entry;

  ///< \todo not intuitive, can't use characters
  long int sub;
  vp.get(ss.str().c_str(), sub, cmd);
  if (sub != cmd)
    IOUT("Key " << entry << " will be mapped to '" << (char)sub << "' (value " << sub << ") for module " << name << ".");

  // check conflicts
  if (_peres[sub] != 0) {
    WOUT("Key " << (char)sub << " (" << sub << ") has already been claimed by another module. Functionality will not be available for module " << name << ".");
    return false;
  }

  // assign maps
  _trans[sub] = cmd;
  _peres[sub] = module;
  if (print) list << entry << "   " << desc << " (" << name << " #" << sub << ")" << std::endl;
  return true;
}

bool KeyRegistry::delegate (const long int cmd, const char* params)
{
  TapirModule *mod = _peres[cmd];
  if (mod == 0) return false;
  mod->notify(_trans[cmd], params); ///< \todo can't transform everything
  return true;
}

