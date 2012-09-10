/***************************************************************************
*    ngrams.h
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

#include <stxxl.h>
#include <stxxl/map>
#include "common.h"

#ifndef NGRAMS_H
#define	NGRAMS_H

#define MAXW 100


using namespace std;

struct ngram_t
{
    typedef char key_t[MAX_KEY_LENGTH];
    key_t _key;
    
    char * key() //const
    {
		
        return _key; //src_id;
    }
    
    ngram_t(){};
    ngram_t(const char * ng){strcpy(_key,ng);};
    ngram_t(const ngram_t& other){
        strcpy(_key,other._key);
    }
    
    static ngram_t min_value()
    {
        ngram_t m;
        memset(m._key,0,sizeof m._key);
        return m;
       
    }
    
    static ngram_t max_value()
    {
        ngram_t m;
        memset(m._key,0xff,sizeof m._key);
        m._key[sizeof(m._key)-1]='\x0';
        return m;
    } 
      
    bool operator ==(ngram_t  other) const
    {
        return !strcmp(_key,other._key); //(_src_id==other._src_id) && (_trgt_id==other._trgt_id);}
    }
    
    bool operator !=(ngram_t  other) const
    {
        return strcmp(_key,other._key); //(_src_id!=other._src_id) || (_trgt_id!=other._trgt_id);}
    
    }
    
};


struct CmpNgrams: public std::binary_function<ngram_t, ngram_t, bool>
{
    bool operator () (const ngram_t & a, const ngram_t & b) const
    {
	//cout<<"Comparing\n";	//printf("Comparin");
//        if(a.key()==b.key())
//			return a.val() < b.val();
	return strcmp(a._key,b._key)<0;//a.key() < b.key();
    }
    static ngram_t min_value()
    {
        return ngram_t::min_value();
    }
    static ngram_t max_value()
    {
        return ngram_t::max_value();
    }
};

bool operator < (const ngram_t & ,const  ngram_t & );
//#define NODE_BLOCK_SIZE         (32 * 1024)
//#define LEAF_BLOCK_SIZE         (32 * 1024)
//
//#define TOTAL_CACHE_SIZE        (750 * 1024 * 1024)
////#define TOTAL_CACHE_SIZE      (150 * 1024 * 1024)
//
////#define NODE_CACHE_SIZE       (1 * (TOTAL_CACHE_SIZE / 40))
////#define LEAF_CACHE_SIZE       (39 * (TOTAL_CACHE_SIZE / 40))
//
//#define NODE_CACHE_SIZE         (64 * 1024 * 1024)
//#define LEAF_CACHE_SIZE         (TOTAL_CACHE_SIZE - NODE_CACHE_SIZE)


//typedef stxxl::map<ngram_t,unsigned long,CmpNgrams,/*sizeof(ngram_t)*16*1024,sizeof(ngram_t)*128*1024*/NODE_BLOCK_SIZE, LEAF_BLOCK_SIZE> ngram_list_t;
typedef pvector<ngram_t> pngram_vector_t;

typedef pngram_vector_t::stxxl_vec ngram_vector_t;//element_vector;


 

typedef std::vector<unsigned> uvec_t;

void psave_ngrams(const char * ,pngram_vector_t * );
void sort_unique_ngrams(pngram_vector_t * , unsigned long =MEM_BLOCK);
void pget_ngrams(const char * ,pngram_vector_t * ,unsigned =7);
void pload_ngrams(const char * ,pngram_vector_t * );

#endif	/* NGRAMS_H */

