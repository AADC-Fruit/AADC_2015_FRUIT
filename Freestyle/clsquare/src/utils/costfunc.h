/*
CLSquare
Copyright (c) Martin Riedmiller
Author: Martin Riedmiller
All rights reserved.

No distribution without permission.
*/


#ifndef _COSTFUNC_H_
#define _COSTFUNC_H_

#include "utils/setdef.h"

namespace CLS {
namespace Util {

class CostFunc{
 public:
  bool init(const int xdim, const int udim, const char *fname=0, const char *section_name=0);
  bool deinit(); //terminate
  double get_costs_standard(const double* state, const double* action, const double* nextstate);
  double get_costs_absdist(const double* state, const double* action, const double* nextstate);
  bool in_xplus(const double *state);
  bool in_xplusterminal(const double *state);
  bool is_failure(const double *state);
  void get_center_of_xplusterminal(double *result);

  double costs_for_terminal_failure;
  double costs_for_terminal_success;
  double costs_inside_xplus;
  double costs_outside_xplus;

 protected:
     
  bool read_options(const char * fname, const char *section_name);

  int u_dim, x_dim;
  double* weights;

  /** definition of working state set. */
  CLS::Util::SetDef  XworkSet;
  CLS::Util::SetDef  XplusSet;
  CLS::Util::SetDef  XminusSet;
  CLS::Util::SetDef  xplusterminal;
};

};
};

#endif

