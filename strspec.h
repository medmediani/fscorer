/***************************************************************************
*    strspec.h
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
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <vector>
#include "aligns.h"
#include "common.h"

#ifndef _STRSPEC_H
#define _STRSPEC_H
/*
#ifdef USE_GZIP
 #undef USE_GZIP
#endif*/

#ifndef STR_SEP
#define STR_SEP '\x01'
#endif
using namespace std;

typedef vector<string> str_vec;


void tokenize_in_vec(char * ,str_vec * );
void get_alignments(char * ,aligns * );


void invert_alignment(char * );
void flip_around(char * str,char c,int =-1,int =-1);
void reverse_chars(char * str,int =-1,int =-1);
inline const char * first_part(const char * pair)
{
    return pair;
}
inline const char * last_part(const char * pair)
{
    return pair+strlen(pair)+1;
}

inline void break_pair(char * pair)
{
    *strchr(pair,STR_SEP)='\0';
}


pair<string,string> split(string ,string =" ");
pair<string,string> rsplit(string ,string =" ");
pair<string,string> split(string ,char );
pair<string,string> rsplit(string ,char);
#endif
