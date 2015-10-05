#ifndef _registry_h_
#define _registry_h_

// Authors: Sascha Lange
// Modified: Thomas Lampe
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

#include <vector>
#include <string>
#include <sstream>
#include "modules.h"
#include <map>

namespace Tapir
{

/** Central registry of factory classes that are able to construct instances
    of an unknown class. */
class AbstractRegistry
{
public:
  /** this interface declares an abstract factory, that is able to construct
      instances of one class. Each class needs a different implementation of 
      the abstract factory. */
  class Factory {  
  public:
    virtual ~Factory() {}
    virtual void* create() const=0;
  };    

  virtual ~AbstractRegistry();
  
  /** add a factory for the class "name" */
  virtual void add(const char* name, const char* desc, Factory* factory);

  /** lookup a factory for the requested class "name" */
  virtual Factory* lookup(const char* name) const;

  virtual int listEntries(char* buf, int buf_size) const;

  virtual void listEntries(std::vector<std::string>* names , std::vector<std::string>* desc) const;

protected:

  AbstractRegistry();              // Abstract class, can not be created
  
  struct Entry {                   // mapping name -> factory
    std::string name;
    std::string desc;
    Factory* factory;
  };

  std::vector< Entry > entries;
};

#define ADD_MODULE_DEFINITION(_type_) \
class _type_; \
class _type_##Factory : public AbstractRegistry { \
public: \
  static _type_##Factory* get(); \
  _type_* create(const char* name) const; \
protected: \
  static _type_##Factory* the##_type_##Factory; \
};

FOR_EACH_MODULE(ADD_MODULE_DEFINITION);

#define REGISTER(_type_,_name_,_desc_) \
class _##_type_##factory_##_name_ : public Tapir::_type_##Factory::Factory { \
public: \
  _##_type_##factory_##_name_() { \
    Tapir::_type_##Factory::get()->add(#_name_, _desc_, this); \
  } \
  void* create () const { return new _name_ (); } \
}; \
static _##_type_##factory_##_name_ *_##_type_##factory_instance_##_name_ = new _##_type_##factory_##_name_ ();

/** Central registry of key mappings. **/
class KeyRegistry
{
public:
  static KeyRegistry* instance ();
  bool signup (const long int cmd, TapirModule* module, const char* entry, const char* name, const char* desc, bool print=true);
  bool delegate (const long int cmd, const char* params);
  std::stringstream list;
  const char* fname;

protected:
  KeyRegistry() {};
  std::map<long int, TapirModule*> _peres;
  std::map<long int, long int> _trans;
};

#define REGISTER_KEY(_name_,_key_,_desc_) \
  Tapir::KeyRegistry::instance()->signup((long int)_key_,this,#_key_,#_name_,_desc_);
#define REGISTER_HIDDEN_KEY(_name_,_key_) \
  Tapir::KeyRegistry::instance()->signup((long int)_key_,this,#_key_,#_name_,"",false);
#define REGISTER_KEY_ALIAS(_name_,_key_,_desc_,_alias_) \
  Tapir::KeyRegistry::instance()->signup((long int)_key_,this,_alias_,#_name_,_desc_);
#define REGISTER_EXTENDED_KEY(_name_,_alias_,_seq0_,_seq1_,_seq2_,_desc_) \
  Tapir::KeyRegistry::instance()->signup((_seq0_+_seq1_*256+_seq2_*65536),this,_alias_,#_name_,_desc_);

}
#endif

