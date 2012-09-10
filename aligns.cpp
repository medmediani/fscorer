/***************************************************************************
*    aligns.cpp
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
#include <sstream>
#include <iostream>
#include <string>

#include "aligns.h"

align_info::align_info(unsigned sw,unsigned tw,aligns * align):nb_s_words(sw),nb_t_words(tw),nb_points(0),alignment(0)
{
	 if(!align)
		 return;	 
	 aligns::reverse_iterator itr;
	 for(itr=align->rbegin();itr!=align->rend();){
		 short src=itr->first;
		 while(itr->first==src){
			 alignment=alignment<<4 | itr->second | 8;
			 //cout<<"stored: "<<itr->second <<endl;
			 //cout<<"Alignment so far: "<<hex<<alignment<<endl;
			 ++nb_points;
			 if(++itr==align->rend())
				 break;
		 }
		 alignment=alignment<<4 | src ;
		 //cout<<"source stored: "<<src<<endl;
		 //cout<<"Alignment so far: "<<hex<<alignment<<endl;
		 ++nb_points;
	 }	 	
}
 
string align_info::align2str()
{
	 ostringstream res;
	 short nb=nb_points;
	 align_info::alignment_t alignment_cp=alignment;
	 while(nb--){
		 short src=alignment_cp&0xF;
		 //cout<<"Got src= "<<src<<endl;
		 alignment_cp >>=4;
		 while((alignment_cp & 0xF) >7){
			 res<<src<<"-"<<(alignment_cp & 0xF % 8)<<" ";
			 //cout<<"got target: "<<(alignment_cp & 0xF % 8)<<endl;
			 alignment_cp >>=4;
			 if(--nb==0)
				 break;
		 }
	 }
	 		
	 return res.str();
}
 
string align_info::source_align()
{
	 ostringstream res;
	 short nb=nb_points;
	 align_info::alignment_t alignment_cp=alignment;
	 
	 for(short i=0;i<nb_s_words;++i){
		 res<<"(";
		 if(nb){
			 short src=alignment_cp&0xF;			
			 if(src==i){
				 alignment_cp >>=4;
				 --nb;
				 while((alignment_cp & 0xF) >7){
					 res<<(alignment_cp & 0xF % 8)<<",";
					 //cout<<"got target: "<<(alignment_cp & 0xF % 8)<<endl;
					 alignment_cp >>=4;
					 if(--nb==0)
						 break;
				 }				 
				 res.seekp(-1,ios_base::cur);
			 }
		 }
		 res<<") ";
	 }
	 return res.str();
}
 
string align_info::target_align()
{
	 ostringstream res;
	 aligns revalign;
	 short nb=nb_points;
	 align_info::alignment_t alignment_cp=alignment;
	 while(nb--){
		 short src=alignment_cp&0xF;
		 //cout<<"Got src= "<<src<<endl;
		 alignment_cp >>=4;
		 while((alignment_cp & 0xF) >7){
			 revalign.insert(make_pair(alignment_cp & 0xF % 8,src));
			// cout<<"got target: "<<(alignment_cp & 0xF % 8)<<endl;
			 alignment_cp >>=4;
			 if(--nb==0)
				 break;
		 }
	 }
	 for(short i=0;i<nb_t_words;++i){
		 res<<"(";
		 if(revalign.count(i)){
			 for(aligns::iterator itr=revalign.equal_range(i).first;
					      itr!=revalign.equal_range(i).second;
					      ++itr){
						      res<<itr->second<<",";
					      }	
					      res.seekp(-1,ios_base::cur);	
		 }
		 res<<") ";
	 }
	 return res.str();	 
}
		 

/*
int main()
{
	
	aligns a;
	for(int i=0;i<5;i++)
		a.insert(make_pair(i%8,(i+10)%8));
	a.insert(make_pair(0,3));
	cout<<"alignments:\n";
	for(aligns::iterator itr=a.begin();
		itr!=a.end();
		itr++)
		cout<<itr->first<<":"<<itr->second<<endl;
	align_info al(6,7,&a);
	cout<<"-------------\nAlignments stored\n";
	cout<<"Alignments as str: "<<al.align2str()<<endl;
	cout<<"Source alignments: "<<al.source_align()<<endl;
	cout<<"Target alignments: "<<al.target_align()<<endl;
	return 0;	
}
*/