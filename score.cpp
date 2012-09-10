/***************************************************************************
*    score.cpp
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
#include <set>
#include <string>
#include "common.h"
#include "crono.h"
//#include "tqueue.h"
#include "strspec.h"
#include "score.h"
#include "merger.h"
#include "aligns.h"



void sort_by_key(pcooc_vector_t * vec,unsigned long mem)
{
//    cout<<"Using memory block: "<<mem/1024/1024<<endl;
    pvectors::sort(vec/*->begin(),vec->end()*/,CmpKey(),mem);
}
//void sort_by_val(pcooc_vector_t* vec,unsigned mem)
//{
//    pvectors::sort(vec/*->begin(),vec->end()*/,CmpVal(),mem);
//
//}
////////functions to combine differnet lexical scores
bool operator >(const lex_pair_t & a,const lex_pair_t & b)
{
    return a.first > b.first;
}
lex_pair_t max_double(const lex_pair_vec_t & vec)
{
    lex_pair_t max_ret=*vec.begin();
    for(lex_pair_vec_t::const_iterator i=vec.begin();i!=vec.end();++i){
        if(*i>max_ret)
            max_ret=*i;
    }
    return max_ret;
}
lex_pair_t average(const lex_pair_vec_t & vec)
{
    double sum=0;
    for(lex_pair_vec_t::const_iterator i=vec.begin();i!=vec.end();++i)
        sum+=i->first;

    return lex_pair_t(sum/vec.size(),vec.begin()->second);
}

struct epsilon_less {
  bool operator() (const lex_pair_t & a, const lex_pair_t & b ) const
  {return b.first-a.first > std::numeric_limits<double>::epsilon();}
};
//bool epsilon_less( double a, double b )
//{
//    return b-a > std::numeric_limits<double>::epsilon();
//}

lex_pair_t max_occur(const lex_pair_vec_t & vec)
{
    typedef map<lex_pair_t,unsigned,epsilon_less> lex_pair_freq_t ;
    lex_pair_freq_t freqs;

    for(lex_pair_vec_t::const_iterator i=vec.begin();i!=vec.end();++i){
        if(!freqs.insert(pair<lex_pair_t,unsigned>(*i,0)).second)
            freqs[*i]+=1;
    }
    unsigned max_freq=0;

    lex_pair_t max_ret=freqs.begin()->first,
            max_ret2=freqs.begin()->first;
    for(lex_pair_freq_t::const_iterator i=freqs.begin();i!=freqs.end();++i){

        if (i->second>max_freq){
            max_freq=i->second;
            max_ret=i->first;
        }
        
        if(i->first>max_ret2)
            max_ret2=i->first;
    }
    //if all items have same frequency, return max
    if(max_freq==1)
        return max_ret2;
    
    return max_ret;
}
/////////////////////////////
double lex_score(char *src,char * tr,char * al,align_info * al_info,LexMap * lmap)
{
    double score=-1;
    str_vec src_vec,
    tr_vec;
    aligns alignments;

// 	cout<<"Lexical score for ('"<<src<<"','"<<tr<<"')"<<endl;

    tokenize_in_vec(src,&src_vec);

    tokenize_in_vec(tr,&tr_vec);

//     cout<<"Alignment: "<<al<<endl;
    get_alignments(al,&alignments);
//     cout<<"Extracted alignments:\n";
    /*for(aligns::iterator i=alignments.begin();i!=alignments.end();++i)
        cout<<i->first<<":"<<i->second	<<endl;
   */
    if (al_info)
        *al_info= align_info(src_vec.size() ,tr_vec.size() ,&alignments);

    ostringstream key;
    short i=0;
    for(str_vec::iterator src_itr=src_vec.begin();
        src_itr !=src_vec.end();
        ++src_itr,++i){


            if(alignments.count(i)==0){
// 				cout<<"the word '"<<*src_itr<<"' was unaligned"<<endl;
				//cout<<(*lmap)[*src_itr]<<"<---from lex map"<<endl;
                key.str(std::string());
                key<<*src_itr<<STR_SEP<<UNALIGNED;
//                 cout<<"Looking for: "<<key.str()<<endl;
                LexMap::iterator found=lmap->find(key.str());//src_itr->append(STR_SEP).append(UNALIGNED));
												  //lex_trans_type(*src_itr,UNALIGNED));
                if(found!=lmap->end()){
//                     cout<<"Map('"<<*src_itr<<"','"<<UNALIGNED<<"')="<<found->second<<endl; 
					if(score<0)
                        score =found->second;
                    else
						score *=found->second;//(*lmap)[lex_trans_type(*src_itr,UNALIGNED)];
                }
					/*
                else
                cout<<"the couple ('"<<*src_itr<<"',NULL) was not found in the lexical map"<<endl;
                                        */
            }
            else{
                double sum=0;
				unsigned npalign=0;
				//cout<<"the word('"<<*src_itr<<"') was aligned at least once\n";
                for(aligns::iterator al_itr=alignments.equal_range(i).first;
                    al_itr!=alignments.equal_range(i).second;
                    ++al_itr){
                        key.str(std::string());
                        key<<*src_itr<<STR_SEP<<tr_vec[al_itr->second];
//                         cout<<"Looking for: "<<key.str()<<endl;
                        LexMap::iterator found=lmap->find(key.str());

                        if(found!=lmap->end()){
//                             cout<<"Map('"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"')="<<found->second<<endl;
                            sum+=found->second;//(*lmap)[lex_trans_type(*src_itr,tr_vec[(*al_itr).second])];
							npalign++;
                        }else{
//                             cout<<"Couldn't find: '"<<*src_itr<<"','"<<tr_vec[al_itr->second]<<"' in the map\n";
                        }
                    } 
                    if(npalign>0){
						if(score<0)
                            score = sum/npalign;
                        else
                        	score *= sum/npalign;
//                     score *= sum/alignments.count(i);
					}
            }
        }
//         cout<<"Score= "<<score<<endl;
        return MAX(score,ZERO_PROB);
}

double lex_score(char *src,char * tr,char * al,LexMap * lmap)
{
    lex_score(src, tr, al,NULL, lmap);
}

bool contains_idem_tuples(cooc_vector_t * vec)
{
    for(cooc_vector_t::iterator itr0=vec->begin();
        itr0!=vec->end() ;++itr0)
        for(cooc_vector_t::iterator itr1=itr0+1;
             itr1!=vec->end() ;++itr1)
            if(*itr0==*itr1)
                return true;
    return false;
}

void  score_one_source(pcooc_vector_t * res_vec,Merger<cooc_type>/*pcooc_vector_t::merge_iterator*/ * merger,aggregation_func func)
{

    std::vector<cooc_type>  tmp_vec;
//    set<unsigned> tids;

    cooc_type::key_t new_src,cur_src;
//    pair<string,string> new_key,cur_key;
    #pragma omp critical(merger_access)
    {

        if(*merger){
            strcpy(new_src,(*merger)->_key);
            //strcpy(cur_src,(*merger)->key());
            break_pair(new_src);//,STR_SEP)='\0';
            strcpy(cur_src,new_src);
            //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
            while(!strcmp(new_src,cur_src)){
               
                tmp_vec.push_back(**merger);
                ++(*merger);
                if(*merger){
                    strcpy(cur_src,(*merger)->_key);
                    break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
                }
                 else
                     break;
            }

        }
    }
    if(tmp_vec.size()==0  )
        return;
    std::vector<cooc_type>::iterator local_itr,
    			             ref_itr;
   
    unsigned long 	socc,tocc,coocs,
        n[3];
    int category;
    string key;

    tocc=0;
    n[0]=n[1]=n[2]=0;
    local_itr=tmp_vec.begin();

    while(local_itr!=tmp_vec.end()){
            ref_itr=local_itr;
            category=-1;
            coocs=0;
            socc=0;
            lex_pair_vec_t tr_lex,
                    src_lex;
//            align_info_vector aligns;
//            max_tr_lex=local_itr->tr_lex_score;
//            max_src_lex=local_itr->src_lex_score;
//
            while(!strcmp(ref_itr->_key,local_itr->_key)){//first==local_itr->first){//for all similar pairs
                    coocs+=local_itr->cooc;
                    socc+=local_itr->socc;
//                    if(tids.insert(local_itr->table_id).second)
                    tocc+=local_itr->tocc;
                    //Merging lexical scores
                    if (func){
                        tr_lex.push_back(lex_pair_t(local_itr->tr_lex_score,local_itr->align));
                        src_lex.push_back(lex_pair_t(local_itr->src_lex_score,local_itr->align));
//                        ref_itr->tr_lex_score=func(ref_itr->tr_lex_score,local_itr->tr_lex_score);
//                        ref_itr->src_lex_score=func(ref_itr->src_lex_score,local_itr->src_lex_score);
                    }
//                    if(local_itr->tr_lex_score!=ref_itr->tr_lex_score)
//                        cout<<"Different target lexical scores for similar pairs: "<<
//                                local_itr->tr_lex_score<<"!="<<ref_itr->tr_lex_score<<endl;
//                    if(local_itr->src_lex_score!=ref_itr->src_lex_score)
//                        cout<<"Different source lexical scores for similar pairs: "<<
//                                local_itr->src_lex_score<<"!="<<ref_itr->src_lex_score<<endl;

                    ++category;
                    ++local_itr;
                    if(local_itr==tmp_vec.end())
                        break;
            }
            //cout<<"Finished run\n";
            ref_itr->cooc=coocs;
            ref_itr->socc=socc;
            if (func){
                lex_pair_t src_p=func(src_lex),
                        tgt_p=func(tr_lex);

                ref_itr->src_lex_score=src_p.first;
                ref_itr->tr_lex_score=tgt_p.first;
                ref_itr->align=(src_p.second != tgt_p.second ? (src_p.first > tgt_p.first ? src_p.second : tgt_p.second) : src_p.second);
            }


            n[category<=2? category:2]++;
    }
   //
    local_itr=tmp_vec.begin();
    while(local_itr!=tmp_vec.end()){
        //update
            local_itr->tocc=tocc;
            local_itr->invn1=n[0];
            local_itr->invn2=n[1];
            local_itr->invn3p=n[2];
            //The source becomes target and vice versa
            /*
            cooc_type::key_t new_pair;
            break_pair(local_itr->key());
            strcpy(new_pair,last_part(local_itr->key()));
            strcpy(last_part(new_pair),first_part(local_itr->key()));
            *(new_pair+strlen(new_pair))=STR_SEP;
             */
            key=string(local_itr->_key);
            flip_around(local_itr->_key,STR_SEP);
            
            /*
            key= local_itr->first.second;
            key.append(1,STR_SEP).append(local_itr->first.first);

             */ //save to the new map
//             #pragma omp critical(print)
//         {
//            cout<<omp_get_thread_num()<<" computed and is writing to the new map\n";
//         }
            res_vec->push_back(omp_get_thread_num(),*local_itr);//cooc_type(key,src_lex_score,tr_lex_score,&al));
            
//             #pragma omp critical(print)
//         {
//            cout<<omp_get_thread_num()<<" successfully wrote to the new map\n";
//         }
//            ref_itr=local_itr;
            do{
                    ++local_itr;
                    if(local_itr==tmp_vec.end())
                        break;
            }while(!strcmp(key.c_str(),local_itr->_key));//skip all similar pairs
//             #pragma omp critical(print)
//         {
//            cout<<omp_get_thread_num()<<" skipped all similar pairs\n";
//         }
    }


}


unsigned long score_one_source(pcooc_vector_t * cooc_vec,unsigned long * big_n1,unsigned long * big_n2,unsigned long * big_n3,unsigned long * big_n4,
                      Merger<cooc_type> * merger,Merger<ngram_t> * ngmerger,pindicator_vector_t * p2k)
{
    std::vector<cooc_type> tmp_vec2;
    set<unsigned> tids;

    //Crono timer;



    cooc_type::key_t new_src,cur_src;
    bool keep=false;
//    pair<string,string> new_key,cur_key;
    #pragma omp critical(merger_access)
    {
        if(*merger){
            strcpy(new_src,(*merger)->_key);
            //strcpy(cur_src,(*merger)->key());
            break_pair(new_src);//,STR_SEP)='\0';
            strcpy(cur_src,new_src);
            //memcpy(cur_src,new_src,strlen((*merger)->key())+1);
            while(!strcmp(new_src,cur_src)){
                tmp_vec2.push_back(**merger);
                ++(*merger);
                if(*merger){
                    strcpy(cur_src,(*merger)->_key);
                    break_pair(cur_src);//cur_key=split_phrase((*merger)->first);
                }else
                     break;
            }

        }
    }
    #pragma omp critical(ngmerger_access)
    {
        if(ngmerger && *ngmerger && !tmp_vec2.empty()){
            while(strcmp(new_src,(*ngmerger)->_key)>0){
                ++(*ngmerger);
            if(!*ngmerger)
                break;
            }
            if(*ngmerger)
                keep=strcmp(new_src,(*ngmerger)->_key)==0;
//                keep=true;
        }       
    }

    if(tmp_vec2.size()==0  )
        return 0;

    std::vector<cooc_type>::iterator local_itr2;
    unsigned long 	socc,//tocc,coocs,
        n[3];

    //string key;
    
    socc=0;
    n[0]=n[1]=n[2]=0;
    for(local_itr2=tmp_vec2.begin();local_itr2!=tmp_vec2.end();++local_itr2){//get all pairs with same source

// 				cout<<"Phrase pair: "<<new_pair<<endl;
//        if(tids.insert(local_itr2->table_id).second)
        socc+=local_itr2->socc;

        n[local_itr2->cooc<=2? local_itr2->cooc-1:2]++;
        switch(local_itr2->cooc){
            case 1: ++(*big_n1);break;
            case 2: ++(*big_n2);break;
            case 3: ++(*big_n3);break;
            case 4: ++(*big_n4);break;
            //		default:more++;
        }
    }
    for(local_itr2=tmp_vec2.begin();local_itr2!=tmp_vec2.end();++local_itr2){
        //                     cout<<"Values to be updated: "<<start_itr->tocc<<"\t"<<start_itr->invn1<<"\t"<<start_itr->invn2<<"\t"<<start_itr->invn3p<<endl;
        local_itr2->socc=socc;
        local_itr2->n1=n[0];
        local_itr2->n2=n[1];
        local_itr2->n3p=n[2];

        cooc_vec->push_back(omp_get_thread_num(),*local_itr2);
        if(p2k)
            p2k->push_back(omp_get_thread_num(),keep);
    }
    if(p2k && keep)
        return tmp_vec2.size();
    else 
        return 0;
}

void score(pcooc_vector_t ** cooc_vec_ptr,/*pcooc_vector_t * res_vec,*/global_score_type * g_scores,unsigned long mem,unsigned lagg,
           const char * testset,const char * ngramsfname,pindicator_vector_t * pairs2keep )
{
    double phase1span,phase2span;
    global_score_type gs;
   
//    cout<<"Scoring started..."<<endl;
    Crono timer;
    timer.start();
    
    pcooc_vector_t * cooc_vec=*cooc_vec_ptr;
    pcooc_vector_t * res_vec=new pcooc_vector_t(cooc_vec->nb_sub_vecs());
    cooc_vec->print_sizes();
	gs.all_occurs=cooc_vec->size();
    cout<<endl;
    cout<<"-------------------------\n"
	      "Sorting (first run)\n"
		  "-------------------------"<<endl;
    sort_by_key (cooc_vec,mem);
    timer.stop(); 

   phase1span=timer.span();
    cout<<"\nDone sorting partial sets\nPartial sorting took "<<timer.formatted_span()<<endl;
    
    cout<<"\n-------------------------\n"
	        "Merging partial sorts\n"
			"-------------------------"<<endl;
    timer.reset();
    timer.start();

	aggregation_func selector=&max_double;
	switch(lagg){
		case OCC_AGG: selector=&max_occur;break;
		case AVG_AGG: selector=&average;break;			
			
	}
    
    Merger<cooc_type> * /*pcooc_vector_t::merge_iterator*/ g_merger=new Merger<cooc_type>(cooc_vec);//->get_merge_iterator();
     
    #pragma omp parallel shared(cooc_vec,res_vec, g_merger,selector)
    {
        g_merger->init(omp_get_thread_num());
        #pragma omp barrier
        
         while(true){

            score_one_source(res_vec,g_merger,selector);
            if(!(*g_merger))
                break;
//            cout<<omp_get_thread_num()<<": "<</*g_merger.count()<<*/"\t";
        }
    }


    timer.stop();

    cout<<"Done merging in: "<<timer.formatted_span()<<endl;

    cout<<//"\n-------------------------------------------------------\n"<<
            "\nFirst run of scoring done \nFirst run, including partial sorting, took "<<
            timer.formatted_span(phase1span)<<endl;

    delete g_merger;
    delete *cooc_vec_ptr;
//    cooc_vec->clear();
	//second sort by target sentences IDs
    *cooc_vec_ptr=new pcooc_vector_t(res_vec->nb_sub_vecs());
    cooc_vec=*cooc_vec_ptr;

    phase1span+=timer.span();
    timer.reset();
    timer.start();
    cout<<"\nNumber of unique pairs: "<<res_vec->size()<<endl;
    res_vec->print_sizes();
    cout<<endl;
    cout<<"---------------------------\nResorting (second run)\n---------------------------"<<endl;
    
    sort_by_key (res_vec,mem);
    
    timer.stop();
    phase2span=timer.span();
    cout<<"\nDone sorting partial sets\nPartial sorting took "<<timer.formatted_span()<<endl;
/*+++++++++++++++++++++++++++Allow subsampling at this stage++++++++++++++++++++++++++++++++++*/    
    pngram_vector_t * ngrams=NULL;
    if(testset != NULL || ngramsfname!= NULL){    
        ngrams= new pngram_vector_t(NB_WORKERS);
		timer.reset();
        timer.start();
        
        if(ngramsfname!= NULL){
            cout<<"\n########################\n"
			        "Loading the n-gram list\n"
					"########################\n"<<endl;
            pload_ngrams(ngramsfname, ngrams);
            cout<<"Done loading n-grams!\n"<<"n-gram list loaded in ";
        }else{
            cout<<"\n########################\n"
			        "Creating the n-gram list\n"
					"########################\n"<<endl;
            pget_ngrams(testset, ngrams);
            cout<<"Done creating n-grams!\n"<<"n-gram list created in ";
        }
        timer.stop();        
        cout<<timer.formatted_span()<<endl;
        timer.reset();
        cout<<"\n------------------------\n"
		        "Sorting the n-gram list\n"
				"------------------------\n"<<endl;        
        timer.start();
        sort_unique_ngrams(ngrams,mem);
        timer.stop();
        cout<<"Sorting n-grams took "<<timer.formatted_span()<<endl;
        
        
        
        if(testset){
            cout<<"\n------------------------\n"
			        "Saving the n-gram list\n"
					"------------------------\n"<<endl;
            timer.reset();
            timer.start();
            psave_ngrams(rsplit(testset,PATH_SEP).second.append(".ngrams").c_str(),ngrams);
            timer.stop();
            cout<<"Saving n-grams took "<<timer.formatted_span()<<endl;
        }
            
        
    }
    
 /*++++++++++++++++++++++++++END of ngram creation/loading+++++++++++++++++++++++++++++++++++++*/   
    if(!ngrams)
        cout<<"\n-------------------------\n"
		        "Merging partial sorts\n"
				"-------------------------"<<endl;
    else
        cout<<"\n---------------------------------------\n"
                "Merging and sub-sampling partial sorts\n"
                "---------------------------------------"<<endl;
    
    timer.reset();
    timer.start();
    Merger<ngram_t> * /*pcooc_vector_t::merge_iterator*/ g_ngmerger=NULL;
    if(ngrams)
        g_ngmerger=new Merger<ngram_t>(ngrams);
    
    g_merger=new Merger<cooc_type>(res_vec);//itr=res_vec->get_merge_iterator(cmp_by_val);
    
	//int more=0;
    /*
    tmp_vec.clear(); 
    unsigned long 	tocc,
                  invn[3];
                      
                  cooc_type::key_type new_trgt;
                  cout<<"Merging the sorted parts\n";
     */
                  unsigned long   big_n1=0,
                  big_n2=0,big_n3=0,big_n4=0,
                  n_matches=0;
    #pragma omp parallel shared(g_merger,g_ngmerger,cooc_vec,res_vec,ngrams,pairs2keep),\
    reduction(+:big_n1),reduction(+:big_n2),reduction(+:big_n3),reduction(+:big_n4), reduction(+:n_matches)
    {
         g_merger->init(omp_get_thread_num());
         if(g_ngmerger)
             g_ngmerger->init(omp_get_thread_num());
        #pragma omp barrier

         while(true){
            n_matches+=score_one_source(cooc_vec,&big_n1,&big_n2,&big_n3,&big_n4,g_merger,g_ngmerger,pairs2keep);//score_one_source(&lock,&lq,res_vec,g_merger);
            if(!(*g_merger))
                break;
//            cout<<omp_get_thread_num()<<": "<</*g_merger.count()<<*/"\t";
        }
    }
                
    timer.stop();
    cout<<"Done merging in: "<<timer.formatted_span()<<endl;

    cout<<"Disposing unneeded resources\n";
    timer.start();
    delete g_merger;
    delete res_vec;
    if(ngrams){
        cout<<"\nNumber of matching pairs to provided ngrams: "<<n_matches<<" ("<<100*double(n_matches)/cooc_vec->size()<<"%)"<<endl;
        delete g_ngmerger;
        delete ngrams;
    }

    gs.big_n1=big_n1;
    gs.big_n2=big_n2;
    gs.big_n3=big_n3;
    gs.big_n4=big_n4;
	//cout<<"Phrases more than 4 times: "<<more<<endl;
    gs.all=cooc_vec->size();
    if(g_scores){
        if(gs.big_n1==0){
            cerr<<"Warning: N1 was found to be zero, taking N1=1!\n";
            gs.big_n1=1;
        }
        if(gs.big_n2==0){
            cerr<<"Warning: N2 was found to be zero, taking N2=1!\n";
            gs.big_n2=1;
        }
        if(gs.big_n3==0){
            cerr<<"Warning: N3 was found to be zero, taking N3=1!\n";
            gs.big_n3=1;
        }
        memcpy(g_scores,&gs,sizeof(global_score_type));
    }

    timer.stop();
    cout<<"\nSecond run of scoring done\nSecond run, including partial sorting, took "<<
            timer.formatted_span(phase2span)<<endl;
    
    cout<<"\nTwo phase phrase scoring took "<<timer.formatted_span(phase1span+phase2span)<<endl<<endl;
	//calc corresponding scores    
}
