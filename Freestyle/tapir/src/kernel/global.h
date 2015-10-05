#ifndef _TAPIR_GLOBAL_H_
#define _TAPIR_GLOBAL_H_

#include "registry.h"
#include "valueparser.h"
#include "object.h"
#include <iostream>
#include <cstdio>
#include <opencv2/opencv.hpp>

#define EOUT(xxx) std::cerr << "\033[1;31m#ERROR\033[0m   [" << __PRETTY_FUNCTION__ << "]: " << xxx << std::endl
#define WOUT(xxx) std::cerr << "\033[1;33m#WARNING\033[0m [" << __PRETTY_FUNCTION__ << "]: " << xxx << std::endl
#define IOUT(xxx) std::cout << "#INFO    [" << __PRETTY_FUNCTION__ << "]: " << xxx << std::endl

#define SET_STRING(_tar_,_src_) { _tar_ = new char[strlen(_src_)+1]; strncpy(_tar_, _src_, strlen(_src_)); _tar_[strlen(_src_)]='\0'; }
#define MAX_STR_LEN 512
#define OUTSPACE "         "

#endif
