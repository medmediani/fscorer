/***************************************************************************
*    load.cpp
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
#include "load.h"
#include "score.h"
#include "read_file_th.h"
#include "crono.h"

inline string join_pair(string first, string second)
{
    return first.append(1,STR_SEP).append(second);

}
void create_lex_map(File lex_file,LexMap * lmap)
{

	char line[MAXL];
	char *whitespace =(char *) " \t\n\f\r\0";
	char * s_word, *t_word,*score_str,
			* savepos;

	
	
	while(fgets(line,MAXL,lex_file)!=NULL){
		//cout<<"Line to be treated: "<<line<<endl;
			s_word = strtok_r(line,whitespace,&savepos);
			t_word = strtok_r(NULL,whitespace,&savepos);
			score_str=strtok_r(NULL,whitespace,&savepos);
			lmap->insert(
						 make_pair(
								   //lex_trans_type(s_word,t_word), 
								   //string(s_word).append(1,STR_SEP).append(string(t_word)),
                                                                    join_pair(s_word,t_word),
								   strtod(score_str,NULL)
								   )
						 );
	}
//cout<<"Map created successfully"<<endl;		
}

bool insert_line( pcooc_vector_t * cooc,string tline,LexMap *lsrcmap,LexMap * ltrmap)
{
    char line[MAXL];
    const char *sep= FIELD_SEP;
	
	char * 	first,
			*instr,
			*second,
			*third;
    string key;
    cooc_type c;
    double src_lex_score,tr_lex_score;
    align_info al;
    
    strcpy(line,tline.c_str());
    first=line;
    instr=strstr(line,sep);
    *instr='\0';
    second=instr+strlen(sep);
    instr=strstr(second,sep);
    *instr='\0';
    third=instr+strlen(sep);
    instr=strstr(third,sep);
    if(instr)
        *instr='\0';
	

    key=string(second);
    key.append(1,STR_SEP).append(first);
    if(key.length()+1>MAX_KEY_LENGTH){
        #pragma omp critical(warning)
        {
            cerr<<"Warning: a pair is longer than the maximum string size ("<<key.length()+1<<">"<<MAX_KEY_LENGTH<<"). If this happens very often, please augment the value of 'MAX_KEY_LENGTH' in the Makefile, then recompile"<<endl;
        }
        return false;
    }

    tr_lex_score=lex_score(first,second,third,&al,lsrcmap);
    
    strcpy(line,tline.c_str());
    first=line;
    instr=strstr(line,sep);
    *instr='\0';
    second=instr+strlen(sep);
    instr=strstr(second,sep);
    *instr='\0';
    third=instr+strlen(sep);
    instr=strstr(third,sep);
    if(instr)
        *instr='\0';

    invert_alignment(third);
    
    src_lex_score=lex_score(second,first,third,ltrmap);
    
    if(src_lex_score<=REAL_ZERO || tr_lex_score<= REAL_ZERO){
        #pragma omp critical(warning)
        {
            cerr<<"Warning: Zero lexical scores were found for: '"<<key<<"\t"<<cooc_type(key.c_str(),src_lex_score,tr_lex_score,&al)<<"'"<<endl;
        }
    }
    
    cooc->push_back(omp_get_thread_num(),cooc_type(key.c_str(),src_lex_score,tr_lex_score,&al));

    return true;
					//cout<<" added"<<endl;

}


void create_maps(File f,LexMap *lsrcmap,LexMap * ltrmap,
				 pcooc_vector_t * cooc)
{
	char line[MAXL];
	
	TQueue<string> lq;//Fifo thread-safe queue

	string tline;

        pair<PhraseMap::iterator,bool> ins;
        unsigned long nb_lines;

	
        #pragma omp parallel  	private(ins,/*first,src_lex_score,tr_lex_score,instr,second,third,*/line,tline)\
							shared(cooc,f,lq,lsrcmap,ltrmap),reduction(+:nb_lines)
	{
		//master will provide input lines
		#pragma omp master
		{
			lq.realloc(FACTOR*omp_get_num_threads());
			char * fr=fgets(line,MAXL,f);
			
			while(fr!=NULL){
				try{
					lq.push(line);
                                        fr=fgets(line,MAXL,f);
//					if((fr=fgets(line,MAXL,f))==NULL) break;
				}catch(FullException & e){
                                    insert_line(cooc,line,lsrcmap,ltrmap);
                                    fr=fgets(line,MAXL,f);
				}				
			}
			//cout<<"Master finished... messaging all threads..."<<endl;
			int i=omp_get_num_threads();
			while(true){					
				try{
					lq.push(TASK_DONE);
					if(--i==0) break;
				}catch(FullException & e){
				}
			//cout<<"Master done!"<<endl;
			}
			
		}//END master loop
			//other threads will treat the lines read by the master
		try{
			tline=lq.pop();
		}catch(EmptyException & e){//else
				tline="";
		}
			//main loop
		while(tline!=TASK_DONE){
			if(tline!=""){

                            insert_line(cooc,tline,lsrcmap,ltrmap);
			}
			try{
				tline=lq.pop();
			}catch(EmptyException & e){
				tline="";
			}
		}
			
	}
      
}


void pcreate_maps(const char * fname,LexMap *lsrcmap,LexMap * ltrmap,
				 pcooc_vector_t * cooc)
{
//	char line[MAXL];
    shared_args sargs;
    private_args pargs;
    
    sargs.fname=const_cast<char*>(fname);

        unsigned long nb_lines;

        Crono timer;
        
        #pragma omp parallel  	private(timer,pargs)\
							shared(cooc,sargs,lsrcmap,ltrmap),reduction(+:nb_lines)
	{
            
            
            timer.start();
            try{
                insert_line(cooc,get_lines(&sargs,&pargs),lsrcmap,ltrmap);//tline=get_lines(&sargs,&pargs);
                while(true){
                    insert_line(cooc,get_lines(NULL,&pargs),lsrcmap,ltrmap);//tline=get_lines(NULL,&pargs);
                }
            }catch(EmptyStreamException & e){
                timer.stop();
                #pragma omp critical(output)
                {
                    
                    cout<<"Thread "<<omp_get_thread_num()<<" done loading in "<<timer.formatted_span()<<endl;
                }
            }
			
	}
      
}