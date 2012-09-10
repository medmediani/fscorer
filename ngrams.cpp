/***************************************************************************
*    ngrams.cpp
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
#include "ngrams.h"
#include "read_file_th.h"
#include "merger.h"
#include "crono.h"

bool operator < (const ngram_t & a,const  ngram_t & b)
{
    return strcmp(a._key,b._key)<0;
}

std::ostream & operator << (std::ostream & o, const ngram_t & ng)
{
    cout<<ng._key;
    
    return o;
}

void sort_unique_ngrams(pngram_vector_t * ngl, unsigned long mem)
{
    pvectors::sort(ngl,CmpNgrams(),mem);
    pvectors::uniquify(ngl);    
}

void get_delims(char * sptr,uvec_t * vec,const char * delims=" \n")
{
//    char * sptr=str;
//    cout<<"Getting delimiters for: '"<<sptr<<"'"<<endl;
    
    char * str=sptr;
    char * cp_str=new char[strlen(str)+1];
    char * where_cp=cp_str;
    char * d;
//    unsigned i;
    for(;*str;++str){        
        for(d=const_cast<char*>(delims);*d;++d)
            if(*str==*d)
                break;
        if(!*d)
            break;
    }
    
//    cout<<"Starting at position: '"<<str-sptr<<"'"<<endl;
    
    for(;*str;){
        for(d=const_cast<char*>(delims);*d;++d)
            if(*str==*d)
                break;
        if(!*d)
            *where_cp++=*str++;
        else{ //Copy spaces only once
//            cout<<"Found space at position: '"<<str-sptr<<"' in source and at position. "<<where_cp-cp_str<<" in copy"<<endl;
            *where_cp++=*str++;
//            cout<<"str_cp so far= '"<<cp_str<<"'"<<endl;
            
            vec->push_back(where_cp-cp_str-1);
            for(;*str;++str){
                for(d=const_cast<char*>(delims);*d;++d)
                    if(*str==*d)
                        break;
                if(!*d)
                    break;
            }
        }
        
    }
    
    *where_cp='\0';
    strcpy(sptr,cp_str);
    delete [] cp_str;
}

void print_delims(uvec_t v)
{
    cout<<"Positions of spaces: ";
    for(uvec_t::const_iterator i= v.begin();i!=v.end();++i)
        cout<<*i<<" ";
    cout<<endl;
}
//
//void get_ngrams(const char * fname,ngram_vector_t * ngl,unsigned window)
//{
//    
//            
//    cout<<"Creating ngrams for '"<<fname<<"'"<<endl;
//    
//    FILE * f=fopen(fname,"r");
//    if(!f){
//        cerr<<"Could not open the test set file: '"<<fname<<"'"<<endl;
//        exit(1);
//    }
//        
//    
//    char line[MAXL];
//    char * got=fgets(line,MAXL,f);
//    uvec_t spos(MAXW);
////    spos.reserve(MAXW);
//    unsigned a;
//    ngram_t::key_t dest;
//    
//    while(got){
////        cout<<"Line= '"<<line<<"'"<<endl;
//        
//        spos.clear();
//        get_delims(line,&spos);
//        if(spos.empty()){
////            cout<<"I got an empty line"<<endl;
//            got=fgets(line,MAXL,f);
//            continue;
//        }
////        exit(0);
////        print_delims(spos);
////        spos.push_back(strlen(line));
//        a=0;
////        cout<<"Ngrams for line: '"<<line<<"'"<<endl;
//        for(uvec_t::const_iterator start=spos.begin();start!=spos.end();a=*start+1,++start)
//            for(unsigned span=0;span<=window && (start+span)!=spos.end();++span){
////                if(*(start+span)<=a)
////                    continue;
//                memset(dest,0,sizeof dest);
////                cout<<"Copying "<<*(start+span)-a-1<<" chars from line (a="<<a<<",span="<<span<<",NEXT POS="<<*(start+span)<<endl;
//                ngl->push_back(ngram_t(strncpy(dest,line+a,*(start+span)-a)));
////                pair<ngram_list_t::iterator,bool> ins=ngl->insert(
////                                                        make_pair(
////                                                                ngram_t(
////                                                                        strncpy(dest,line+a,*(start+span)-a)
////                                                                        ),
////                                                                (unsigned long)1)
////                                                        );
//////                cout<<"'"<<dest<<"'\t";
////                if(!ins.second)
////                    ++(ins.first->second);
//            }
////        exit(0);
////        cout<<endl;
//        got=fgets(line,MAXL,f);
//    }
//}
//
//void load_ngrams(const char * fname,ngram_list_t * ngl)
//{
//    
//    ifstream fin(fname);
//   
//    if(!fin){
//        cerr<<"Could not open the ngram list file: '"<<fname<<"'"<<endl;
//        exit(1);        
//    }
// 
//    string line;
//
//    while (getline(fin, line)) {
//        pair<ngram_list_t::iterator,bool> ins=ngl->insert(
//                                                        make_pair(
//                                                                ngram_t(line.c_str()),1
//                                                                )
//                                                        );
////                cout<<"'"<<dest<<"'\t";
//        if(!ins.second)
//            ++(ins.first->second); 
//    }
//}

void insert_line_ngrams(pngram_vector_t * ngl,string strline,unsigned window)
{
    uvec_t spos(MAXW);
//    spos.reserve(MAXW);
    char * line=new char[strline.length()+1];
    unsigned a=0;
    ngram_t::key_t dest;
    
    strcpy(line,strline.c_str());
//    while(got){
//        cout<<"Line= '"<<line<<"'"<<endl;
        
    spos.clear();
    get_delims(line,&spos);
//    #pragma omp critical(output)
//    {
//        cout<<"Thread "<<omp_get_thread_num()<<" adding line: '"<<line<<"'"<<endl;
//        print_delims(spos);
////        cout<<<<endl;
//    }
    
    if(spos.empty())
        return;
 
//    {
////            cout<<"I got an empty line"<<endl;
//            got=fgets(line,MAXL,f);
//            continue;
//        }
//        exit(0);
//        print_delims(spos);
//        spos.push_back(strlen(line));
//    a=0;
//        cout<<"Ngrams for line: '"<<line<<"'"<<endl;
    for(uvec_t::const_iterator start=spos.begin();start!=spos.end();a=*start+1,++start)
        for(unsigned span=0;span<=window && (start+span)!=spos.end();++span){
            //                if(*(start+span)<=a)
            //                    continue;
            memset(dest,0,sizeof dest);
//            strncpy(dest,line+a,*(start+span)-a);
//            #pragma omp critical(output)
//    {
//        cout<<"Thread "<<omp_get_thread_num()<<" the ngram '"<<dest<<"'"<<endl;
////        cout<<<<endl;
//    }
    
            //                cout<<"Copying "<<*(start+span)-a-1<<" chars from line (a="<<a<<",span="<<span<<",NEXT POS="<<*(start+span)<<endl;
            ngl->push_back(omp_get_thread_num(),ngram_t(strncpy(dest,line+a,*(start+span)-a)));
//                pair<ngram_list_t::iterator,bool> ins=ngl->insert(
//                                                        make_pair(
//                                                                ngram_t(
//                                                                        strncpy(dest,line+a,*(start+span)-a)
//                                                                        ),
//                                                                (unsigned long)1)
//                                                        );
////                cout<<"'"<<dest<<"'\t";
//                if(!ins.second)
//                    ++(ins.first->second);
        }
//        exit(0);
//        cout<<endl;
//        got=fgets(line,MAXL,f);
//    } 
//     #pragma omp critical(output)
//    {
//        cout<<"Thread "<<omp_get_thread_num()<<" inserted line: '"<<line<<"'"<<endl;
////        cout<<<<endl;
//    }
    delete [] line;
}

void pget_ngrams(const char * fname,pngram_vector_t * ngl,unsigned window)
{
    
    cout<<"Creating ngrams for '"<<fname<<"'"<<endl;
    
    shared_args sargs;
    private_args pargs;
    
    sargs.fname=const_cast<char*>(fname);
    
    Crono timer;
    
    #pragma omp parallel  	private(timer,/*ins,first,src_lex_score,tr_lex_score,instr,second,third,*/pargs)\
                                shared(ngl,window,sargs)
    {
        timer.start();
        try{
            insert_line_ngrams(ngl,get_lines(&sargs,&pargs),window);
            while(true){
//                    #pragma omp critical(output)
//                    {
//                        //                    cout<<"Thread "<<omp_get_thread_num()<<" adding line: "<<i++<<": '"<<curline<<"'"<<endl;
//                        out<<curline<<endl;
//                    }
                insert_line_ngrams(ngl,get_lines(NULL,&pargs),window);
            }
        }catch(EmptyStreamException & e){
            timer.stop();
            #pragma omp critical(output)
            {
                cout<<"Thread "<<omp_get_thread_num()<<" done in "<<timer.formatted_span()<<endl;
            }
        }
            
        
        
    }
            
            

}

void pload_ngrams(const char * fname,pngram_vector_t * ngl)
{
    
    cout<<"Loading ngrams from '"<<fname<<"'"<<endl;
    
    shared_args sargs;
    private_args pargs;
    
    sargs.fname=const_cast<char*>(fname);
    
    Crono timer;
    
    #pragma omp parallel  	private(timer,/*ins,first,src_lex_score,tr_lex_score,instr,second,third,*/pargs)\
                                shared(ngl,sargs)
    {
        timer.start();
        try{
            ngl->push_back(omp_get_thread_num(),ngram_t(get_lines(&sargs,&pargs).c_str()));
            while(true){
//                    #pragma omp critical(output)
//                    {
//                        //                    cout<<"Thread "<<omp_get_thread_num()<<" adding line: "<<i++<<": '"<<curline<<"'"<<endl;
//                        out<<curline<<endl;
//                    }
                ngl->push_back(omp_get_thread_num(),ngram_t(get_lines(NULL,&pargs).c_str()));
                
            }
        }catch(EmptyStreamException & e){
            timer.stop();
            #pragma omp critical(output)
            {
                cout<<"Thread "<<omp_get_thread_num()<<" done in "<<timer.formatted_span()<<endl;
            }
        }
    }            
       
}

unsigned long fill_ngram_buf(char * buf,Merger<ngram_t> * merger)
{
//    unsigned long written_bytes=0;
    char * pos_ptr=buf;
    
//    string entry;
    ngram_t::key_t nk;
    
    #pragma omp critical(merger_access)
    {
        if(*merger){
//            if(pos_ptr+strlen((*merger)->_key)>BUF_SIZE)
//                break;
            do{
                memcpy(pos_ptr,(*merger)->_key,strlen((*merger)->_key));
                pos_ptr+=strlen((*merger)->_key);
                *pos_ptr='\n';
                pos_ptr++;
                strcpy(nk,(*merger)->_key);
                do{
                    ++(*merger);
                    
                }while(*merger && !strcmp(nk,(*merger)->_key));          
            }while(*merger && (pos_ptr+strlen((*merger)->_key)+1-buf)<=BUF_SIZE);
        }
    }    
    return pos_ptr-buf;
    
}

void psave_ngrams(const char * fname,pngram_vector_t * ngl)
{ 
    cout<<"Saving the ngrams list into '"<<fname<<"'"<<endl;
    char * buf;
    unsigned long global_off=0,//local_off=off,
            myoff=0;
//    pngram_vector_t::const_iterator  my_it//=cooc->begin()
//                                ,my_begin,
//                                    my_end
//                                ;
    Merger<ngram_t> *  merger=new Merger<ngram_t>(ngl);
    int fh=open(fname,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    unsigned long bytes_to_write;
    Crono mywrite;
    
    #pragma omp parallel shared(global_off,ngl,fh,merger),\
                         private(buf,bytes_to_write,myoff,mywrite)
    {
        merger->init(omp_get_thread_num());
        #pragma omp barrier

        buf=new char[BUF_SIZE];
        mywrite.start();
        
        do{
                        
            bytes_to_write=fill_ngram_buf(buf,merger);
            if(!bytes_to_write)
                break;
            
            #pragma omp critical(offset_update)
            {                
                myoff=global_off;                
                global_off+=bytes_to_write;
            }
            pwrite(fh,buf,bytes_to_write,myoff); 
//            if(!(*merger))
//                break;              
    }while(*merger);
    delete [] buf;
    mywrite.stop();
    #pragma omp critical(output)
    {
        cout<<"Thread "<<omp_get_thread_num()<<" writing ngrams took "<<mywrite.formatted_span()<<endl;
    }
  } 
  close(fh);
            
}