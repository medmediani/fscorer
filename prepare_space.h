/***************************************************************************
*    prepare_space.h
*    Part of fscorer	: a parallel phrase scoring tool 
*                         for extra large corpora
*    copyright            : (C) 2012 by Mohammed Mediani
*    email                : mohammed.mediani@kit.edu
****************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/
#include <cstdio>
#include <iomanip>
#include <vector>
#include <sstream>
#include <stdarg.h>

#include <stxxl/io>
#include <stxxl/aligned_alloc>
#include <unistd.h>
#include "common.h"

#ifndef _PREPARE_SPACE_H
#define _PREPARE_SPACE_H

#ifdef BLOCK_ALIGN
 #undef BLOCK_ALIGN
#endif

#define BLOCK_ALIGN  4096

#define NOREAD

//#define DO_ONLY_READ

#define POLL_DELAY 1000

#define RAW_ACCESS

//#define WATCH_TIMES
/*
#ifdef USE_GZIP
 #undef USE_GZIP
#endif*/

#define CHECK_AFTER_READ 0
string tmp_name(string =".",bool =true);
void remove_tmp(const str_vec_t &);//(const char * );
void prepare(stxxl::int64,const str_vec_t &);//const char *);
void create_space(stxxl::int64 , const char * ,... );

#endif