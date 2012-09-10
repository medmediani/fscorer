/***************************************************************************
*    common.h
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

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <vector>
#include <string.h>

#include <stxxl/io>
#include <stxxl/mng>
#include <stxxl/ksort>
#include <stxxl/sort>
#include <stxxl/vector>
#include <string>
#include <map>
#include <zlib.h>
#include <math.h>
#include "aligns.h"
#include "pvector.h"

using std::cout;
using std::endl;
using std::cerr;


using namespace pvectors;

#ifndef _COMMON_H
#define _COMMON_H

#define MEM_BLOCK (512 * 1024 * 1024)
#define DEFAULT_MEM_BLOCK 512
#define MAXL 1000
#define FIELD_SEP " ||| "
#define TASK_DONE "#"
#define MAX_ALIGNMENTS 8
#define UNALIGNED "NULL"
#define STR_SEP  '\x01'
#define STTK_SEP  "#"
#define DEFAULT_LEX "model/lex.0-0.e2f"
#define DEFAULT_REVLEX "model/lex.0-0.f2e"
#define DEFAULT_EXTRACT "model/extract.0-0.gz"
#define DEFAULT_STAT "phrase-table.0-0.stat.half.f2e"
#define DEFAULT_TMP "/tmp"
#define DEFAULT_TMP_SIZE 0
#define STXXL_EXT ".stxxl"
#define STXXL_CFG_ENV_VAR "STXXLCFG"
#define STXXL_CFG_EXT ".cfg"
#define PATH_SEP '/'
#define MIN(X,Y) ((X)<=(Y)?(X):(Y))	
#define MAX(X,Y) ((Y)<=(X)?(X):(Y))
#define NB_WORKERS omp_get_max_threads()

#define REAL_ZERO 1e-100
#define FACTOR 3

//different output formats
#define ABRIDGED_MOSES 0
#define MOSES 1


//different smoothing techniques
#define	NO_SMOOTHING 0
#define	KN_SMOOTHING 1
#define WB_SMOOTHING 2
#define GT_SMOOTHING 3

//different lexical score selection techniques
#define MAX_AGG 0
#define OCC_AGG 1
#define AVG_AGG 2

#ifndef MAX_KEY_LENGTH
	#define MAX_KEY_LENGTH 250
#endif

#define BUF_SIZE (2*1024*1024ull)


#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif


	typedef map<string, long > PhraseMap;
	typedef map<long, string > PhraseIdMap;
	typedef map<string,double > LexMap;


#ifdef USE_GZIP
    #undef fopen
    #undef fgets
    #undef fclose
    #undef fputs
    #undef fprintf
    #undef feof
    #undef FILE
    #undef fflush
    #undef fread
    #undef fwrite
    #undef fseek
    #undef ftell
            
    #define File gzFile
    #define fopen gzopen
    #define fdopen gzdopen
    #define fgets(X,Y,Z) gzgets(Z,X,Y)
    #define fclose gzclose
    #define fputs(X,Y) gzputs(Y,X)
    #define fprintf gzprintf
    #define feof gzeof
    #define fflush(X) gzflush(X, Z_SYNC_FLUSH)
    #define fread(W,X,Y,Z) gzread(Z,W,X*Y)
    #define fwrite(W,X,Y,Z) gzwrite(Z,W,X*Y)
    #define fseek gzseek
    #define ftell gztell
#else
    #define File FILE *
        
#endif

typedef vector<string> str_vec_t;

struct global_score_type
{
	unsigned long 	big_n1,
					big_n2,
					big_n3,
					big_n4,
					all;
	unsigned long long all_occurs;
	double D[3];
	double gt[2];
	void estimate_global_params()
	{
		//Compute D1, D2, D3
		double y=(double)big_n1/(big_n1+2*big_n2);
		double frac;
		
		for(int i=1;i<=3;++i){
				
			switch(i){
				case 1:	frac=(double)big_n2/big_n1;break;
				case 2: frac=(double)big_n3/big_n2;break;
				case 3: frac=(double)big_n4/big_n3;break;
			}
			
			D[i-1]=i-(i+1)*y*frac;
		}

		//Compute good-turing two parameters
// 		gt[0]=pow((double)big_n3/big_n1,0.06)*
// 			  pow((double)big_n4/big_n2,0.14)*
// 			  pow((double)big_n4/big_n1,0.25);

			  // fitting against non-log count
// 		gt[1]=pow(big_n1,0.64)*
// 		      pow(big_n2,0.46)*
// 			  pow(big_n3,0.16)*
// 			  pow(big_n4,-0.26)/all;
			 //fitting against log count	
		gt[0]=pow(big_n1,-0.565)*
			  pow(big_n2,-0.365)*
			  pow(big_n3,0.126)*
			  pow(big_n4,0.804);
		gt[1]=pow(big_n1,0.678)*
		      pow(big_n2,0.573)*
			  pow(big_n3,0.171)*
			  pow(big_n4,-0.422)/all;		
	}
	global_score_type():big_n1(0),big_n2(0),big_n3(0),big_n4(0),all(0),all_occurs(0)
	{		
// 		D[0]=D[1]=D[2]=gt[0]=gt[1]=-1;
	}
	inline double d(int i)
	{
		return D[i-1];
	}
	double gt_count(unsigned long n)
	{
		return (n+1)*pow(gt[0],log(1+1./n));//return (n+1)*gt[0];
	}
	inline double gt_discount()
	{
		return gt[1];
	}
};

struct cooc_type
{
    typedef char key_t[MAX_KEY_LENGTH];

    key_t _key;

	
	unsigned long 	cooc;
	///TODO

	unsigned long 	socc,
					tocc,
					n1,
					n2,
					n3p;
	
	unsigned long 	invn1,
					invn2,
					invn3p;
	
	double src_lex_score,tr_lex_score;
	align_info align;

//Kneser-Ney discounting
	double kn_smoothed_source_score(global_score_type * gs)
	{
		double d=gs->d(cooc<=2? cooc:3);
		
		double first=(cooc-d)/socc;
		double alpha=(n1*gs->d(1)+n2*gs->d(2)+n3p*gs->d(3))/socc;//gs->all;
		double pb=(double)(invn1+invn2+invn3p)/gs->all;
		return first+alpha*pb;
	}
	
	double kn_smoothed_target_score(global_score_type * gs)
	{
		double d=gs->d(cooc<=2? cooc:3);
		
		double first=(cooc-d)/tocc;
		double alpha=(invn1*gs->d(1)+invn2*gs->d(2)+invn3p*gs->d(3))/tocc;//gs->all;
		double pb=(double)(n1+n2+n3p)/gs->all;
		return first+alpha*pb;
	}
//Witten-bell discounting
	double wb_smoothed_source_score()
	{
		return ((double)cooc)/(socc+n1+n2+n3p);
	}
	
	double wb_smoothed_target_score()
	{
		return ((double)cooc)/(tocc+invn1+invn2+invn3p);		
	}
//Good-Turing "estimated"

	double gt_smoothed_source_score(global_score_type * gs)
	{
		return gs->gt_count(cooc)/(socc*gs->gt_discount()+n1*gs->gt_count(1)+n2*gs->gt_count(2)+n3p*gs->gt_count(3));
	}
	
	double gt_smoothed_target_score(global_score_type * gs)
	{
		return gs->gt_count(cooc)/(tocc*gs->gt_discount()+invn1*gs->gt_count(1)+invn2*gs->gt_count(2)+invn3p*gs->gt_count(3));
	}
//    key_type
    char * key() //const
    {
		
        return _key; //src_id;
    }
//
//	key_type val() const
//	{
//		return _trgt_id;
//	}
	
	cooc_type():n1(0),n2(0),n3p(0),invn1(0),invn2(0),invn3p(0),
				cooc(0),socc(0),tocc(0),src_lex_score(0.0),tr_lex_score(0.0)
				{ }
	cooc_type(/*key_type sid,key_type tid=0*/ const char * pair)://src_id(sid),_trgt_id(tid),
					n1(0),n2(0),n3p(0),invn1(0),invn2(0),invn3p(0),
					cooc(0),socc(0),tocc(0),src_lex_score(0.0),tr_lex_score(0.0)
					{strcpy(_key,pair); }
    
	cooc_type(/*key_type sid,key_type tid=0*/ const char * pair,double src_score,double tr_score,align_info * al_inf)://_src_id(sid),_trgt_id(tid),
					n1(0),n2(0),n3p(0),invn1(0),invn2(0),invn3p(0),
					cooc(1),socc(1),tocc(1),src_lex_score(src_score),tr_lex_score(tr_score){
                                                                strcpy(_key,pair);
								if(al_inf) 
									memcpy(&align,al_inf,sizeof(align_info));
								else
                                                                        align=align_info();
					}
                                       
     cooc_type(const cooc_type& other)://_src_id(other._src_id),_trgt_id(other._trgt_id),
 					n1(other.n1),n2(other.n2),n3p(other.n3p),invn1(other.invn1),invn2(other.invn2),invn3p(other.invn3p),
 					cooc(other.cooc),socc(other.socc),tocc(other.tocc),src_lex_score(other.src_lex_score),tr_lex_score(other.tr_lex_score){
         strcpy(_key,other._key);
         memcpy(&align,&other.align,sizeof(align_info));
     }
	
    static cooc_type min_value()
    {
        cooc_type m;
        memset(m._key,0,sizeof m._key);
        return m;

    }
    static cooc_type max_value()
    {
        cooc_type m;
        memset(m._key,0xff,sizeof m._key);
        m._key[sizeof(m._key)-1]='\x0';
        return m;


    } 
	
	bool operator ==(const cooc_type & other)
        {
            return !strcmp(_key,other._key); //(_src_id==other._src_id) && (_trgt_id==other._trgt_id);}
        }
        bool operator !=(const cooc_type & other)
        {
            return strcmp(_key,other._key); //(_src_id!=other._src_id) || (_trgt_id!=other._trgt_id);}
        }
};

bool operator < (const cooc_type & a,const  cooc_type & b)
{
    return strcmp(a._key,b._key)<0;//a.key()<b.key();
}
//

struct CmpKey//: public std::binary_function<cooc_type, cooc_type, bool>
{
    bool operator () (const cooc_type & a, const cooc_type & b) const
    {
	//cout<<"Comparing\n";	//printf("Comparin");
//        if(a.key()==b.key())
//			return a.val() < b.val();
	return strcmp(a._key,b._key)<0;//a.key() < b.key();
    }
    static cooc_type min_value()
    {
        return cooc_type::min_value();
    }
    static cooc_type max_value()
    {
        return cooc_type::max_value();
    }
};

std::ostream & operator << (std::ostream & o, const cooc_type & obj)
{
    cooc_type::key_t key;
    strcpy(key,obj._key);
    *strchr(key,STR_SEP)='\t';
    o << key<<"\t"<<obj.cooc<<
		"\t"<<obj.socc<<"\t"<<obj.tocc<<"\t"<<
		obj.n1<<"\t"<<obj.n2<<"\t"<<obj.n3p<<"\t"<<
		obj.invn1<<"\t"<<obj.invn2<<"\t"<<obj.invn3p<<"\t"<<obj.src_lex_score<<
		"\t"<<obj.tr_lex_score;
	return o;
}


typedef pvector<cooc_type> pcooc_vector_t;
typedef pcooc_vector_t::value_type value_type;
typedef pcooc_vector_t::stxxl_vec cooc_vector_t;//element_vector;

typedef  std::vector<value_type> local_vector;

typedef pvector<bool> pindicator_vector_t;
typedef pindicator_vector_t::stxxl_vec indicator_vector_t;


void print_cooc_vec(cooc_vector_t * cooc_vec,global_score_type * gs=NULL)
{
	cooc_vector_t::iterator itr;
	cout<<"src_id\ttrg_id\tcooc\ts_occ\tt_occ\tn1\tn2\tn3p\tinvn1\tinvn2\tinvn3p\ts_lex\tt_lex";
	if(gs)
		cout<<"\ts-S-score\ts-T-score";
	cout<<endl;
	for (itr = cooc_vec->begin(); 
		 itr != cooc_vec->end(); ++itr) {
			 cout<<*itr;
			 if(gs)
				 cout<<"\t"<<itr->kn_smoothed_source_score(gs)<<
				 "\t"<<itr->kn_smoothed_target_score(gs);
			 cout<<endl;
		 }
}

void print_pcooc_vec(pcooc_vector_t * pcooc_vec,global_score_type * gs=NULL)
{
	unsigned i=0;
	for(pcooc_vector_t::iterator itr=pcooc_vec->begin();itr!=pcooc_vec->end();++itr,++i){
		cout<<"========================================VECTOR "<<i
			<<"========================================\n";	
		print_cooc_vec(&*itr,gs);
	}
	
}

#endif
