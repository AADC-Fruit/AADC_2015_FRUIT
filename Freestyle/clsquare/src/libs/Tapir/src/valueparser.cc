/*
 * Copyright (c) 1999 - 2001, Artur Merke <artur.merke@udo.edu> 
 *

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the <ORGANIZATION> nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE. 
*/

#include "tapir/valueparser.h"
#include "tapir/global.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <vector>

using namespace std;
namespace Tapir
{

#define _FNAME_ "\n(valueparser.C):"


KeyValueTab::KeyValueTab() {
  cur_size= 0;
  max_size= 0;
  tab= 0;
}

KeyValueTab::KeyValueTab(int new_max_size) {
  cur_size= 0;
  max_size= 0;
  tab= 0;
  set_max_size(new_max_size);
}

KeyValueTab::~KeyValueTab() {
  if (!tab)
    return;
  for (int i=0; i<cur_size;i++) {
    if (tab[i].key)
      delete[] tab[i].key;
    if (tab[i].val)
      delete[] tab[i].val;
  }
  delete[] tab;
}

bool KeyValueTab::set_cur_size(int size) {
  if (size < 0)
    return false;

  if (size <= max_size) 
    cur_size= size;
  else {
    if (size/2 > max_size)
      set_max_size(size+10);
    else
      set_max_size(2*size+10);
    cur_size= size;
  }
  return true;
}

bool KeyValueTab::set_key(int i, const char * value, int len) {
  if (i < 0 || i >= cur_size) {
    EOUT("KeyValueTab::set_key: index " << i << " not in range [0," << cur_size << ")");
    return false;
  }

  if (tab[i].key) {
    delete[] tab[i].key;
    tab[i].key= 0;
  }

  if (!value || len== 0)
    return true;

  if (len < 0)
    len= strlen(value);

  tab[i].key= new char[len+1];
  strncpy(tab[i].key,value,len);
  tab[i].key[len]= '\0';
  return true;
}

bool KeyValueTab::set_val(int i, const char * value, int len) {
  if (i < 0 || i >= cur_size) {
    EOUT("KeyValueTab::set_val: index " << i << " not in range [0," << cur_size << ")");
    return false;
  }

  if (tab[i].val) {
    delete[] tab[i].val;
    tab[i].val= 0;
  }

  if (!value || len == 0)
    return true;
  
  if (len < 0)
    len= strlen(value);

  tab[i].val= new char[len+1];
  strncpy(tab[i].val,value,len);
  tab[i].val[len]= '\0';
  return true;
}

bool KeyValueTab::append_val(int i, const char * value, bool sep) {
  if (i < 0 || i >= cur_size) {
    EOUT("KeyValueTab::append: index " << i << " not in range [0," << cur_size << ")");
    return false;
  }
  int my_len = 0;
  int val_len = 0;

  if (!value) 
    return true;

  val_len= strlen(value);
  if (tab[i].val) my_len= strlen(tab[i].val);

  char * dum;
  int size= my_len+ val_len+1;
  if (sep && my_len>0)
    size++;

  dum= new char[size];
  if (my_len>0) {
    strncpy(dum,tab[i].val,my_len);
    if (sep) {
      dum[my_len]= ' ';
      my_len++; 
    }
  }
  
  strcpy(dum+my_len,value);

  if (tab[i].val)
    delete[] tab[i].val;

  tab[i].val= dum;
  return true;
}

bool KeyValueTab::set_max_size(int new_max_size) {
  if (new_max_size<0) return false;
  if (new_max_size<=max_size) return true;
  if (cur_size > max_size)
    cur_size= max_size;
  max_size= new_max_size;
  if (cur_size<= 0 || tab==0) {
    if (tab) 
      delete[] tab;
    if (0==max_size) 
      tab= 0;
    else {
      tab= new Entry[max_size];
    }
  } else {
    Entry * dum= tab;
    tab= new Entry[max_size];
    memcpy( (void*)tab,(void*)dum, sizeof(Entry) * cur_size);
    delete[] dum;
  }
  return true;
}

ostream& operator<< (ostream& o,const KeyValueTab& t) {
  o << "\ncur_size= " << t.cur_size << ", max_size= " << t.max_size;
  for (int i= 0; i<t.cur_size; i++) 
    o << "\n" << t.tab[i].key << "= " << t.tab[i].val;
  return o;
}
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
ostream& operator<< (ostream& o,const ValueParser& t) {
  o << "\n<ValueParser>";
  o << t.kv_tab;
  o << "\n</ValueParser>";
  return o;
}


ValueParser::ValueParser(const char * fname,const char * block) 
  : kv_tab(50), max_key_length(0)
{
  set_verbose(false);
  set_warnings(false);

  ifstream *is = new ifstream(fname);
  if (!is->good()) { 
    EOUT("Unable to open file:  " << fname);
    exit(1);
    //return;
  }

  char line[maxLineLength+1];
  char* act;
  int lineNo=0;
  //cerr << "\nParsing file \"" << fname << "\" with block [" << block << "] ";
  bool in_active_block= false;
  int block_size= 0;
  if (block) block_size= strlen(block);

  if (block_size == 0) //
    in_active_block= true;

  std::vector<ifstream*> stack;
  while (is->good() || stack.size() > 0) {
    if (!is->good()) {
      is->close();
      is = stack.back();
      stack.pop_back();
      continue;
    }

    is->getline(line,maxLineLength); lineNo++;
    
    act=line;
    while (isspace(*act)) act++; //strip leading whitespace

    if (*act == '$') {
      stack.push_back(is);
      is = new ifstream(&act[2]);
      if (!is->good())
        EOUT("Sub-configuration file " << &act[2] << " could not be read.");
//      else
//        IOUT("Reading sub-configuration file " << &act[2] << ".");
      //cout << "\nAppending content of file \"" << &act[2] << "\" to configuration...";
      continue;
    }

    // Check if comment
    if (*act=='#') continue; // OK, comment
    if (*act=='[') { // OK, recognizes [block]
      in_active_block= false;
      act++;
      //cout << "\n act= " << act << "\n blockdata()= " << block.data() << "\n block_size= " << block_size;
      if ( 0==strncmp(act, block, block_size) ) {
        act+= block_size;
        if ( ']'== *act )
          in_active_block= true;
      }
      continue; 
    }

    if (!in_active_block)
      continue;

    read_line(act);
  }
  is->close();
}

ValueParser::ValueParser(int argc, char const* const * argv, const char *prefix_str)
  : kv_tab(50), max_key_length(0)
{
  set_verbose(false);
  set_warnings(false);
#if 1
  append_from_command_line(argc,argv,prefix_str);
#else
  int prefix_str_len= 0;
  bool valid_prefix= false;

  if (prefix_str)
    prefix_str_len= strlen(prefix_str);

  if (0 == argc) return;

  int idx= -1;
  for (int i=0; i< argc; i++) {
    // Free entries left?
    const char *  option_str= argv[i];

    if ( '-' == option_str[0] &&  int( option_str[1] ) >= 58 ) //must not begin with a ciffer
      if ( prefix_str_len && strncmp(option_str+1,prefix_str,prefix_str_len ) != 0 )
	valid_prefix= false;
      else {
	idx++;
	valid_prefix= true;
	if (idx >= kv_tab.cur_size)
	  kv_tab.set_cur_size(idx+1);
	
	kv_tab.set_key(idx,option_str + 1 + prefix_str_len);
      }
    else if (valid_prefix) {
      kv_tab.append_val(idx,option_str,true);
    }
  }
  //cerr << "->found " << numEntries << " entries.";
#endif
}

bool ValueParser::append_from_command_line(int argc, char const* const * argv, const char *prefix_str)
{
  int prefix_str_len= 0;
  bool valid_prefix= false;

  if (prefix_str)
    prefix_str_len= strlen(prefix_str);

  if (0 == argc) return true;

  int idx= kv_tab.cur_size-1;
  for (int i=0; i< argc; i++) {
    // Free entries left?
    const char *  option_str= argv[i];

    if ( '-' == option_str[0] &&  int( option_str[1] ) >= 58 ) //must not begin with a ciffer
      if ( prefix_str_len && strncmp(option_str+1,prefix_str,prefix_str_len ) != 0 )
	valid_prefix= false;
      else {
	idx++;
	valid_prefix= true;
	if (idx >= kv_tab.cur_size)
	  kv_tab.set_cur_size(idx+1);
	
	kv_tab.set_key(idx,option_str + 1 + prefix_str_len);
      }
    else if (valid_prefix) {
      kv_tab.append_val(idx,option_str,true);
    }
  }
  return true;
  //cerr << "->found " << numEntries << " entries.";
}

ValueParser::ValueParser(int mode, const char * line) 
  : kv_tab(10), max_key_length(0)
{
  set_verbose(false);
  set_warnings(false);
  read_line(line);
}

bool ValueParser::read_line(const char * line) {
  const char * act=line;
  while ( true ) {
    while( isspace(*act) )
      act++;

    if (*act=='\0' || *act=='#') 
      return true; // OK, comment or end

    if (*act== '=') {
      EOUT("ValueParser: wrong entry line:" << line);
      return false;
    }

    const char * dum= act;

    while ( !isspace(*dum) && *dum!= '\0' && *dum!= '=' && *dum!= '#')
      dum++;

    if (*dum== '\0' || *dum== '#' ){
      EOUT("ValueParser: wrong entry line:" << line);
      return false;
    }

    kv_tab.inc_cur_size();    
    kv_tab.set_last_key(act,dum-act); // located keyword

    while ( isspace(*dum) )
      dum++;

    if ( *dum != '=' ) {
      EOUT("ValueParser: wrong entry line:" << line);
      return false;
    }

    dum++;

    while ( isspace(*dum) )
      dum++;

    act= dum;
    /* search for the end of the value entry. this end is defined
       as the end of the last succeding string, which is not a key */

    while ( *dum != '\0' && *dum != '#' && *dum != '=' )
      dum++;

    if ( *dum == '\0' || *dum == '#') {
      dum--;
      while ( isspace(*dum) )
	dum--;
      dum++;
    }
    else { //go back until you are before the corresponding key
      dum--;
      while ( isspace(*dum) )
	dum--;

      
      if (*dum == '='){
	EOUT("ValueParser: wrong entry line:" << line);
	return false;
      }
      
      while ( ! isspace(*dum) )
	dum--;

      while ( isspace(*dum) )
	dum--;

      dum++;
    }

    kv_tab.set_last_val(act,dum-act);
    act= dum;
  }
  return true;
}

int ValueParser::num_of_not_accessed_entries() const {
  int res= 0;
  for (int i=0; i< kv_tab.cur_size; i++)
    if ( kv_tab.tab[i].access < 1) 
      res+= 1;
  return res;
}

int ValueParser::num_of_accessed_entries() const {
  int res= 0;
  for (int i=0; i< kv_tab.cur_size; i++)
    if ( kv_tab.tab[i].access > 0) 
      res+= 1;
  return res;
}

int ValueParser::show_not_accessed_entries(ostream & out) const {
  int res= 0;
  for (int i=0; i< kv_tab.cur_size; i++)
    if ( kv_tab.tab[i].access  < 1) {
      out << "\n"
	   << setw(max_key_length) << setiosflags(ios::left)
	   << kv_tab.tab[i].key << " = "
	   << kv_tab.tab[i].val;
    }
  return res;
}

int str_to_int_array(bool & warning,const char* value_str,int* value,int len) {
  if (!value_str) return -1;
  if (strlen(value_str)==0) {
    if (len>0) warning= true;
    return 0;
  }
  
  char* error_ptr;
  long tmp_long;

  for (int i=0; i< len; i++) {
    while (*value_str == ' ') value_str++;
    if (*value_str == '\0') {
      warning= true;
      return i;
    }

    tmp_long= strtol(value_str, &error_ptr, 10);

    if (*error_ptr != '\0' && *error_ptr != ' ') {
      warning= true;
      return i;
    }
    else if (ERANGE == errno || tmp_long > INT_MAX || tmp_long < INT_MIN ) {
      warning= true;
      return i;
    }

    value[i]= tmp_long;
    
    value_str= error_ptr;
  }
  while (*value_str == ' ') value_str++;
  if (*value_str != '\0') 
    warning= true;

  return len;
}

int str_to_long_array(bool & warning,const char* value_str,long* value,int len) {
  if (!value_str) return -1;
  if (strlen(value_str)==0) {
    if (len > 0) warning= true;
    return 0;
  }

  char* error_ptr;
  long tmp_long;
  ////////
  for (int i=0; i< len; i++) {
    while (*value_str == ' ') value_str++;
    if (*value_str == '\0') {
      warning= true;
      return i;
    }

    tmp_long= strtol(value_str, &error_ptr, 10);

    if (*error_ptr != '\0' && *error_ptr != ' ') {
      warning= true;
      return i;
    }
    else if (ERANGE == errno) {
      warning= true;
      return i;
    }

    value[i]= tmp_long;
    
    value_str= error_ptr;
  }
  while (*value_str == ' ') value_str++;
  if (*value_str != '\0') 
    warning= true;
  return len;
}

int  str_to_float_array(bool & warning,const char* value_str,float* value,int len) {
  if (!value_str) return -1;
  if (strlen(value_str)==0) {
    if (len > 0) warning= true;
    return 0;
  }

  char* error_ptr;
  double tmp_double;

  for (int i=0; i< len; i++) {
    while (*value_str == ' ') value_str++;
    if (*value_str == '\0') {
      warning= true;
      return i;
    }

    tmp_double= strtod(value_str, &error_ptr);

    if (*error_ptr != '\0' && *error_ptr != ' ') {
      warning= true;
      return i;
    }
    else if (ERANGE == errno) {
      warning= true;
      return i;
    }

    value[i]= float(tmp_double);
    
    value_str= error_ptr;
  }
  while (*value_str == ' ') value_str++;
  if (*value_str != '\0') 
    warning= true;
  return len;
}

int  str_to_double_array(bool & warning,const char* value_str,double* value,int len) {
  if (!value_str) return -1;
  if (strlen(value_str)==0) {
    if (len > 0) warning= true;
    return 0;
  };

  char* error_ptr;
  double tmp_double;

  for (int i=0; i< len; i++) {
    while (*value_str == ' ') value_str++;
    if (*value_str == '\0') {
      warning= true;
      return i;
    }

    tmp_double= strtod(value_str, &error_ptr);

    if (*error_ptr != '\0' && *error_ptr != ' ') {
      warning= true;
      return i;
    }
    else if (ERANGE == errno) {
      warning= true;
      return i;
    }

    value[i]= tmp_double;
    
    value_str= error_ptr;
  }
  while (*value_str == ' ') value_str++;
  if (*value_str != '\0') 
    warning= true;
  return len;
}

int str_to_bool_array(bool & warning,const char* value_str,bool* value,int len) {
  if (!value_str) return -1;
  if (strlen(value_str)==0) {
    if (len>0) warning= true;
    return 0;
  }
  
  int i;
  bool tmp_bool;
  for (i=0; i< len; i++) {
    while (*value_str == ' ' ) 
      value_str++;
    if (*value_str == '\0') {
      warning= true;
      return i;
    }

    if (strncmp("true",value_str,4)==0) {
      value_str+= 4;
      tmp_bool = true;
    }
    else if (strncmp("false",value_str,5)==0) {
      value_str+= 5;
      tmp_bool= false;
    }
    else {
      char *error_ptr= 0;
      long tmp_long = strtol(value_str, &error_ptr, 10);
      if ( tmp_long != 1 && tmp_long != 0) {
	warning= true;
      }
      tmp_bool= bool(tmp_long);
      value_str= error_ptr;
    }
    if ( *value_str == '\0' || *value_str == ' ') {
      value[i]= tmp_bool;
    }
    else {
      warning= true;
      return i;
    }
  }
  return len;
}

int str_to_char_array(bool & warning,const char* value_str,char* value,int len) {
  if (!value_str) return -1;
  int dum= strlen(value_str);
  if (dum == 0) { 
    if (len!= 0) warning= true;
    return 0;
  }
  for (int i=0; i<len; i++) {
    if (i> dum) {
      //warning= true; keien Warnung, falls String kuerzer als angegeben (nur bei char arrays)
      return i;
    }
    value[i]= value_str[i];
  }
  if (len <= dum) warning= true; // Warnung, falls String laenger als angegeben 
  return len;
}

int ValueParser::getValue(const char * key,int type,void* value,int arr_length) const {
  const char* value_str= 0;
  int  value_str_len= 0;
  int key_idx= -1;
  bool warning= false;
  int read_entries= -1;

  for (int idx= kv_tab.cur_size-1; idx >= 0; idx--) {
    KeyValueTab::Entry * entry= kv_tab.tab+idx;
    if ( entry->key == 0) {
      EOUT("at index " << idx << " a zero key entry");
      continue;
    }
    if ( strcmp(key,entry->key) != 0 ) 
      continue;
    
    entry->access += 1;
    if (key_idx < 0) 
      key_idx= idx;
    else {
      IOUT("ignoring because of multiple definition: " 
         << setw(max_key_length) << setiosflags(ios::left)
         << key << " = ["
         << entry->val << "]");
      entry= kv_tab.tab+key_idx;
      IOUT("using (last definition): [" << entry->val << "]");
    }
  }

  if (key_idx<0) {
    if ((warnings_mode || verbose_mode))  
      EOUT("key= " << key << "   WARNING: NO SUCH ENTRY WAS FOUND");
    return read_entries; 
  }

  value_str= kv_tab.tab[key_idx].val;
  value_str_len = 0;
  if (value_str) value_str_len= strlen(value_str);
    
  if (0 == value_str_len && arr_length != 0 && type!= type_string)
    warning= true;
    
  switch (type) {
  case type_int:
    read_entries= str_to_int_array(warning,value_str,(int*)value,arr_length);
    break;
  case type_long:
    read_entries= str_to_long_array(warning,value_str,(long*)value,arr_length);
    break;
  case type_float:
    read_entries= str_to_float_array(warning,value_str,(float*)value,arr_length);
    break;	
  case type_double:
    read_entries= str_to_double_array(warning,value_str,(double*)value,arr_length);
    break;
#if 0
  case type_string:
    if (0==value_str_len) 
      break;
    *((string*) value)=values[i];
    break;
#endif
  case type_bool:
    read_entries= str_to_bool_array(warning,value_str,(bool*)value,arr_length);
    break;
  case type_char:
    read_entries= str_to_char_array(warning,value_str,(char*)value,arr_length);
    break;
  }

  if (read_entries < 0) 
    read_entries= 0; //the key was there, so the minimum of options is 0

  if ( (warning && warnings_mode) || verbose_mode) {
    IOUT("entry: res= " << read_entries);
    showValue(key,type,value,arr_length);
    WOUT(10, "   WARNING: read from  [" << value_str << "]");
  }

  return read_entries;
}

void ValueParser::showValue(const char * key,int type,void* value,int arr_length) const {
  cout << "    "
       << setw(max_key_length) << setiosflags(ios::left)
       << key << " = [";
  
  for (int i=0; i<arr_length; i++) {
    //cout << " ";
    bool stop= false;
    switch (type) {
    case type_int:
      cout << ((int*) value)[i]; 
      if (i<arr_length-1) cout << " ";
      break;
    case type_long:
      cout << ((long*) value)[i]; 
      if (i<arr_length-1) cout << " ";
      break;
    case type_float:
      cout << ((float*) value)[i]; 
      if (i<arr_length-1) cout << " ";
      break;
    case type_double:
      cout << ((double*) value)[i]; 
      if (i<arr_length-1) cout << " ";
      break;
#if 0
    case type_string:
      cout << *((string*) value); 
      break;
#endif
    case type_bool:
      if ( ((bool*)value)[i] ) 
	cout << "true";
      else 
	cout << "false";
      if (i<arr_length-1) cout << " ";
      break;
    case type_char:
      if ( ((char*) value)[i] == '\0') {
	cout << "\\0";
	stop= true;
      }
      cout << ((char*) value)[i]; 
      break;
    default: cout << "< ? >";
    }
    if (stop)
      break;
  }    
  cout << "]\n";
}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/******************************* T E S T *************************************/
#if 0

void test() {
  const int maxLineLength= 1024;
  const char fname[]= "test.txt";
  ifstream is(fname);
  if (!is) { 
    cerr << "\nCannot open control file" << fname;
    exit(1);
    //return;
  }

  char line[maxLineLength+1];

  char fun[20];
  int size;
  int rand;
  double err;

  while (is) {
    is.getline(line,maxLineLength); 
    const char * dum= line;
    while ( isspace(*dum) )
      dum++;

    if ( *dum == '\0' || *dum == '#' ) //comment or blank line
      continue; 

    ValueParser vp(0,line);
    
    vp.get("fun",fun,20);
    vp.get("size",size, -1);
    vp.get("rand",rand,-1);
    int res= vp.get("err",err);
    if (res != 1 || size < 0 || rand < 0) {
      cerr << "\nerrornous line [" << line << "]";
    }
    else
      cout << "\n" << fun << " | " << size << " | " << rand << " -> " << err;
  }
  cout << endl;
}

int main (int argc,char **argv) {
  test(); return 0;

  KeyValueTab st;
  st.set_cur_size(20);
  st.set_key(0,"bla");
  //return 1;

  //ValueParser P(argv[1],argv[2]);
  //ValueParser P(argc-1,argv+1);//,"d1_");
  //ValueParser P("vp.test","BLOCK");
  if (argc< 2) {
    cout << "\nneed one argument";
    return 1;
  }
  ValueParser P(0,argv[1]);
  P.set_verbose(true);
  cout << P;
  bool h;
  P.get("help",h);

  //int i;
  bool b_arr[4];
  int i_arr[4];
  long l_arr[4];
  char c_arr[4];
  char c2_arr[4];
  float f_arr[4];
  double d_arr[4];

  int res;

  res= P.get("l",l_arr,4,-100);   
  cout << "\n res= " << res << "\n----------";
  res= P.get("b",b_arr,4,false);  
  cout << "\n res= " << res << "\n----------";
  res= P.get("c",c_arr,4,'\0');    
  cout << "\n res= " << res << "\n----------";
  res= P.get("c2",c2_arr,4,"123");
  cout << "\n res= " << res << "\n----------";
  res= P.get("i",i_arr,4,-100);   
  cout << "\n res= " << res << "\n----------";
  res= P.get("f",f_arr,4,-100.11);
  cout << "\n res= " << res << "\n----------";
  res= P.get("d",d_arr,4,-100.55);
  cout << "\n res= " << res << "\n----------";

  cout << endl;

  P.show_not_accessed_entries(cerr);
  cerr << "\nnumber of accessed entries= " << P.num_of_accessed_entries() << endl;
  return 0;
}

#endif

};
