#ifndef _registry_h_
#define _registry_h_

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

#include <vector>
#include <string>

#define FOR_EACH_CLS_MODULE_TYPE(_macro_) \
  _macro_(Plant)                          \
  _macro_(Controller)                     \
  _macro_(Reward)                         \
  _macro_(Graphic)                        \
  _macro_(Statistics)                     \
  _macro_(Observer)                       \
  _macro_(Input)                          \
  _macro_(Output)

using namespace std;

namespace CLS
{

#define CLS_FACTORY_DECLARE_MODULE_TYPE(_type_) \
  class _type_;
FOR_EACH_CLS_MODULE_TYPE(CLS_FACTORY_DECLARE_MODULE_TYPE);

/** Central registry of factory classes that are able to construct instances
    of an unknown class. */
class AbstractRegistry
{
public:
  /** this interface declares an abstract factory, that is able to construct
      instances of one class. Each class needs a different implementation of
      the abstract factory. */
  class Factory
  {
  public:
    virtual ~Factory() {}
    virtual void* create() const = 0;
  };

  virtual ~AbstractRegistry();

  /** add a factory for the class "name" */
  virtual bool add(const char* name, const char* desc, Factory* factory);

  /** lookup a factory for the requested class "name" */
  virtual Factory* lookup(const char* name) const;

  virtual int listEntries(char* buf, int buf_size) const;

  virtual void listEntries(std::vector<std::string>* names , std::vector<std::string>* desc) const;

protected:

  AbstractRegistry();              // Abstract class, can not be created

  struct Entry {                   // mapping name -> factory
    // use char arrays over strings here to prevent possible memory leaks in COW
    const char *name;
    const char *desc;
    Factory* factory;
  };

  std::vector< Entry > entries;
};

/** Central registry of factory classes for one module type. Implemented as singleton. */
#define CLS_FACTORY_DEFINE_CLASS(_type_)             \
class _type_##Factory : public AbstractRegistry      \
{                                                    \
public:                                              \
  static _type_##Factory* getThe##_type_##Factory(); \
  static void deleteThe##_type_##Factory();          \
  _type_* create(const char* name) const;            \
protected:                                           \
  static _type_##Factory* the##_type_##Factory;      \
};
FOR_EACH_CLS_MODULE_TYPE(CLS_FACTORY_DEFINE_CLASS);


// Das folgende Makro generiert beim Aufruf eine Farbrikklasse, z.B.
// factory_MyPlant. Davon wird genau ein statisches Objekt angelegt, damit der
// Konstruktor der Farbikklasse aufgerufen wird. Dieser sorgt wiederum daf�r,
// dass die Fabrikklasse bei der zentralen Registry angemeldet wird.
// Von nun an steht die Fabrik in der Registry zur Verf�gung. Statische
// Variablen werden beim Programmstart generell initialisiert, bevor main
// aufgerufen wird.
#define REGISTER(_type_, _name_, _desc_)                                         \
class _##_type_##factory_##_name_ : public CLS::_type_##Factory::Factory {       \
public:                                                                          \
  _##_type_##factory_##_name_() {                                                \
    CLS::_type_##Factory::getThe##_type_##Factory()->add(#_name_, _desc_, this); \
  }                                                                              \
  void* create() const { return new _name_(); }                                  \
};                                                                               \
static _##_type_##factory_##_name_ *_##_type_##factory_instance_##_name_ =       \
  new _##_type_##factory_##_name_ ();

// For backward compatibility with older modules type-specific macros are
// provided, but should not be used anymore.
#define REGISTER_PLANT(_name_, _desc_) REGISTER(Plant, _name_, _desc_)
#define REGISTER_CONTROLLER(_name_, _desc_) REGISTER(Controller, _name_, _desc_)
#define REGISTER_OBSERVER(_name_, _desc_) REGISTER(Observer, _name_, _desc_)
#define REGISTER_REWARD(_name_, _desc_) REGISTER(Reward, _name_, _desc_)
#define REGISTER_GRAPHIC(_name_, _desc_) REGISTER(Graphic, _name_, _desc_)
#define REGISTER_STATISTICS(_name_, _desc_) REGISTER(Statistics, _name_, _desc_)
#define REGISTER_INPUT(_name_, _desc_) REGISTER(Input, _name_, _desc_)
#define REGISTER_OUTPUT(_name_, _desc_) REGISTER(Output, _name_, _desc_)


/** This macro creates an implementation of the abstract factory for the
    class "name" and adds it to the central CONTROLLER factory.
    This is the special case of a controller in a loadable library.
*/
#define REGISTER_LIB_CONTROLLER(name, desc)                                \
class _controllerfactory_##name : public CLS::ControllerFactory::Factory { \
public:                                                                    \
  _controllerfactory_##name() {;}                                          \
  void* create() const { return new name (); }                             \
};                                                                         \
extern "C"                                                                 \
{                                                                          \
   bool create()                                                           \
   {                                                                       \
      return CLS::ControllerFactory::getTheControllerFactory()->add(#name, \
                                 desc, new _controllerfactory_##name () ); \
   }                                                                       \
};

typedef void create_t;

} // namespace CLS

#endif
