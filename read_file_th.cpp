/***************************************************************************
*    read_file_th.cpp
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
#include <unistd.h>
#include <algorithm>
#include <omp.h>
#include <fcntl.h>
#include "read_file_th.h"
#include "common.h"




string get_lines(shared_args * sargs,private_args * pargs)
{
    static shared_args * oldsargs=NULL;
    shared_args * mysargs;
    
    if(!sargs)
        mysargs=oldsargs;
    else{
        oldsargs=sargs;
        mysargs=sargs;
    }
//    if(!mysargs)
//        cout<<"Thread "<<omp_get_thread_num()<<": mysargs is null\n";
    
    
//        throw EmptyStreamException();
//    return string();

    if(sargs)//first time here?
    {
        pargs->fh=open(sargs->fname,O_RDONLY);
        if(pargs->fh<0){
            cerr<<"Error opening: '"<<sargs->fname<<endl;
            exit(-1);
        }
            
        pargs->buf=new char[BUF_SIZE+1];
        pargs->fline="";
        #pragma omp single
        {
//                cout<<"Thread "<<omp_get_thread_num()<<" setting file size"<<endl;
            sargs->fsize=lseek(pargs->fh,0,SEEK_END);
            if(omp_get_num_threads()>1){
                sargs->residues=new string *[omp_get_num_threads()-1];
                memset(sargs->residues,0,sizeof(string *)*(omp_get_num_threads()-1));
            }else
                sargs->residues=NULL;
        }
//            #pragma omp barrier
        pargs->quota=sargs->fsize/omp_get_num_threads();
        //#pragma omp critical(output)
        //    {
        //    cout<<"Thread "<<omp_get_thread_num()<<": myquota: "<<myquota<<" chars\n";
        //    }
        //    
        pargs->off=omp_get_thread_num()*pargs->quota;
        if(omp_get_thread_num()==omp_get_num_threads()-1)
            pargs->quota+=sargs->fsize%omp_get_num_threads();
        pargs->prev="";  
        pargs->strm.setstate(istringstream::eofbit);
    }
    
    if(pargs->buf){
        try_get_newline:
        if(!pargs->strm.eof() ){
            
                    
//     #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" my previous value of prev: '"<<pargs->prev<<"'"<<endl;
//        }
            getline(pargs->strm,pargs->line);
//            #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" I got a new line from buffer: '"<<pargs->line<<"'"<<endl;
//        } 
            if(!pargs->strm.eof()){// we got a new line
                if(omp_get_thread_num()>0 && mysargs->residues[omp_get_thread_num()-1]==NULL){
                            pargs->fline=pargs->line;
                            mysargs->residues[omp_get_thread_num()-1]=&pargs->fline;
                            goto try_get_newline;
                }else{
//                       #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" 1. returning line: '"<<pargs->line<<"'"<<endl;
//        } 
                    return pargs->line;
                }
            }
            if(pargs->buf[pargs->bytesread-1]!='\n')
                pargs->prev=pargs->line;
//                               #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" new prev value: '"<<pargs->prev<<"'"<<endl;
//        } 
        }

        while(pargs->quota>0){ //read new buffer
            pargs->bytesread=pread(pargs->fh,pargs->buf,min(BUF_SIZE,pargs->quota),pargs->off);
            pargs->quota-=pargs->bytesread;
            if(pargs->bytesread<=0){
                 #pragma omp critical(output)
                {
                cout<<"Thread "<<omp_get_thread_num()<<"Nothing to read: "<<pargs->bytesread<<endl;
                }
                break;//throw EmptyStreamException();
            }

            pargs->off+=pargs->bytesread;
            pargs->buf[pargs->bytesread]='\0';
            pargs->strm.clear();
            pargs->strm.str(pargs->buf);

//            #pragma omp critical(output)
//            {  
//        cout<<"Thread "<<omp_get_thread_num()<<" new Buffer: '"<<pargs->buf<<"'"<<endl;
//              }
            getline(pargs->strm,pargs->line);
    //        unsigned scannedbytes=0;
//    #pragma omp critical(output)
//            {  
//        cout<<"Thread "<<omp_get_thread_num()<<" I got from buf: '"<<pargs->line<<"' prev='"<<pargs->prev<<"'"<<endl;
//              }
            
            pargs->line=pargs->prev+pargs->line;
            pargs->prev="";

            if(!pargs->strm.eof() ){
                if(omp_get_thread_num()>0 && mysargs->residues[omp_get_thread_num()-1]==NULL){
                    pargs->fline=pargs->line;
                    mysargs->residues[omp_get_thread_num()-1]=&pargs->fline;
                    goto try_get_newline;
                }else{
//                                      #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" 2. returning line: '"<<pargs->line<<"'"<<endl;
//        } 
                    return pargs->line;
                }
            }

            if(pargs->buf[pargs->bytesread-1]!='\n')
                pargs->prev=pargs->line;

        }
        
//      
//#pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" freeing my buf"<<endl;
//        }
        delete [] pargs->buf;
        pargs->buf=NULL;
        close(pargs->fh);
      
        if(omp_get_thread_num()<(omp_get_num_threads()-1)){
//                    
//     #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" my previous before adding '"<<pargs->prev<<"'"<<endl;
//        }
            while(mysargs->residues[omp_get_thread_num()]==NULL);
            pargs->prev+=*mysargs->residues[omp_get_thread_num()];
               
//     #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" my previous after adding '"<<pargs->prev<<"'"<<endl;
//        }
            if(omp_get_thread_num()>0 && mysargs->residues[omp_get_thread_num()-1]==NULL){
                pargs->fline=pargs->prev;
                mysargs->residues[omp_get_thread_num()-1]=&pargs->fline;
            }else{
//            #pragma omp critical(output)
//             {
//                cout<<"Thread "<<omp_get_thread_num()<<" last line: '"<<pargs->prev<<"'"<<endl;
//             }
                return pargs->prev;//out<<pargs->prev<<endl;
            }
        }
    }
     
    time_t now=time(NULL);    
    
//    #pragma omp critical(output)
//    {
//        cout<<"Thread "<<omp_get_thread_num()<<" started waiting at the barrier on: "<<ctime(&now)<<endl;
//    }
    
    #pragma omp barrier
    now=time(NULL); 
//    #pragma omp critical(output)
//    {
//        cout<<"Thread "<<omp_get_thread_num()<<" crossed the barrier on: "<<ctime(&now)<<endl;
//    }
    
    #pragma omp single
    { 
          //finally
        oldsargs=NULL;
        
        delete [] mysargs->residues;
    }
        
//     #pragma omp critical(output)
//        {
//            cout<<"Thread "<<omp_get_thread_num()<<" throwing the exception"<<endl;
//        }
    throw EmptyStreamException();    
 
}
//
//int main(int argc, char* argv[])
//{
//    shared_args sargs;
//    private_args pargs;
//    
//    
//    char fname[]="README";
//    sargs.fname=fname;
//    
////    int fh,bytesread;
////    unsigned long off;
////    unsigned long fsize;
////    unsigned long myquota;
////    string prev,line;
////    istringstream strm;
////    string ** residues;
//    ofstream out;
//    out.open("reproduced");
////    string  fline;
//    string curline;
//    unsigned i=0;
//    #pragma omp parallel shared(sargs,out,i), private(pargs,curline)
//    {
//        try{
//            curline=get_lines(&sargs,&pargs);
//            
//            while(true){
//                #pragma omp critical(output)
//                {
////                    cout<<"Thread "<<omp_get_thread_num()<<" adding line: "<<i++<<": '"<<curline<<"'"<<endl;
//                    out<<curline<<endl;
//                }
//                curline=get_lines(NULL,&pargs);
//            }
//        }catch(EmptyStreamException & e){
//             #pragma omp critical(output)
//            {
//                    cout<<"Thread "<<omp_get_thread_num()<<" done"<<endl;
//            }
//        }
//    }
//    
//    return 0;
//}
