/*
CLSquare
Copyright (c) Martin Riedmiller
Author: Martin Riedmiller
All rights reserved.

No distribution without permission.
*/

#include "valueparser.h"
#include "utils/costfunc.h"
#include <math.h>
#include <string.h>

using namespace CLS::Util;

bool CostFunc::init(const int xdim, const int udim, const char *fname, 
		    const char *section_name){
    
  //defaults
  x_dim = xdim;
  u_dim = udim;
  costs_for_terminal_failure = 1000;
  costs_for_terminal_success = 0;
  costs_inside_xplus = 0;
  costs_outside_xplus = 1; 
  
  weights = new double[x_dim];
  for(int i= 0; i< x_dim; i++)
    weights[i] = 0;

  XworkSet.setDefaults(x_dim); // full set
  XminusSet.setDefaults(x_dim,1,-1); // empty set
  XplusSet.setDefaults(x_dim,1,-1); // empty set
  xplusterminal.setDefaults(x_dim,1,-1); // empty set

  return read_options(fname, section_name);
}

bool CostFunc::in_xplus(const double *state){
  if( XplusSet.isWithinSet(state,x_dim) == true)
    return true;
  else
    return false;
}

bool CostFunc::in_xplusterminal(const double *state){
  if( xplusterminal.isWithinSet(state,x_dim) == true)
    return true;
  else
    return false;
}

bool CostFunc::is_failure(const double *state){
  if (XworkSet.isWithinSet(state,x_dim) == false)
    return true;
  if (XminusSet.isWithinSet(state,x_dim) == true)
    return true;
  return false;
 }

void CostFunc::get_center_of_xplusterminal(double *result){
  for(int i=0;i<x_dim;i++){
    if(isnan(xplusterminal.subsets[0][i].min) || isnan(xplusterminal.subsets[0][i].max))
      result[i] = 0.0;
    else
      result[i] = (xplusterminal.subsets[0][i].min + xplusterminal.subsets[0][i].max) / 2.0;
  }
}

double CostFunc::get_costs_standard(const double* state, const double* action, const double* nextstate){

  double costs;

  if(is_failure(nextstate)){
    costs = costs_for_terminal_failure; 
  }
  else if(in_xplusterminal(nextstate)){
    costs = costs_for_terminal_success; 
  }
  else if(in_xplus(nextstate)){
    costs = costs_inside_xplus;
  }
  else
    costs = costs_outside_xplus;
  //  std::cout<<"In costs. costs "<<costs<<std::endl;

  return costs;
}

double CostFunc::get_costs_absdist(const double* state, const double* action, 
				   const double* nextstate){

  double costs;

  if(is_failure(nextstate)){
    costs = costs_for_terminal_failure; 
    return costs;
  }
  if(in_xplusterminal(nextstate)){
    costs = costs_for_terminal_success; 
    return costs;
  }
  
  double *center =new double[x_dim];
  get_center_of_xplusterminal(center);
  costs = 0.0;
  for(int i=0;i<x_dim; i++)
    costs += weights[i] * fabs(nextstate[i]-center[i]);
  return costs;
}


bool CostFunc::read_options(const char * fname, const char * section_name) {
#define MAX_STR_LEN 500
  char tmp_str[MAX_STR_LEN];

  if(fname == 0)
    return true;

  CLS::ValueParser vp(fname,section_name);
  vp.get("weights",weights,x_dim);
  vp.get("costs_for_terminal_failure",costs_for_terminal_failure);
  vp.get("costs_for_terminal_success",costs_for_terminal_success);
  vp.get("costs_inside_target",costs_inside_xplus);
  vp.get("costs_outside_target",costs_outside_xplus);

  if(vp.get("xwork",tmp_str,MAX_STR_LEN)>=0) 
    XworkSet.parseStr(tmp_str, x_dim);

  if(vp.get("xminus",tmp_str,MAX_STR_LEN)>=0)
    XminusSet.parseStr(tmp_str, x_dim);
  
  if(vp.get("xplus",tmp_str,MAX_STR_LEN)>=0)
    XplusSet.parseStr(tmp_str, x_dim);

  if(vp.get("xplusterminal",tmp_str,MAX_STR_LEN)>=0)
    xplusterminal.parseStr(tmp_str, x_dim);

  // check parameters and stop, if an entry is unknown
  if ( vp.num_of_not_accessed_entries() ) {
    std::cout << " unrecognized options in config-file "<<fname<<", section "<<section_name<<":"
	      <<vp.show_not_accessed_entries(std::cout)<<std::endl;
    //    WARNINGOUT(ss.str()); exit(0);
    //    NFQCALIB_QUIT(ss.str());
    return false;
  }
  return true;
}

