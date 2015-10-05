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
#include "string.h"
#include "stdlib.h"
#include "global.h"

#define ERROUT_REG( __x__ ) std::cerr << "#ERROR: " << __FILE__ << " in Line: " << __LINE__ << " in " << __PRETTY_FUNCTION__ << __x__ << "\n"

using namespace CLS;

AbstractRegistry::AbstractRegistry()
{
  entries.clear();
}

AbstractRegistry::~AbstractRegistry()
{
  for (unsigned int i=0; i < entries.size(); i++)
    delete entries[i].factory;
    // no need to delete names and descriptions, since they were created by macros
  entries.clear();
}

bool AbstractRegistry::add(const char* name, const char* desc, Factory *factory)
{
  if ( lookup(name) != 0 ) {
    EOUT("An entity of name: " << name << " already exists in the registry. Will not add a second one!" );
    delete factory;
    return false;
  }
  Entry ne;
  ne.name = name;
  ne.desc = desc;
  ne.factory = factory;
  entries.push_back(ne);
  return true;
}

AbstractRegistry::Factory* AbstractRegistry::lookup(const char* name) const
{
  for (unsigned int i=0; i < entries.size(); i++) {
    if (strcmp(name,entries[i].name) == 0) {
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
    strncat(buf, entries[i].name, available);
    available -= strlen(entries[i].name);
    strncat(buf, "\n\t", available);
    available -= 2;
    strncat(buf, entries[i].desc, available);
    available -= strlen(entries[i].desc);
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
  for (unsigned int i=0; i < entries.size(); i++) {
    names->push_back( std::string(entries[i].name) );
    if (desc!=0) desc->push_back( std::string(entries[i].desc) );
  }
}

#define CLS_FACTORY_IMPLEMENT(_type_)                       \
_type_* _type_##Factory::create(const char* name) const     \
{                                                           \
  Factory* factory = lookup(name);                          \
  if (!factory) {                                           \
    return 0;                                               \
  } else {                                                  \
    return (_type_*) factory->create();                     \
  }                                                         \
}                                                           \
                                                            \
_type_##Factory* _type_##Factory::getThe##_type_##Factory() \
{                                                           \
  if (!the##_type_##Factory) {                              \
    the##_type_##Factory = new _type_##Factory();           \
  }                                                         \
  return the##_type_##Factory;                              \
}                                                           \
                                                            \
void _type_##Factory::deleteThe##_type_##Factory()          \
{                                                           \
  if (the##_type_##Factory) {                               \
    delete the##_type_##Factory;                            \
    the##_type_##Factory = NULL;                            \
  }                                                         \
}
FOR_EACH_CLS_MODULE_TYPE(CLS_FACTORY_IMPLEMENT);

// Die Registry wird NICHT statisch erzeugt. Bei statischer Erzeugung wäre
// nicht sicher, dass sie vor den ebenfall statisch erzeugten Factoryklassen
// angelegt worden wäre (the_plantFactory könnte beim Aufruf des Konstruktors
// der Factory also noch ins Nirvana zeigen). Daher wird die Registry erst bei
// Bedarf von getThePlantFactory erzeugt.
#define CLS_FACTORY_CREATE_POINTER(_type_) \
_type_##Factory* _type_##Factory::the##_type_##Factory = 0;
FOR_EACH_CLS_MODULE_TYPE(CLS_FACTORY_CREATE_POINTER);

