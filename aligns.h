/***************************************************************************
*    aligns.h
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

#include <map>
#include <string.h>

#ifndef _ALIGNS_H
#define _ALIGNS_H
/*
#ifdef USE_GZIP
 #undef USE_GZIP
#endif*/

using namespace std;
typedef multimap<unsigned short, unsigned short> aligns;
struct align_info
{
    typedef long long alignment_t ;
	//private:
		alignment_t alignment;
		unsigned	nb_s_words,
    				nb_t_words,
	  			nb_points;
	//public:
		 align_info(unsigned =0 ,unsigned =0 ,aligns * =NULL );
		 string align2str();
		 string source_align();
		 string target_align();
                 align_info & operator=(const align_info& a) {
                     if (this != &a) {
                         memcpy(this,&a,sizeof(align_info));
                     }
                     return *this;
                 }
                 bool operator ==(const align_info & other)
                 {
                     return alignment==other.alignment && nb_s_words== other.nb_s_words && nb_t_words==other.nb_t_words;
                 }
                 bool operator !=(const align_info & other)
                 {
                     return !(*this==other);
                 }
};

//typedef vector<align_info> align_info_vector;
#endif