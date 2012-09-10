/***************************************************************************
*    score.h
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
#include "aligns.h"
#include "load.h"
#include "pvector.h"
#include "merger.h"
#include "ngrams.h"

#ifndef _SCORE_H
#define _SCORE_H

#define ZERO_PROB 1e-30

void sort_by_key(pcooc_vector_t * ,unsigned =MEM_BLOCK );
void sort_by_val(pcooc_vector_t * ,unsigned =MEM_BLOCK );

typedef pair<double,align_info> lex_pair_t;
typedef vector< lex_pair_t> lex_pair_vec_t;

typedef lex_pair_t (*aggregation_func)(const lex_pair_vec_t &);

double lex_score(char *,char * ,char * ,align_info * ,LexMap * );

double lex_score(char *,char * ,char * ,LexMap * );
void score_one_source(pcooc_vector_t *,Merger<cooc_type>/*pcooc_vector_t::merge_iterator*/ * =NULL, aggregation_func=NULL);
unsigned long score_one_source(pcooc_vector_t *,unsigned long * big_n1,unsigned long * big_n2,unsigned long * big_n3,unsigned long * big_n4,
                                unsigned long *,Merger<cooc_type>/*pcooc_vector_t::merge_iterator*/ * =NULL,
                                Merger<ngram_t> * =NULL,pindicator_vector_t * =NULL);

void score(pcooc_vector_t ** /*,pcooc_vector_t */ ,global_score_type * =NULL,unsigned long =MEM_BLOCK,unsigned=0 ,
        const char * =NULL,const char * =NULL,pindicator_vector_t *  = NULL);

#endif
