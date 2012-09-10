/***************************************************************************
*    out.cpp
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
#include <math.h>
#include <iomanip>
#include <sstream>
#include <vector>
#include "out.h"
#include "strspec.h"

#define PRECISION 20
typedef  std::vector<value_type> local_vector;

inline double t_no_smooth(cooc_type cooc,global_score_type * gs)
{
	return double(cooc.cooc)/cooc.tocc;
}

inline double s_no_smooth(cooc_type cooc,global_score_type * gs)
{
	return double(cooc.cooc)/cooc.socc;
}

inline double t_kn_smooth(cooc_type cooc,global_score_type * gs)
{
	return cooc.kn_smoothed_target_score(gs);
}

inline double s_kn_smooth(cooc_type cooc,global_score_type * gs)
{
	return  cooc.kn_smoothed_source_score(gs);
}

inline double t_wb_smooth(cooc_type cooc,global_score_type * gs)
{
	return cooc.wb_smoothed_target_score();
}

inline double s_wb_smooth(cooc_type cooc,global_score_type * gs)
{
	return  cooc.wb_smoothed_source_score();
}


inline double t_gt_smooth(cooc_type cooc,global_score_type * gs)
{
	return cooc.gt_smoothed_target_score(gs);
}

inline double s_gt_smooth(cooc_type cooc,global_score_type * gs)
{
	return  cooc.gt_smoothed_source_score(gs);
}


inline string format_moses(cooc_type cooc,global_score_type * gs, smoother_t tsmoother,smoother_t ssmoother)
{
    ostringstream ret;
    double penalty=exp(1);
   
           
					 
    cooc_type::key_t key;
    strcpy(key,cooc._key);
    break_pair(key);
    ret<<setprecision(PRECISION)<<scientific<<
    
    first_part(key)<<" ||| "<<last_part(key)<<
                     " ||| "<<cooc.align.source_align()<<"||| "<<cooc.align.target_align()<<
                     "||| "<<
                     tsmoother(cooc,gs)<<" "<<
                     cooc.tr_lex_score<<" "<<
                     ssmoother(cooc,gs)<<" "<<
                     cooc.src_lex_score<<" "<<
                     penalty<<
                     " ||| "<<
                     cooc.cooc<<" "<<cooc.tocc<<" "<<cooc.socc<<" "<<cooc.invn1<<" "<<cooc.invn2<<" "<<cooc.invn3p<<" "<<
                     cooc.n1<<" "<<cooc.n2<<" "<<cooc.n3p
                     <<
                     endl;

    return ret.str();

}

inline string format_moses_restricted(cooc_type cooc,global_score_type * gs, smoother_t tsmoother,smoother_t ssmoother)
{
    ostringstream ret;
    double penalty=exp(1);

    cooc_type::key_t key;
    strcpy(key,cooc._key);
    break_pair(key);
    ret<<setprecision(PRECISION)<<scientific<<
            first_part(key)<<" ||| "<<last_part(key)<<
                     " ||| "<<cooc.align.source_align()<<"||| "<<cooc.align.target_align()<<
                     "||| "<<tsmoother(cooc,gs)<<" "<<cooc.tr_lex_score<<
						" "<<ssmoother(cooc,gs)<<" "<<cooc.src_lex_score<<" "<<
                     penalty
            <<endl;

    return ret.str();

}

inline string format_entry(cooc_type cooc,global_score_type * gs, smoother_t tsmoother,smoother_t ssmoother,unsigned fmt)
{
    switch(fmt){     
        case MOSES:
            return format_moses(cooc, gs,tsmoother,ssmoother);

    }
    
    return format_moses_restricted(cooc, gs,tsmoother,ssmoother);
//     return format_sttk(cooc,gs);
}


unsigned long fill_buf(char * buf,cooc_vector_t * cooc,cooc_vector_t::const_iterator * current,
        global_score_type * gs,  smoother_t tsmoother,smoother_t ssmoother,unsigned fmt,
        indicator_vector_t * spairs,char *  sbuf, indicator_vector_t::const_iterator *cur_ind,unsigned long * sbytes_to_write)
{
//    unsigned long written_bytes=0;
    char * pos_ptr=buf;
    char * spos_ptr=sbuf;
    string entry;
    for(;*current!=cooc->end();++(*current)){ 
         //for(local_vector::const_iterator it=myvec->begin();it!=myvec->end();++it){
        entry=format_entry(**current,gs,tsmoother,ssmoother, fmt);
//        #pragma omp critical(output)
//        {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" thread "<<omp_get_thread_num()<<" ENTRY='"<<entry<<"'"<<endl;
//        }
        if((pos_ptr+entry.length()-buf)>BUF_SIZE)
            break;
        memcpy(pos_ptr,entry.c_str(),entry.length());
        pos_ptr+=entry.length();
        
        if(spairs && (*cur_ind != spairs->end())){
            if(**cur_ind){
                memcpy(spos_ptr,entry.c_str(),entry.length());
                spos_ptr+=entry.length();
            }
            ++(*cur_ind);
        }
    }
    *sbytes_to_write=spos_ptr-sbuf;
    return pos_ptr-buf;
}

/*
void save_to_file(File fh,pcooc_vector_t * cooc,global_score_type * gs,unsigned smoothing, unsigned fmt,string subfn,pindicator_vector_t * spairs)
{
    //TODO: the following three lines should be removed
//    ostringstream msg;
//    msg<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" ends here"<<endl;
//    text_bytes+=msg.str().length();

    //determine my offset
//    unsigned long off;
//    MPI::COMM_WORLD.Scan(&text_bytes,&off,1,MPI::UNSIGNED_LONG,MPI::SUM);
//    off-=text_bytes;
//    if(text_bytes==0)
//        return;

//    int se=posix_fallocate(fh,off,text_bytes);
    //Now output at this offset
//    fseek(fh,off,SEEK_SET);
//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" fallocate:"<<se<<endl;
//    fh.precision(20);
	smoother_t ssmoother,tsmoother;
	CHOOSE_SMOOTHERS(smoothing,ssmoother,tsmoother);
    
	string entry;
    Crono timer;
	File sfh=NULL;
	
    if(!subfn.empty())
        sfh=fopen(subfn.c_str(),"w");
	pindicator_vector_t::iterator cur_indicator;
	if(sfh){
		cur_indicator=spairs->begin();
	}
	indicator_vector_t::const_iterator indicator_itr;

    for(pcooc_vector_t::iterator v=cooc->begin();v!= cooc->end(); ++v){
       
//    cout<<"Saving one vector in STTK format!\n";
        if(sfh){
			indicator_itr=cur_indicator->begin();
		}
        timer.start();  
        for(cooc_vector_t::const_iterator i=v->begin()
                ;i!=v->end()
                ;++i){
            entry=format_entry(*i,gs,tsmoother,ssmoother,fmt);
            fwrite(entry.c_str(),1,entry.length() ,fh); 
			if(sfh && (indicator_itr!= cur_indicator->end())){
				if(*indicator_itr){
					fwrite(entry.c_str(),1,entry.length() ,sfh); 
					
				}
				++indicator_itr;
				
			}
//        fh.Write_at(off,entry.c_str(),entry.length() , MPI::CHAR);
//        off+=entry.length();
        }
        timer.stop();
        cout<<"Vector "<<v-cooc->begin()<<" written in "<<timer.formatted_span()<<endl;
		if(sfh){
			++cur_indicator;
		}
    }

    //just for testing

//   fwrite(msg.str().c_str(),1,msg.str().length() ,fh);
//    fh.Write_at(off,msg.str().c_str(),msg.str().length(),MPI::CHAR);

}*/

// void save_to_stream(ostream & o_file,pcooc_vector_t * cooc,global_score_type * gs, unsigned smoothing, unsigned fmt,string subfn,pindicator_vector_t * spairs)
// {
//     //TODO: the following three lines should be removed
// //    ostringstream msg;
// //    msg<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" ends here"<<endl;
// //    text_bytes+=msg.str().length();
// 
//     //determine my offset
// //    unsigned long off;
// //    MPI::COMM_WORLD.Scan(&text_bytes,&off,1,MPI::UNSIGNED_LONG,MPI::SUM);
// //    off-=text_bytes;
// //    if(text_bytes==0)
// //        return;
// 
// //    int se=posix_fallocate(fh,off,text_bytes);
//     //Now output at this offset
// //    fseek(fh,off,SEEK_SET);
// //    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" fallocate:"<<se<<endl;
// //    fh.precision(20);
// //     string entry;
// 	smoother_t ssmoother,tsmoother;
// 	CHOOSE_SMOOTHERS(smoothing,ssmoother,tsmoother);
//     Crono timer;
// 	
// 	File sfh=NULL;
// 	
//     if(!subfn.empty())
//         sfh=fopen(subfn.c_str(),"w");
// 	pindicator_vector_t::iterator cur_indicator;
// 	if(sfh){
// 		cur_indicator=spairs->begin();
// 	}
// 	indicator_vector_t::const_iterator indicator_itr;
// 
// 	string entry;
//     for(pcooc_vector_t::iterator v=cooc->begin();v!= cooc->end(); ++v){
//        
// //    cout<<"Saving one vector in STTK format!\n";
//         if(sfh){
// 			indicator_itr=cur_indicator->begin();
// 		}
//         timer.start();  
//         for(cooc_vector_t::const_iterator i=v->begin()
//                 ;i!=v->end()
//                 ;++i){
// 					entry=format_entry(*i,gs,tsmoother,ssmoother,fmt);
// 				
//                   o_file <<entry;
// 				  
// 				  if(sfh && (indicator_itr!= cur_indicator->end())){
// 					  if(*indicator_itr){
// 						 fwrite(entry.c_str(),1,entry.length() ,sfh); 
// 					}
// 					++indicator_itr;
// 				}
//         }
//         timer.stop();
//         cout<<"Vector "<<v-cooc->begin()<<" written in "<<timer.formatted_span()<<endl;
// 		if(sfh){
// 			++cur_indicator;
// 		}
//     }
// }

void save_to_stream(ostream & o_file, pcooc_vector_t * cooc,global_score_type * gs,unsigned smoothing, unsigned fmt,string subfn,pindicator_vector_t * spairs)
{
    //TODO: the following three lines should be removed
    char * buf;
    char * sbuf;

    unsigned long global_off=0,//local_off=off,
            myoff=0,
            sglobal_off=0,
            smyoff=0;
			
	smoother_t ssmoother,tsmoother;
	CHOOSE_SMOOTHERS(smoothing,ssmoother,tsmoother);
	
    cooc_vector_t::const_iterator  my_it//=cooc->begin()
                                ,my_begin,
                                    my_end
                                ;
    indicator_vector_t::const_iterator smy_it//=cooc->begin()
                                ,smy_begin,
                                    smy_end
                                ;
    
    
	File sfh=NULL;
	if(!subfn.empty())
        sfh=fopen(subfn.c_str(),"w");
   indicator_vector_t * smyind;
   
//     if(!subfn.empty())
//         sfh=open(subfn.c_str(),O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    
//    local_vector myvec;
//    unsigned long myquota;
    unsigned long bytes_to_write,
            sbytes_to_write;

//    omp_lock_t lock;
//    omp_init_lock(&lock);
    Crono mywrite;
//
    #pragma omp parallel shared(cooc,o_file,sfh,gs,fmt,tsmoother,ssmoother),\
                         private(my_it,buf,bytes_to_write,myoff,my_begin,my_end,\
                         smyind,smy_it,sbuf,sbytes_to_write,smy_begin,smy_end,mywrite)
    {
        buf=new char[BUF_SIZE];
        
        smyind=NULL;
        my_begin=cooc->begin_of(omp_get_thread_num());
        my_end=cooc->end_of(omp_get_thread_num());
                
        my_it=my_begin;
        if(sfh){
            sbuf=new char[BUF_SIZE];
            smy_begin=spairs->begin_of(omp_get_thread_num());
            smy_end=spairs->end_of(omp_get_thread_num());
            smyind=&*(spairs->begin()+omp_get_thread_num());
            smy_it=smy_begin; 
        }

        mywrite.start();
        while(my_it!=my_end){
//            get_thread_share(&myvec,cooc, &global_it);
//            if(myvec.size()==0)
//                break;
            bytes_to_write=fill_buf(buf,&*(cooc->begin()+omp_get_thread_num()),
                    &my_it,gs,tsmoother,ssmoother, fmt,smyind,sbuf,&smy_it,&sbytes_to_write);
//            myvec.clear();
			buf[bytes_to_write]='\0';
            #pragma omp critical(ex_write)
            {
				o_file<< buf;
			}
            if(sfh){
                #pragma omp critical(ex_sub_write)
                {
					fwrite(sbuf,1,sbytes_to_write,sfh);
				}
            }
        }        
        delete [] buf;
        if(sfh)
            delete [] sbuf;
        
        mywrite.stop();
         #pragma omp critical(output)
        {
            cout<<"Thread "<<omp_get_thread_num()<<" writing took "<<mywrite.formatted_span()<<endl;
        }
    }
}




void save_to_file(File fh, pcooc_vector_t * cooc,global_score_type * gs,unsigned smoothing, unsigned fmt,string subfn,pindicator_vector_t * spairs)
{
    //TODO: the following three lines should be removed
    char * buf;
    char * sbuf;

    unsigned long global_off=0,//local_off=off,
            myoff=0,
            sglobal_off=0,
            smyoff=0;
			
	smoother_t ssmoother,tsmoother;
	CHOOSE_SMOOTHERS(smoothing,ssmoother,tsmoother);
	
    cooc_vector_t::const_iterator  my_it//=cooc->begin()
                                ,my_begin,
                                    my_end
                                ;
    indicator_vector_t::const_iterator smy_it//=cooc->begin()
                                ,smy_begin,
                                    smy_end
                                ;
    
    
	File sfh=NULL;
	if(!subfn.empty())
        sfh=fopen(subfn.c_str(),"w");
   indicator_vector_t * smyind;
   
//     if(!subfn.empty())
//         sfh=open(subfn.c_str(),O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    
//    local_vector myvec;
//    unsigned long myquota;
    unsigned long bytes_to_write,
            sbytes_to_write;

//    omp_lock_t lock;
//    omp_init_lock(&lock);
    Crono mywrite;
//
    #pragma omp parallel shared(cooc,fh,sfh,gs,fmt,tsmoother,ssmoother),\
                         private(my_it,buf,bytes_to_write,myoff,my_begin,my_end,\
                         smyind,smy_it,sbuf,sbytes_to_write,smy_begin,smy_end,mywrite)
    {
        buf=new char[BUF_SIZE];
        
        smyind=NULL;
        my_begin=cooc->begin_of(omp_get_thread_num());
        my_end=cooc->end_of(omp_get_thread_num());
                
        my_it=my_begin;
        if(sfh){
            sbuf=new char[BUF_SIZE];
            smy_begin=spairs->begin_of(omp_get_thread_num());
            smy_end=spairs->end_of(omp_get_thread_num());
            smyind=&*(spairs->begin()+omp_get_thread_num());
            smy_it=smy_begin; 
        }

        mywrite.start();
        while(my_it!=my_end){
//            get_thread_share(&myvec,cooc, &global_it);
//            if(myvec.size()==0)
//                break;
            bytes_to_write=fill_buf(buf,&*(cooc->begin()+omp_get_thread_num()),
                    &my_it,gs,tsmoother,ssmoother, fmt,smyind,sbuf,&smy_it,&sbytes_to_write);
//            myvec.clear();
            #pragma omp critical(ex_write)
            {
				fwrite(buf,1,bytes_to_write,fh);
			}
            if(sfh){
                #pragma omp critical(ex_sub_write)
                {
					fwrite(sbuf,1,sbytes_to_write,sfh);
				}
            }
        }        
        delete [] buf;
        if(sfh)
            delete [] sbuf;
        
        mywrite.stop();
         #pragma omp critical(output)
        {
            cout<<"Thread "<<omp_get_thread_num()<<" writing took "<<mywrite.formatted_span()<<endl;
        }
    }
}



void psave_to_file(int fh, pcooc_vector_t * cooc,global_score_type * gs,unsigned smoothing, unsigned fmt,string subfn,pindicator_vector_t * spairs)
{
    //TODO: the following three lines should be removed
    char * buf;
    char * sbuf;

//    ostringstream msg;
//    msg<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" ends here"<<endl;
//    text_bytes+=msg.str().length();

    //determine my offset
//    unsigned long off;
//    MPI::COMM_WORLD.Scan(&text_bytes,&off,1,MPI::UNSIGNED_LONG,MPI::SUM);
//    off-=text_bytes;
//    if(text_bytes==0)
//        return;
    
//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" writing to disk:"<<text_bytes<<endl;
//    int se=posix_fallocate(fh,off,text_bytes);
    //Now output at this offset
//    fseek(fh,off,SEEK_SET);
//    cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" fallocate:"<<se<<endl;
    unsigned long global_off=0,//local_off=off,
            myoff=0,
            sglobal_off=0,
            smyoff=0;
			
	smoother_t ssmoother,tsmoother;
	CHOOSE_SMOOTHERS(smoothing,ssmoother,tsmoother);
	
    cooc_vector_t::const_iterator  my_it//=cooc->begin()
                                ,my_begin,
                                    my_end
                                ;
    indicator_vector_t::const_iterator smy_it//=cooc->begin()
                                ,smy_begin,
                                    smy_end
                                ;
    
    int sfh=-1;
   indicator_vector_t * smyind;
   
    if(!subfn.empty())
        sfh=open(subfn.c_str(),O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    
//    local_vector myvec;
//    unsigned long myquota;
    unsigned long bytes_to_write,
            sbytes_to_write;

//    omp_lock_t lock;
//    omp_init_lock(&lock);
    Crono mywrite;
//
    #pragma omp parallel shared(global_off,sglobal_off,cooc,fh,sfh,gs,fmt,tsmoother,ssmoother),\
                         private(my_it,buf,bytes_to_write,myoff,my_begin,my_end,\
                         smyind,smy_it,sbuf,sbytes_to_write,smyoff,smy_begin,smy_end,mywrite)
    {
        buf=new char[BUF_SIZE];
        
        smyind=NULL;
        my_begin=cooc->begin_of(omp_get_thread_num());
        my_end=cooc->end_of(omp_get_thread_num());
                
        my_it=my_begin;
        if(sfh>-1){
            sbuf=new char[BUF_SIZE];
            smy_begin=spairs->begin_of(omp_get_thread_num());
            smy_end=spairs->end_of(omp_get_thread_num());
            smyind=&*(spairs->begin()+omp_get_thread_num());
            smy_it=smy_begin; 
        }
//        myquota=cooc->size()/omp_get_num_threads();

//        #pragma omp critical(output)
//        {
//            cout<<"Process "<<MPI::COMM_WORLD.Get_rank()<<" thread "<<omp_get_thread_num()<<" QUOTA="<<myquota<<endl;
//        }
//
//        mybegin=cooc->begin()+omp_get_thread_num()*myquota;
//        myend=mybegin+myquota;
//
//        if(omp_get_thread_num()==omp_get_num_threads()-1)
////            myquota+=cooc->size()%omp_get_num_threads();
//            myend=cooc->end();
//        while(true){
        mywrite.start();
        while(my_it!=my_end){
//            get_thread_share(&myvec,cooc, &global_it);
//            if(myvec.size()==0)
//                break;
            bytes_to_write=fill_buf(buf,&*(cooc->begin()+omp_get_thread_num()),
                    &my_it,gs,tsmoother,ssmoother, fmt,smyind,sbuf,&smy_it,&sbytes_to_write);
//            myvec.clear();
            #pragma omp critical(offset_update)
            {
                myoff=global_off;
                global_off+=bytes_to_write;
            }
            pwrite(fh,buf,bytes_to_write,myoff);            
            if(sfh>-1){
                #pragma omp critical(soffset_update)
                {
                    smyoff=sglobal_off;
                    sglobal_off+=sbytes_to_write;
                }
                pwrite(sfh,sbuf,sbytes_to_write,smyoff); 
            }
        }        
        delete [] buf;
        if(sfh>-1)
            delete [] sbuf;
        
        mywrite.stop();
         #pragma omp critical(output)
        {
            cout<<"Thread "<<omp_get_thread_num()<<" writing took "<<mywrite.formatted_span()<<endl;
        }
    }


}