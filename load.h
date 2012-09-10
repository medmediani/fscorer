/***************************************************************************
*    load.h
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
#include "common.h"
#include "crono.h"
#include "tqueue.h"
#include "strspec.h"

#include "pvector.h"

#ifndef _LOAD_H
#define _LOAD_H
void create_lex_map(File ,LexMap * );


                      
void create_maps(File ,LexMap *,LexMap * ,pcooc_vector_t * );

void pcreate_maps(const char * ,LexMap *,LexMap * ,pcooc_vector_t * );

bool insert_line( pcooc_vector_t * ,string ,LexMap *,LexMap * );

#endif
