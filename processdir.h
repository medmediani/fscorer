/***************************************************************************
*    processdir.h
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
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <iostream>

#ifndef _PROCESSDIR_H
#define _PROCESSDIR_H
/*
#ifdef USE_GZIP
 #undef USE_GZIP
#endif*/

#define PATH_SEP '/'


using namespace std;

bool isdir(string );
bool remove_tree(string ) ;


#endif