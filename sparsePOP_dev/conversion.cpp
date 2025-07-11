/* -------------------------------------------------------------
 *
 * This file is a component of SparsePOP
 * Copyright (C) 2007 SparsePOP Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * ------------------------------------------------------------- */

#include <iostream>
#include "conversion.h"


void s3r::redundant_OneBounds(class supsetSet & BasisSupports, class supSet & allSup, class supSet & OneSup){
    int nDim = Polysys.dimVar;
    int mDim = Polysys.numSys;
    int kDim = BasisSupports.supsetArray.size()-mDim;
    class spvec_array spvecs, tarray, spSet, cspv, ei;
    class supSet momentSup, redSup;
    int i, k;
    int ssize, nnz, idx, tsize, tnnz;
    list<int>::iterator it;
    //pick up diagonal parts of all moment matrices
    
    nnz = 0;
    ssize = 0;
    for(i = 0;i < kDim; i++){
        tnnz  =  BasisSupports.supsetArray[i+mDim].nnz();
        nnz  = nnz + tnnz*tnnz;
        tsize =  BasisSupports.supsetArray[i+mDim].size();
        ssize = ssize + tsize*(tsize+1)/2;
    }
    for(i=0;i < nDim;i++){
        // find local mat. having index i.
        for(it=Polysys.posOfubds[i].begin();it!=Polysys.posOfubds[i].end();++it){
            //nnzVec[i]++;
            idx = (*it);
            nnz   = nnz   + BasisSupports.supsetArray[idx-1].nnz();
            nnz   = nnz   + BasisSupports.supsetArray[idx-1].size();
            ssize = ssize + BasisSupports.supsetArray[idx-1].size();
        }
    }
    cspv.alloc(ssize, nnz);
    cspv.dim = nDim;
    nnz = 0;
    ssize = 0;
    //pick up diagonal parts of all moment matrices
    for(i = 0;i < kDim; i++){
        momentSup = BasisSupports.supsetArray[i+mDim];
        initialize_spvecs(momentSup, spvecs);
        minkSumMinusDiagPart(spvecs, tarray);
        pushsups(tarray, cspv);
        nnz = nnz + tarray.vap_size;
        ssize = ssize + tarray.pnz_size;
    }
    
    //pick up diagonal parts of all localizin mat of type ((1-x_i)* u_iu_i^T)
    ei.alloc(1, 1);
    ei.pnz_size = 1;
    ei.vap_size = 1;
    ei.vap[1][0] = 1;
    ei.pnz[0][0] = 0;
    ei.pnz[1][0] = 1;
    for(i=0;i < nDim;i++){
        // make e_i vector with sparse format.
        ei.vap[0][0] = i;
        it = Polysys.posOfubds[i].begin();
        for(;it !=Polysys.posOfubds[i].end();++it){
            idx =(*it);
            momentSup = BasisSupports.supsetArray[idx-1];
            initialize_spvecs(momentSup, spvecs);
            for(k = 0;k <spvecs.vap_size ; k++){
                spvecs.vap[1][k] = spvecs.vap[1][k]  << 1;
            }
            minkovsum(spvecs, ei, tarray);
            pushsups(tarray, cspv);
            nnz = nnz + tarray.vap_size;
            ssize = ssize + tarray.pnz_size;
        }
    }
    cspv.vap_size = nnz;
    cspv.pnz_size = ssize;
    initialize_supset(cspv, redSup);
    redSup.unique();
    list<sup>::iterator b1 = allSup.begin();
    list<sup>::iterator e1 = allSup.supList.end();
    list<sup>::iterator b2 = redSup.begin();
    list<sup>::iterator e2 = redSup.supList.end();
    list<sup>::iterator r1 = OneSup.begin();

    set_difference(b1, e1, b2, e2, inserter(OneSup.supList, r1), comp_sup);
    OneSup.sort();
}

void s3r::redundant_ZeroBounds(class supsetSet & BasisSupports, class supSet & allSup, class supSet & ZeroSup){
    int nDim = Polysys.dimVar;
    int mDim = Polysys.numSys;
    int kDim = BasisSupports.supsetArray.size()-mDim;
    class supSet redSup, momentSup;
    class spvec_array spvecs, cspv, aspv, ei, tarray;
    //vector<int> nnzVec(nDim);
    int i, j, k, ssize, nnz, idx;
    list<sup>::iterator ite;
    list<int>::iterator it;
    
    // counting the number of monomials in supsets.
    ssize = 0;
    nnz = 0;
    for(i = 0;i < kDim; i++){
        nnz   = nnz   + BasisSupports.supsetArray[i+mDim].nnz();
        ssize = ssize + BasisSupports.supsetArray[i+mDim].size();
    }
    for(i=0;i < nDim;i++){
        // find local mat. having index i.
        for(it = Polysys.posOflbds[i].begin();it!=Polysys.posOflbds[i].end();++it){
            idx = (*it);
            nnz   = nnz   + BasisSupports.supsetArray[idx-1].nnz();
            nnz   = nnz   + BasisSupports.supsetArray[idx-1].size();
            ssize = ssize + BasisSupports.supsetArray[idx-1].size();
        }
    }
    cspv.alloc(ssize, nnz);
    cspv.dim = nDim;
    nnz = 0;
    //pick up diagonal parts of all moment matrices
    for(i = 0;i < kDim; i++){
        momentSup = BasisSupports.supsetArray[i+mDim];
        initialize_spvecs(momentSup, spvecs);
        for(j = 0;j <spvecs.vap_size ; j++){
            spvecs.vap[1][j] = spvecs.vap[1][j]  << 1;
        }
        nnz = nnz + spvecs.vap_size;
        pushsups(spvecs, cspv);
    }
    //pick up diagonal parts of all localizin mat of type (x_i u_iu_i^T)
    ei.alloc(1, 1);
    ei.pnz_size = 1;
    ei.vap_size = 1;
    ei.vap[1][0] = 1;
    ei.pnz[0][0] = 0;
    ei.pnz[1][0] = 1;
    for(i=0;i < nDim;i++){
        // make e_i vector with sparse format.
        ei.vap[0][0] = i;
        it = Polysys.posOflbds[i].begin();
        for(;it!= Polysys.posOflbds[i].end();++it){
            idx = (*it);
            momentSup = BasisSupports.supsetArray[idx-1];
            initialize_spvecs(momentSup, spvecs);
            for(k = 0;k <spvecs.vap_size ; k++){
                spvecs.vap[1][k] = spvecs.vap[1][k]  << 1;
            }
            minkovsum(spvecs, ei, tarray);
            pushsups(tarray, cspv);
            nnz = nnz + tarray.vap_size;
        }
    }
    cspv.vap_size = nnz;
    cspv.pnz_size = ssize;
    initialize_supset(cspv, redSup);
    redSup.unique();
    list<sup>::iterator b1 = allSup.begin();
    list<sup>::iterator e1 = allSup.supList.end();
    list<sup>::iterator b2 = redSup.begin();
    list<sup>::iterator e2 = redSup.supList.end();
    list<sup>::iterator r1 = ZeroSup.begin();
/*
	cout << "all " << endl;
    allSup.disp();
	cout << "redSup" << endl;
	redSup.disp();
	cout << "ZeroSup " << endl;
*/  
  set_difference(b1, e1, b2, e2, inserter(ZeroSup.supList, r1), comp_sup);
    ZeroSup.sort();
    //cout << "Size of ZeroSup = " << ZeroSup.size() << endl;
//    ZeroSup.disp();
}



int write_sdp_polyinfo(string outname, class poly_info & polyinfo){
    
    if(polyinfo.typeCone != SDP)
        return false;
    
    //file open
    std::fstream fout( outname.c_str(), ios::out );
    if( fout.fail() )
        return false;
    
    //data out
    fout<<polyinfo.typeCone<<endl;
    fout<<polyinfo.sizeCone<<endl;
    fout<<polyinfo.numMs<<endl;
    fout<<polyinfo.mc[polyinfo.numMs*polyinfo.sizeCone]<<endl;
    fout<<polyinfo.numMs*polyinfo.sizeCone+1<<endl;
    for(int i=0;i<polyinfo.mc[polyinfo.numMs*polyinfo.sizeCone];i++){
        fout<<polyinfo.coef[i][0]<<" ";
    }
    fout<<endl;
    for(int i=0;i<polyinfo.mc[polyinfo.numMs*polyinfo.sizeCone];i++){
        fout<<polyinfo.mr[i]<<" ";
    }
    fout<<endl;
    for(int i=0;i<polyinfo.numMs*polyinfo.sizeCone+1;i++){
        fout<<polyinfo.mc[i]<<" ";
    }
    fout<<endl;
    
    if(!write_bassinfo("polysups", polyinfo.sup))
        return false;
    
    //file close
    fout.close();
    
    return true;
}
int write_bassinfo(string outname, class spvec_array & bassinfo){
    
    //file open
    std::fstream fout( outname.c_str(), ios::out );
    if( fout.fail() )
        return false;
    
    //data out
    fout<<bassinfo.dim<<endl;
    fout<<bassinfo.dim2<<endl;
    fout<<bassinfo.pnz_size<<endl;
    fout<<bassinfo.vap_size<<endl;
    if(bassinfo.pnz_size>0){
        for(int i=0;i<bassinfo.pnz_size;i++){
            fout<<bassinfo.pnz[0][i]<<" ";
        }
        fout<<endl;
        for(int i=0;i<bassinfo.pnz_size;i++){
            fout<<bassinfo.pnz[1][i]<<" ";
        }
        fout<<endl;
        if(bassinfo.vap_size>0){
            for(int i=0;i<bassinfo.vap_size;i++){
                fout<<bassinfo.vap[0][i]<<" ";
            }
            fout<<endl;
            for(int i=0;i<bassinfo.vap_size;i++){
                fout<<bassinfo.vap[1][i]<<" ";
            }
            fout<<endl;
        }
    }
    
    //file close
    fout.close();
    
    return true;
    
}

// 2010-01-09 H.Waki
void get_lsdp(class spvec_array & allsups, class mysdp & psdp, vector<int> & linearterms, class spvec_array & xIdxVec){
#ifdef DEBUG
	double t1, t2, t3, t4, t5;
	t1 = (double)clock();
#endif
	// allocate memory to save order of vector
	vector<int> slist(psdp.ele.sup.pnz_size);
	for(int i=0;i<psdp.ele.sup.pnz_size;i++){
		slist[i] = i;
	}
	//vector<int> slist;
	//slist.resize(psdp.ele.sup.pnz_size,0);
#ifdef DEBUG
	t2 = (double)clock();
#endif
	//sort PSDP
	qsort_psdp(slist, psdp);
#ifdef DEBUG
	t3 = (double)clock();
#endif
	// numbering variables
	//in this function, check whether some one order monomials are deleted or not.
	variable_numbering(allsups, slist, psdp, linearterms, xIdxVec);
#ifdef DEBUG
	t4 = (double)clock();
#endif
	//computation of nonnzero elements of upper triangle matrices
	count_upper_nnz(slist, psdp);
	psdp.nBlocks --;
#ifdef DEBUG
	t5 = (double)clock();
	printf("%4.2f sec in get_lsdp\n", (double)(t2-t1)/(double)CLOCKS_PER_SEC);
	printf("%4.2f sec in get_lsdp\n", (double)(t3-t2)/(double)CLOCKS_PER_SEC);
	printf("%4.2f sec in get_lsdp\n", (double)(t4-t3)/(double)CLOCKS_PER_SEC);
	printf("%4.2f sec in get_lsdp\n", (double)(t5-t4)/(double)CLOCKS_PER_SEC);
#endif
}



// 2010-01-09 H.Waki
//void variable_numbering(class spvec_array & allsups,vector<int> plist,class mysdp & psdp,vector<int> & linearterms){
//void variable_numbering(class spvec_array & allsups,vector<int> plist,class mysdp & psdp,vector<int> & linearterms, vector<vector<int> > & xIdxVec){
void variable_numbering(class spvec_array & allsups, vector<int> plist, class mysdp & psdp, vector<int> & linearterms, class spvec_array & xIdxVec){
	//if this value is i+1, index i is a monomial with degree 1.
	//allsups has been already sorted and is unique.
    int as=0;
    int pp=0;
    int novar=0;
    
    if(allsups.pnz[1][0]!=0){
        novar=1;
    }
    int deg1terms = 0;
    int apos, ppos;
    int amax, pmax;
    int idx;
    bool issame = true;
    bool doesuse = false;
    
    while(as < allsups.pnz_size && pp < psdp.ele.sup.pnz_size){
        //compare supports
        if( allsups.pnz[1][as] == psdp.ele.sup.pnz[1][plist[pp]]){
            
            apos = allsups.pnz[0][as];
            amax = allsups.pnz[0][as] + allsups.pnz[1][as];
            ppos = psdp.ele.sup.pnz[0][plist[pp]];
            pmax = psdp.ele.sup.pnz[0][plist[pp]] + psdp.ele.sup.pnz[1][plist[pp]];
            
            issame = true;
            
            while(apos < amax && ppos <pmax){
                //compare index
                if(allsups.vap[0][apos] == psdp.ele.sup.vap[0][ppos]){
                    //compare value
                    if(allsups.vap[1][apos] == psdp.ele.sup.vap[1][ppos]){
                        apos++;
                        ppos++;
                    }
                    else{
                        issame = false;
                        break;
                    }
                }
                else{
                    issame = false;
                    break;
                }
            }
        }
        else{
            issame = false;
        }
        
        //if same...
        if(issame){
            // substitute no. of variables
            psdp.ele.sup.pnz[0][plist[pp]] = novar;
            if(novar == 0){
                //cout << " obj.val. = " << psdp.ele.coef[plist[pp]]<< endl;
                psdp.ele.coef[plist[pp]] *= -1;
            }
            doesuse = true;
            pp++;
        }
        //otherwise
        else{
            if(doesuse == true){
                idx = allsups.pnz[0][as];
                if(allsups.pnz[1][as] == 1 && allsups.vap[1][idx] == 1){
                    linearterms[deg1terms] = allsups.vap[0][idx] + 1;
                    deg1terms++;
                }
                //increase no. of variables
                novar++;
                doesuse = false;
            }
            as++;
        }
    }
    
    if(doesuse == true){
        idx = allsups.pnz[0][as];
        if(allsups.pnz[1][as] == 1 && allsups.vap[1][idx] == 1){
            linearterms[deg1terms] = allsups.vap[0][idx] + 1;
            deg1terms++;
        }
        novar++;
    }
    xIdxVec = allsups;
    psdp.mDim = novar-1;
}

//
// 2010-01-09 H.Waki
//
void genxIdxVec(class spvec_array & allsups, vector<vector<int> > & xIdxVec, int noLinearterms){
    //allocate xIdxVec and substitute powers into xIdxVec
    int k;
    xIdxVec.resize(allsups.pnz_size);
    for(int i=0; i<allsups.pnz_size; i++){
        xIdxVec[i].resize(noLinearterms, 0);
        for(int j=0;j<allsups.pnz[1][i];j++){
            k = allsups.vap[0][allsups.pnz[0][i] + j];
            xIdxVec[i][k] = allsups.vap[1][allsups.pnz[0][i]+j];
        }
    }
}
void count_upper_nnz(vector<int> plist, class mysdp & psdp){
    
    //cout<<" ***> count_upper_nnz ---> "<<endl;
    
    vector<vector<int> > temp(3);
    for(int i=0;i<3;i++){
        temp[i].resize(psdp.ele.sup.pnz_size, 0);
    }
    psdp.utnnz = temp;
    psdp.utsize=0;
    
    int nonobj_size=0;
    for(int i=0;i<psdp.ele.sup.pnz_size;i++){
        if(psdp.ele.bij[0][plist[i]] !=0){
            plist[nonobj_size] = plist[i];
            nonobj_size++;
        }
    }
    psdp.utnnz[0][psdp.utsize] = psdp.ele.sup.pnz[0][plist[0]];
    psdp.utnnz[1][psdp.utsize] = psdp.ele.bij[0][plist[0]];
    psdp.utnnz[2][psdp.utsize] = 1;
    
    for(int i=1;i<nonobj_size;i++){
        if(psdp.ele.sup.pnz[0][plist[i]] == psdp.ele.sup.pnz[0][plist[i-1]]){
            if(psdp.ele.bij[0][plist[i]] == psdp.ele.bij[0][plist[i-1]]){
                psdp.utnnz[2][psdp.utsize] ++ ;
            }
            else {
                psdp.utsize++;
                psdp.utnnz[0][psdp.utsize] = psdp.ele.sup.pnz[0][plist[i]];
                psdp.utnnz[1][psdp.utsize] = psdp.ele.bij[0][plist[i]];
                psdp.utnnz[2][psdp.utsize] = 1;
            }
        }
        else{
            psdp.utsize++;
            psdp.utnnz[0][psdp.utsize] = psdp.ele.sup.pnz[0][plist[i]];
            psdp.utnnz[1][psdp.utsize] = psdp.ele.bij[0][plist[i]];
            psdp.utnnz[2][psdp.utsize] = 1;
        }
    }
    psdp.utsize ++;
}
sup_a_block::sup_a_block(){
    //this->vap0 = NULL;
    //this->vap1 = NULL;
}
sup_a_block::~sup_a_block(){
    this->vap0.clear();
    this->vap1.clear();
}
void sup_a_block::alloc(int mdim){
    this->vap0.resize(mdim, 0);
    this->vap1.resize(mdim, 0);
}
void sup_a_block::input(class mysdp & psdp, int i){
    if(this->vap0.empty()){
        this->vap0.resize(psdp.mDim, 0);
    }
    if(this->vap1.empty()){
        this->vap1.resize(psdp.mDim, 0);
    }
    this->block = psdp.ele.bij[0][i];
    
    this->nnzsize = psdp.ele.sup.pnz[1][i];
    
    this->deg=0;
    for(int k=0;k<psdp.ele.sup.pnz[1][i];k++){
        //		cout<<":s:";
        this->vap0[k] = psdp.ele.sup.vap[0][k+psdp.ele.sup.pnz[0][i]];
        //		cout<<" "<<this->vap0[k]+1;
        //		cout<<" :v:";
        this->vap1[k] = psdp.ele.sup.vap[1][k+psdp.ele.sup.pnz[0][i]];
        //		cout<<" "<<this->vap1[k];
        this->deg += this->vap1[k];
    }    
}
void sup_a_block::input(class spvec_array & supset, int i){
    
    if(this->vap0.empty()){
        this->vap0.resize(supset.dim, 0);
    }
    if(this->vap1.empty()){
        this->vap1.resize(supset.dim, 0);
    }
    
    this->nnzsize = supset.pnz[1][i];
    
    this->deg=0;
    for(int k=0;k<supset.pnz[1][i];k++){
        //		cout<<":s:";
        this->vap0[k] = supset.vap[0][k+supset.pnz[0][i]];
        //		cout<<" "<<this->vap0[k]+1;
        //		cout<<" :v:";
        this->vap1[k] = supset.vap[1][k+supset.pnz[0][i]];
        //		cout<<" "<<this->vap1[k];
        this->deg += this->vap1[k];
    }
    //	cout<<endl;
    
}

void qsort_sups(vector<int> & slist, class spvec_array & spvecs){
    
    class supSet supsets;
    initialize_supset(spvecs, supsets);
    list<class sup>::iterator ite;
    int i = 0;
    for(ite=supsets.begin(); ite!=supsets.supList.end(); ++ite){
        (*ite).no = i;
        i++;
    }
    supsets.supList.sort(comp_sup);
    i=0;
    for(ite=supsets.begin(); ite!=supsets.supList.end(); ++ite){
        slist[i] = (*ite).no;
        i++;
    }
    
}
void simplification(/*IN*/ class spvec_array & vecs){
    if(vecs.pnz_size > 0){
        
        int ssize = vecs.pnz_size;
        vector<int> slist(ssize);
        for(int i=0;i<ssize;i++){
            slist[i] = i;
        }
        //cout << "before sorting" << endl;
        qsort_sups(slist, vecs);
        
        //delete the same elements from the first if exists
        int k=0;
        int s=1;
        int a, b;
        int ma;
        bool flag;
        
        while(s<ssize){
            flag = true;
            if( vecs.pnz[1][slist[k]] == vecs.pnz[1][slist[s]] ){
                a  = vecs.pnz[0][slist[k]];
                ma = a + vecs.pnz[1][slist[k]];
                b = vecs.pnz[0][slist[s]];
                while(a < ma){
                    if( vecs.vap[0][a] != vecs.vap[0][b]){
                        flag = false;
                        break;
                    }else if(vecs.vap[1][a] != vecs.vap[1][b]){
                        flag = false;
                        break;
                    }
                    a++;
                    b++;
                }
            }else{
                flag = false;
            }
            if(flag == false){
                //k-th supports not equal to s^th supports => changes s-th supports into k+1-th.
                k++;
                slist[k] = slist[s];
            }
            s++;
        }
        vector<vector<int> > dumarray(2);
        dumarray[0].clear();
        dumarray[0].resize(vecs.pnz_size, 0);
        dumarray[1].clear();
        dumarray[1].resize(vecs.pnz_size, 0);
        for(int i=0;i<vecs.pnz_size;i++){
            dumarray[0][i] = vecs.pnz[0][i];
            dumarray[1][i] = vecs.pnz[1][i];
        }
        vecs.pnz_size = k+1;
        for(int i=0;i<vecs.pnz_size;i++){
            vecs.pnz[0][i] = dumarray[0][slist[i]];
            vecs.pnz[1][i] = dumarray[1][slist[i]];
        }
        dumarray.clear();
    }
    
}
void get_removelist(/*IN*/ int * alist, class spvec_array & allsups, class spvec_array & removesups, /*OUT*/ int * rlist) {
    
    int rp, ap, ma;
    int numsame;
    
    for(int i=0;i<removesups.pnz_size;i++){
        for(int j=0;j<allsups.pnz_size;i++){
            //check whether supports of allsups included in removesup exist or not.
            if(removesups.pnz[1][i] < allsups.pnz[1][alist[j]]){
                
                rp = removesups.pnz[0][i];
                
                ap = allsups.pnz[0][alist[j]];
                ma = ap + allsups.pnz[1][alist[j]];
                
                numsame = 0;
                
                while(ap < ma){
                    //equal to index
                    if(removesups.vap[0][rp] > allsups.vap[0][ap]){
                        ap ++ ;
                    }
                    else if(removesups.vap[0][rp] < allsups.vap[0][ap]){
                        break;
                    }
                    else{
                        // degree is included
                        if(removesups.vap[1][rp] <= allsups.vap[1][ap]){
                            numsame ++;
                            rp++;
                        }
                        else{
                            break;
                        }
                        ap++;
                    }
                }
                
                // includes!
                if(numsame == removesups.pnz[1][i]){
                    rlist[alist[j]] = false;
                }
            }
        }
        
    }
    
}	
void remove_Binarysups(class mysdp & psdp, vector<int> binvec){
	int sval, bidx, spidx, j, idx;
	int bsize = binvec.size();
	for(bidx=0;bidx<bsize;bidx++){
		for(spidx=0;spidx<psdp.ele.sup.pnz_size;spidx++){
			if(psdp.ele.sup.pnz[0][spidx] >= 0){
				for(j=0; j<psdp.ele.sup.pnz[1][spidx]; j++){
					idx = psdp.ele.sup.pnz[0][spidx]+j;
					sval = psdp.ele.sup.vap[1][idx];
					if(sval > 1 && psdp.ele.sup.vap[0][idx] == binvec[bidx]){
						psdp.ele.sup.vap[1][idx] = 1;
					}
				}
			}
		}
	}
}
void remove_SquareOnesups(class mysdp & psdp, vector<int> Sqvec){
	class supSet supsets,tmpsupSet;
	class sup tmpsup;
	initialize_supset(psdp.ele.sup, supsets);
	psdp.ele.sup.clean();
	list<class sup>::iterator ite;
	vector<int> idx, val;	

	for(ite = supsets.begin(); ite != supsets.end(); ++ite){
		(*ite).getIdxsVals(idx, val);
		for(int i=0; i<idx.size(); i++){
			for(int j=0; j<Sqvec.size(); j++){
				if(idx[i] == Sqvec[j]){
					val[i] = (val[i] % 2);
					break;
				}
			}
			if(val[i] != 0){
				tmpsup.push(idx[i], val[i]);
			}	
		}
		idx.clear();
		val.clear();
		tmpsupSet.pushSup(tmpsup);
		tmpsup.clear();
	}
	//tmpsupSet.unique();
	initialize_spvecs(tmpsupSet, psdp.ele.sup);	
}
void remove_sups(class mysdp & psdp, class spvec_array & removesups){
    //cout<<"***> remove_sups ( for psdp) ---> "<<endl;
    int dum = 0;
    int movesize = 0;
    int firstIdx, maxIdx;
    psdp.ele.sup.pnz_size = 0;
    int maxblock = 0;
    for(int i=0;i<psdp.nBlocks;i++){
        if(maxblock < psdp.block_info[1][i]){
            maxblock = psdp.block_info[1][i];
        }
    }
    vector<bool> remainIdx(maxblock);
    //remainIdx.clear();
    int noblock = 0;
    int nnznoblock = 0;
    vector<int> onpattern ;
    int temp, temp2;
    
    while(noblock < psdp.nBlocks){
        firstIdx = psdp.block_info[0][noblock];
        maxIdx   = firstIdx + psdp.block_info[1][noblock];
        int s, k;
        //1. remove unnecesarry supports
        for(s=0; s<psdp.block_info[1][noblock]; s++){
            remainIdx[s] = true;
        }
        
        //1-1.find positions of support in sups, included in removesups
        int r, a, ma;
        int numsame;
        for(r=0;r<removesups.pnz_size;r++){
            temp=-999;
            temp2 = -1;
            for( s=firstIdx; s<maxIdx; s++){
                if(remainIdx[s - firstIdx] == true && removesups.pnz[1][r] <= psdp.ele.sup.pnz[1][s]){
                    
                    k = removesups.pnz[0][r];
                    a  = psdp.ele.sup.pnz[0][s];
                    ma = a + psdp.ele.sup.pnz[1][s];
                    numsame = 0;
                    while(a < ma){
                        //equal to index
                        if(removesups.vap[0][k] > psdp.ele.sup.vap[0][a]){
                            a ++ ;
                        }else if(removesups.vap[0][k] < psdp.ele.sup.vap[0][a]){
                            break;
                        }else{
                            //removesup.degree is less than or equal to sup.degree
                            if(removesups.vap[1][k] <= psdp.ele.sup.vap[1][a]){
                                numsame ++;
                                k++;
                            }
                            //removesup.degree is more than sup.degree
                            else{
                                break;
                            }
                            a++;
                        }
                    }
                    // Yes it is included
                    if(numsame == removesups.pnz[1][r]){
                        //1-2.remove unnecesarry supports
                        remainIdx[s - firstIdx] = false; dum++;
                    }
                }
            }
        }
        onpattern.clear();
        onpattern.resize(abs(psdp.bLOCKsTruct[noblock])+1, -999);
        temp  = -1;
        temp2 =  1;
        for(s=firstIdx; s<maxIdx; s++){
            if(remainIdx[s-firstIdx] == true){
                if(temp != psdp.ele.bij[1][s] ){
                    temp = psdp.ele.bij[1][s];
                    onpattern[temp] = temp2;
                    temp2++;
                }
            }
        }
        
        //2. Set size of matrices
        if(psdp.bLOCKsTruct[noblock] < 0){
            psdp.bLOCKsTruct[noblock] = 1 - temp2;
        }else{
            psdp.bLOCKsTruct[noblock] = temp2 -1 ;
        }
        
        //3.arrange no. of row and column        
        psdp.block_info[0][noblock] -= movesize;
        temp = psdp.ele.bij[1][firstIdx];
        //2-1.reassign no. of row and col.
        for(s=firstIdx; s<maxIdx; s++){
            if(remainIdx[s-firstIdx] == true){
                psdp.ele.sup.pnz[0][s - movesize] = psdp.ele.sup.pnz[0][s];
                psdp.ele.sup.pnz[1][s - movesize] = psdp.ele.sup.pnz[1][s];
                psdp.ele.bij[0][s - movesize] = nnznoblock;
                psdp.ele.bij[1][s - movesize] = onpattern[psdp.ele.bij[1][s]];
                psdp.ele.bij[2][s - movesize] = onpattern[psdp.ele.bij[2][s]];
                psdp.ele.coef[s - movesize] = psdp.ele.coef[s];
            }else{
                movesize ++ ;
                psdp.block_info[1][noblock]--;
            }
        }
        if(psdp.block_info[1][noblock] !=0){
            psdp.bLOCKsTruct[nnznoblock]   = psdp.bLOCKsTruct[noblock];
            psdp.block_info[0][nnznoblock] = psdp.block_info[0][noblock];
            psdp.block_info[1][nnznoblock] = psdp.block_info[1][noblock];
            psdp.block_info[2][nnznoblock] = psdp.block_info[2][noblock];
            psdp.ele.sup.pnz_size += psdp.block_info[1][noblock];
            nnznoblock++;
        }
        noblock++;
    }
    psdp.nBlocks = nnznoblock;
    //cout<<"<--- remove_sups ( for psdp) <*** "<<endl<<endl;
}
void remove_Binarysups(vector<int> binvec, class spvec_array & allsups){
	int sval, bidx, spidx, j, idx;
	int bsize = binvec.size();
	for(bidx=0;bidx<bsize;bidx++){
		for(spidx=0;spidx<allsups.pnz_size;spidx++){
			if(allsups.pnz[0][spidx] >= 0){
				for(j=0; j<allsups.pnz[1][spidx]; j++){
					idx = allsups.pnz[0][spidx]+j;
					sval = allsups.vap[1][idx];
					if(sval > 1 && allsups.vap[0][idx] == binvec[bidx]){
						allsups.vap[1][idx] = 1;
					}
				}
			}
		}
	}
	/*
	*/
	int ridx = 0;
	for(spidx=0;spidx<allsups.pnz_size;spidx++){
		if(allsups.pnz[0][spidx] >= 0){
			allsups.pnz[0][ridx] = allsups.pnz[0][spidx];
			allsups.pnz[1][ridx] = allsups.pnz[1][spidx];
			ridx ++ ;
		}
	}
	allsups.pnz_size = ridx;
	simplification(allsups);
}
void remove_SquareOnesups(vector<int> Sqvec, class spvec_array & allsups){
	//cout<<" ***> remove_SquareOnesups ---> "<<endl;
	class supSet supsets,tmpsupSet;
	class sup tmpsup;
	initialize_supset(allsups, supsets);
	allsups.clean();
	list<class sup>::iterator ite;
	vector<int> idx, val;	

	for(ite = supsets.begin(); ite != supsets.end(); ++ite){
		(*ite).getIdxsVals(idx, val);
		for(int i=0; i<idx.size(); i++){
			for(int j=0; j<Sqvec.size(); j++){
				if(idx[i] == Sqvec[j]){
					val[i] = (val[i] % 2);
					break;
				}
			}
			if(val[i] != 0){
				tmpsup.push(idx[i], val[i]);
			}	
		}
		idx.clear();
		val.clear();
		tmpsupSet.pushSup(tmpsup);
		tmpsup.clear();
	}
	tmpsupSet.unique();
	initialize_spvecs(tmpsupSet, allsups);	
}
void remove_sups(class spvec_array & removesups, class spvec_array & sups){
    //cout<<" ***> remove_sups( for spvecarray ) ---> "<<endl;
    int sv, mv;
    int rp, rv;
    int sp;
    int numsame=0;
    for(rp=0;rp<removesups.pnz_size;rp++){
        for(sp=0;sp<sups.pnz_size;sp++){
            if(sups.pnz[0][sp] >= 0){
                rv = removesups.pnz[0][rp];
                sv = sups.pnz[0][sp];
                mv = sv + sups.pnz[1][sp];
                numsame = 0;
                while(sv < mv){
                    //equal to index
                    if(removesups.vap[0][rv] > sups.vap[0][sv]){
                        sv ++ ;
                    }
                    else if(removesups.vap[0][rv] < sups.vap[0][sv]){
                        break;
                    }
                    else{
                        //removesup.degree is less than or equal to sup.degee
                        if(removesups.vap[1][rv] <= sups.vap[1][sv]){
                            numsame ++;
                            rv ++;
                        }
                        //removesup.degree is more than sup.degree
                        else{
                            break;
                        }
                        sv ++;
                    }
                }
                //Yes it is included
                if(numsame == removesups.pnz[1][rp]){
                    sups.pnz[0][sp] = -1;
                }
            }
        }
    }
    //arrange supports
    rp = 0;
    for(sp=0;sp<sups.pnz_size;sp++){
        if(sups.pnz[0][sp] >= 0){
            sups.pnz[0][rp] = sups.pnz[0][sp];
            sups.pnz[1][rp] = sups.pnz[1][sp];
            rp ++ ;
        }
    }
    sups.pnz_size = rp;
}

void s3r::set_relaxOrder(int Order){
    //find max degree
    int minOrder=(int)ceil((double)(this->Polysys.maxDeg())/2.0);
    if(Order<minOrder){
        this->param.relax_Order=minOrder;
    }
    else{
        param.relax_Order=Order;
    }
}

void gen_basisindices(
        int sparsesw, double multifactor,
        class polysystem & polysys,
        class cliques & maxcliques,
        vector<list<int> > & BasisIndices) {
    int nDim=polysys.dimvar();
    int rowSize=polysys.numsys();
    
    if(sparsesw==0){
        BasisIndices.resize(rowSize+1);
        for(int i=0; i<rowSize+1;i++){
            for(int j=0; j<nDim; j++){
                BasisIndices[i].push_back(j);
            }
        }
    }else{
        //cpuTime[0] = (double)clock();
        int nClique=maxcliques.numcliques;
        BasisIndices.resize(rowSize+nClique);
        bool flag;
        int maxSizeClique=maxcliques.maxnnz();
        int maxSize, nnz;
        list<int> candidates, varList, nzIndicator, dummyNzIndicator, clique;
        list<int>::iterator lit1, lit2;
        //cpuTime[1] = (double)clock();
        for(int i=1;i<rowSize;i++){
            varList.clear();
            candidates.clear();
            polysys.sumSupports(i, varList);
            //double t1,t2,t3,t4;
            //t1 = (double)clock();
            for(int j=0;j<nClique;j++){
                nnz=0;
                lit1 = varList.begin();
                lit2 = varList.end();
                flag = includes(maxcliques.clique[j].begin(), maxcliques.clique[j].end(), lit1, lit2);
                if(flag){
                    candidates.push_back(j);
                }
            }
            //t2 = (double)clock();
            //cpuTime[3] = cpuTime[3] + (t2-t1);
            //t1 = (double)clock();
			if(candidates.empty()){
				cout << "Error: the construction of max. cliques has something worng." << endl;
				exit(EXIT_FAILURE);
			}
            lit1 = candidates.begin();
            nzIndicator = maxcliques.clique[(*lit1)];
            maxSize=(int)(maxSizeClique*multifactor);
            //expanding nzIndicator until its size does not exceed maxSize
            nnz = nzIndicator.size();
            dummyNzIndicator = nzIndicator;
            if(nnz < maxSize){
                advance(lit1, 1);
                for(;lit1 != candidates.end();++lit1){
                    clique = maxcliques.clique[(*lit1)];
                    dummyNzIndicator.merge(clique);
                    dummyNzIndicator.unique();
                    nnz = dummyNzIndicator.size();
                    if(nnz <= maxSize){
                        nzIndicator = dummyNzIndicator;
                        nnz = nzIndicator.size();
                        if(nnz >= maxSize){
                            break;
                        }
                    }
                    else{
                        dummyNzIndicator = nzIndicator;
                    }
                }
            }
            BasisIndices[i] = nzIndicator;
            //t2 = (double)clock();
            //cpuTime[4] = cpuTime[4] + (t2-t1);
        }
        for(int i=0;i<nClique;i++){
            BasisIndices[i+rowSize] = maxcliques.clique[i];
        }
        //s2 = (double)clock();
        //cpuTime[5] = s2 -s1;
        //cpuTime[2] = (double)clock();
    }
}

void s3r::write_pop(int ell, string fname){
    std::ofstream fout;
    fout.open(fname.c_str(), ios::app);
    if(fout.fail()){
        cout << "error@write_pop:file does not open for output" << endl;
        cout << fname;
        exit(EXIT_FAILURE);
    }
    if(ell == 1){
        fout << "# Scaled and modified POP to be solved" << endl;
    }else if(ell == 0){
        fout << "# POP to be solved" << endl;
    }else{
        fout << "ell should be 0 or 1." << endl;
        exit(EXIT_FAILURE);
    }
    int nDim = this->Polysys.dimVar;
    
    for(int k=0;k < this->Polysys.numSys;k++){
        if(k==0){
            fout << "objPoly" << endl;
        }else{
			if(k==1){
            	fout << "ineqPolySys" << endl;
			}
            fout << "Polynomial ";
			fout.width(2);
			fout << k;
			fout << ":" << endl;
        }
		int typeCone = this->Polysys.polyTypeCone(k);
		fout.setf(ios::showpos);
        fout <<	"typeCone = " << typeCone << " ";
		fout.unsetf(ios::showpos);
		fout.width(1);
        fout << "sizeCone = " << this->Polysys.polySizeCone(k) << " ";
		fout.width(1);
        fout << "dimVar = " << nDim << " ";
		fout.width(1);
        fout << "degree = " << this->Polysys.polyDegree(k) << " ";
		fout.width(1);
        fout << "noTerms = " << this->Polysys.polyNoterms(k) << endl;
        fout << "supportSet = " << endl;
        
        int j;
        list<class mono>::iterator Mono = this->Polysys.polynomial[k].monoList.begin();
        int length = this->Polysys.polynomial[k].monoList.size();
        bool flag;
		int p=0;
        for(;Mono!=Polysys.polynomial[k].monoList.end();++Mono){
		p = p + 1;
		fout.width(3);
		fout << p << ":"; 
            for(int i=0; i<(*Mono).nDim; i++){
                j = 0;
                flag = true;
                while(j < (*Mono).supIdx.size()){
                    if(i == (*Mono).supIdx[j]){
						fout.width(3);
						fout << (*Mono).supVal[j];
                        flag = false;
                        break;
                    }else{
                        j++;
                    }
                }
                if(flag){
					fout.width(3);
					fout << " 0";
                }
            }
            fout << endl;
        }
        Mono = this->Polysys.polynomial[k].monoList.begin();
        fout << "coefficient =" << endl;
        if(Polysys.polynomial[k].sizeCone != 1){
            for(int i =0; i < length; i++){
				fout.width(3);
                fout << i+1 << ":";
                for(int j = 0; j < (*Mono).lengthCoef(); j++){
					fout.precision(2);
					fout.setf(ios::showpos);
					fout.setf(ios::scientific);
                    fout << (*Mono).Coef[j] << " ";
					fout.unsetf(ios::showpos);
					fout.unsetf(ios::scientific);
                }
                fout << endl;
                ++Mono;
            }
        }else{
            for(int i =0; i < length; i++){
				fout.width(3);
                fout << i+1 << ":";
                for(int j = 0; j < (*Mono).lengthCoef(); j++){
					fout.precision(2);
					fout.setf(ios::showpos);
					fout.setf(ios::scientific);
                    fout << (*Mono).Coef[j] << " ";
					fout.unsetf(ios::showpos);
					fout.unsetf(ios::scientific);
                }
                if((i+1) % 10 == 0){
                    fout << endl;
                }
                ++Mono;
            }
        	fout << endl;
        }
    }
    fout << endl;
    fout << "lbd = " << endl;
    for(int i = 0; i < nDim; i++){
		fout.width(3);
        fout << i+1 << ":";
		fout.precision(2);
		fout.setf(ios::showpos);
		fout.setf(ios::scientific);
        if(ell == 1){
            fout << this->Polysys.boundsNew.lbd(i);
        }else if(ell== 0){
            fout << this->Polysys.bounds.lbd(i);
        }
		fout.unsetf(ios::showpos);
		fout.unsetf(ios::scientific);
        //fout << " ";
        if((i+1) % 10 == 0){
            fout << endl;
        }
    }
    fout << endl;
    fout << "ubd = " << endl;
    for(int i = 0; i < nDim; i++){
		fout.width(3);
        fout << i+1 << ":";
		fout.precision(2);
		fout.setf(ios::showpos);
		fout.setf(ios::scientific);
        if(ell == 1){
            fout << this->Polysys.boundsNew.ubd(i);
        }else if(ell== 0){
            fout << this->Polysys.bounds.ubd(i);
        }
		fout.unsetf(ios::showpos);
		fout.unsetf(ios::scientific);
        //fout << " ";
        if((i+1) % 10 == 0){
            fout << endl;
        }
    }
    fout << endl;
    fout << endl;
    fout.close();
}

void s3r::write_BasisSupports(int i, string fname, class supsetSet & bSup){
    std::ofstream fout;
    fout.open(fname.c_str(), ios::app);
    if(fout.fail()){
        cout << "error@write_BasisSupports:file does not open for output" << endl;
        cout << fname;
        exit(EXIT_FAILURE);
    }
    if(i == 1){
        fout << "# basisSupports after reduction" << endl;
    }else if(i == 0){
        fout << "";
    }else{
        fout << "Error MSG" << endl;
        exit(EXIT_FAILURE);
    }
    fout << "# basisSupports --- the support set" << endl;
    fout << "  used for each inequality and equality" << endl;
    
    int mDim;
    class supSet sup;
    mDim = bSup.supsetArray.size();
    for(int i=0;i<mDim;i++){
        sup = bSup.supsetArray[i];
        sup.out_full(i, fname);
    }
    fout << endl;
    fout.close();
}

void s3r::write_BasisIndices(string fname){
    std::ofstream fout;
    fout.open(fname.c_str(), ios::out|ios::app);
    if(fout.fail()){
        cout << "error@write_BasisIndices:file does not open for output" << endl;
        cout << fname;
        exit(EXIT_FAILURE);
    }
    fout << "# basisIndices --- the set of nonzero coordinates" << endl;
    fout << "  used for each equality and inequality" << endl;
    
    int i, j, mDim;
    mDim = (int)(this->bindices.size());
    list<int>::iterator lit;
    for(i=1;i<mDim;i++){
		fout.width(3);
        fout << i << ":";
        lit = bindices[i].begin();
        for(;lit != bindices[i].end();++lit){
            j = (*lit);
			fout.width(3);
            fout << j+1;
        }
        fout << endl;
    }
    fout << endl;
    fout.close();
}

//genBasisSupports
void s3r::genBasisSupports(class supsetSet & BasisSupports){
    int i;
    int rowSize=bindices.size();
    int nDim=this->Polysys.dimvar();
    
    int nVars;
    list<class sup> List;
    class supSet Moment;
    int sosDim;
    BasisSupports.push(Moment);
    for(i=1;i<rowSize;i++){
        nVars=0;
        nVars = bindices[i].size();
        List.clear();
        if(i<this->Polysys.numsys()){
            sosDim=this->param.relax_Order-(int)ceil((double)(this->Polysys.polyDegree(i))/2.0);
            if(this->Polysys.polyTypeCone(i)==EQU){
                //cout<<"Equality constraints "<<i<<" sosDim="<<sosDim<<endl;
                genLexAll(nVars, 2*sosDim, List);
            }
            else{
                //cout<<"Inequality constraints"<<i<<" sosoDim="<<sosDim<<endl;
                genLexAll(nVars, sosDim, List);
            }
        }
        else{
            //cout<<"Moment matrices "<<i<<" sosDim="<<sosDim<<endl;
            genLexAll(nVars, this->param.relax_Order, List);
        }
        Moment.dimVar = this->Polysys.dimVar;
        Moment.setSupSet(nDim, List);
        Moment.changeIndicesAll(bindices[i]);
        BasisSupports.push(Moment);
    }
}

void s3r::eraseBinaryInObj(vector<int> binvec){
	//cout<<" ***> s3r::eraseBinaryInObj ---> "<<endl;
	list<mono>::iterator bite = Polysys.polynomial[0].monoList.begin();
	list<mono>::iterator eite = Polysys.polynomial[0].monoList.end();
	list<mono>::iterator ite;
	class poly objPoly;

	for(ite = bite; ite!= eite; ++ite){
		class sup sup;
		(*ite).getSup(sup);
		class mono newMono;
		newMono.allocSupp(Polysys.polynomial[0].dimVar);
		for (int i=0; i<sup.idx.size();i++){
			for(int j=0; j<binvec.size(); j++){
				if(binvec[j] == sup.idx[i]){
					sup.val[i] = 1;
					break;
				}	
			}
			newMono.setSupp(sup.idx[i], sup.val[i]);
		}
		//cout << "size = " << (*ite).Coef.size() << endl;
		newMono.allocCoef((*ite).Coef.size());
		for(int i=0; i<(*ite).Coef.size(); i++){
			newMono.setCoef((*ite).getCoef(i),i);
		}
		//newMono.writeMono();
		objPoly.addMono(newMono);	
		newMono.clear();
	}
	objPoly.setNoSys(0);
	objPoly.setDimVar(Polysys.polynomial[0].dimVar);
	objPoly.setTypeSize(1,1);
	objPoly.setDegree();
	Polysys.polynomial[0].clear();
	Polysys.polynomial[0] = objPoly;	
	Polysys.layawayObjConst();
}

void s3r::eraseSquareOneInObj(vector<int> Sqvec){
	list<mono>::iterator bite = Polysys.polynomial[0].monoList.begin();
	list<mono>::iterator eite = Polysys.polynomial[0].monoList.end();
	list<mono>::iterator ite;
	class poly objPoly;
	for(ite = bite; ite!= eite; ++ite){
		class sup sup;
		(*ite).getSup(sup);
		class mono newMono;
		newMono.allocSupp(Polysys.polynomial[0].dimVar);
		for (int i=0; i<sup.idx.size();i++){
			for(int j=0; j<Sqvec.size(); j++){
				if(Sqvec[j] == sup.idx[i]){
					sup.val[i] = (sup.val[i] % 2);
					break;
				}	
			}
			newMono.setSupp(sup.idx[i], sup.val[i]);
		}
		newMono.allocCoef((*ite).Coef.size());
		for(int i=0; i<(*ite).Coef.size(); i++){
			newMono.setCoef((*ite).getCoef(i),i);
		}
		objPoly.addMono(newMono);	
		newMono.clear();
	}
	objPoly.setNoSys(0);
	objPoly.setDimVar(Polysys.polynomial[0].dimVar);
	objPoly.setTypeSize(1,1);
	objPoly.setDegree();
	Polysys.polynomial[0].clear();
	Polysys.polynomial[0] = objPoly;	
	Polysys.layawayObjConst();

}
void s3r::eraseCompZeroSups(class supSet & czSups){
	/*NOT IMPLEMENTED */
}
void s3r::eraseBinarySups(vector<int> binvec, vector<class supSet> & BsupArray){
	//cout<<" ***> s3r::eraseBinarySups ---> "<<endl;
	int sizeP=BsupArray.size();
	int bsize=binvec.size();
	int basize;
	bool flag1 = false;
	bool flag2 = false;
	class supSet tmpSupSet;
	class sup tmpsup;
	list<class sup>::iterator baIte;
 	vector<int> baIdx, baVal;
	for(int j=1;j<sizeP;j++){
		tmpSupSet.clear();
		for(baIte = BsupArray[j].begin(); baIte != BsupArray[j].end(); ++ baIte){
			tmpsup.clear();
			baIdx.clear();
			baVal.clear();
			(*baIte).getIdxsVals(baIdx, baVal);
			basize = baIdx.size();
			for(int baidx = 0; baidx < basize; baidx++){
				for(int i=0;i<bsize;i++){
					if(baIdx[baidx] == binvec[i]){
						tmpsup.push(binvec[i], 1);
						flag1 = true;
						flag2 = true;
						break;
					}
				}
				if(flag1 == false){
					tmpsup.push(baIdx[baidx], baVal[baidx]);	
				}else{
					flag1 = false;
				}
			}
			tmpSupSet.pushSup(tmpsup);
		}
		if(flag2 == true){
			tmpSupSet.unique();
			//cout << "before "<< endl;
			//BsupArray[j].disp();
			BsupArray[j].clear();
			BsupArray[j] = tmpSupSet;
			//cout << "after "<< endl;
			//BsupArray[j].disp();
		}
		flag2 = false;
	}
}
void s3r::eraseSquareOneSups(vector<int> Sqvec, vector<class supSet> & BsupArray){
	//cout<<" ***> s3r::eraseSquareOneSups ---> "<<endl;
	int sizeP=BsupArray.size();
	int Sqsize=Sqvec.size();
	int basize, val;
	bool flag1 = false;
	bool flag2 = false;
	class supSet tmpSupSet;
	class sup tmpsup;
	list<class sup>::iterator baIte;
 	vector<int> baIdx, baVal;
	for(int j=1;j<sizeP;j++){
		tmpSupSet.clear();
		for(baIte = BsupArray[j].begin(); baIte != BsupArray[j].end(); ++ baIte){
			tmpsup.clear();
			baIdx.clear();
			baVal.clear();
			(*baIte).getIdxsVals(baIdx, baVal);
			basize = baIdx.size();
			for(int baidx = 0; baidx < basize; baidx++){
				for(int i=0;i<Sqsize;i++){
					if(baIdx[baidx] == Sqvec[i]){
						val = baVal[baidx] % 2;
						//cout << "val = " << val << endl;
						//cout << "idx = " << Sqvec[i] << endl;
						tmpsup.push(Sqvec[i], val);
						flag1 = true;
						flag2 = true;
						break;
					}
				}
				if(flag1 == false){
					tmpsup.push(baIdx[baidx], baVal[baidx]);	
				}else{
					flag1 = false;
				}	
			}
			tmpSupSet.pushSup(tmpsup);
		}
		if(flag2 == true){
			tmpSupSet.unique();
			//cout << "before "<< endl;
			//BsupArray[j].disp();
			BsupArray[j].clear();
			BsupArray[j] = tmpSupSet;
			//cout << "after "<< endl;
			//BsupArray[j].disp();
		}
		flag2 = false;
	}
}
//Delete unnecessarry supports by exploiting complimentarity constraints.
void s3r::eraseCompZeroSups(class supSet & czSups, vector<class supSet> & BaSups) {
    //cout<<" ***> s3r::eraseCompZeroSups ---> "<<endl;
    int sizeP=BaSups.size();
    int sizeCZ=czSups.size();
    list<class sup>::iterator czIte=czSups.begin();
    vector<int> czIdx, czVal;
    
    for(int i=0;i<sizeCZ;i++){
        czIdx.clear();
        czVal.clear();
        (*czIte).getIdxsVals(czIdx, czVal);
        for(int j=1;j<sizeP;j++){
            int sizeBA=BaSups[j].size();
            list<class sup>::iterator baIte=BaSups[j].begin();
            vector<int> baIdx, baVal;
            for(int k=0;k<sizeBA;k++){
                baIdx.clear();
                baVal.clear();
                (*baIte).getIdxsVals(baIdx, baVal);
                int sizeCID=czIdx.size();
                int sizeBID=baIdx.size();
                if(sizeBID >= sizeCID){
                    int h=0;
                    for(int s=0;s<sizeBID && h<sizeCID;s++){
                        if(baIdx[s] == czIdx[h]){
                            if(baVal[s] >= czVal[h]){
                                h++;
                            }
                        }
                    }
                    if(h==sizeCID){
                        //	cout<<"Erased: czSup--> ";(*czIte).disp();
                        //	cout<<"        baSup--> ";(*baIte).disp();
                        baIte=BaSups[j].erase(baIte);
                        sizeBA--;
                        //	cout<<"        from BaSups["<<j<<"]"<<endl;
                    }
                }
                baIte++;
            }
        }
        czIte++;
    }
}

void Div2(class sup & sup1){
    for(vector<int>::iterator it=sup1.val.begin();it!=sup1.val.end();++it){
        (*it) >>= 1;
    }
}

void Multi2(class sup & sup1){
    for(vector<int>::iterator it=sup1.val.begin();it!=sup1.val.end();++it){
        (*it) <<= 1;
    }
}

bool IsNonNegative(class sup & sup1){
    for(vector<int>::iterator it=sup1.val.begin();it!=sup1.val.end();++it){
        if((*it) < 0){
            return false;
        }
    }
    return true;
}
bool IsZero(class sup & sup1){
    for(vector<int>::iterator it=sup1.val.begin();it!=sup1.val.end();++it){
        if((*it) != 0){
            return false;
        }
    }
    return true;
}

void s3r::reduceSupSets(class supsetSet & BasicSupports, class supSet & allNzSups){
	#ifdef DEBUG
		double t1, t2, t3, t4, t5, t6;
		t1 = (double)clock();
	#endif /* #ifdef DEBUG */
            
	//cout<<" ***>s3r::reduceSupSets --->"<<endl;
            
	//Find supports with all even coordinates
	class supSet Fe;
	Fe.setDimVar(Polysys.dimvar());
	allNzSups.getEvenSups(Fe, YES);
	class sup zerosup, Sup, Sup1, Sup2, Sup3;
	Fe.pushSup(zerosup);
	
	// 2011-07-03 H.Waki
	// Implementation for param.errorBdIdx
	//-->
	if(param.aggressiveSW == 1){
		if(param.errorBdIdxVec.empty() == 0){
			//Fe.disp();
			for(int i=0; i<param.errorBdIdxVec.size(); i++){
				class sup sup2;
				sup2.push(param.errorBdIdxVec[i],2);
				Fe.pushSup(sup2);
			}
		}
		//Fe.disp();
	}else{
		int nDim = Polysys.dimvar();
		for(int i=0; i<nDim; i++){
			class sup sup2;
			sup2.push(i,2);
			Fe.pushSup(sup2);
		}
	}
	//<--
	
    #ifdef DEBUG
            t2 = (double)clock();
    #endif /* #ifdef DEBUG */
            //Fe=Fe/2
            for_each(Fe.begin(), Fe.supList.end(), Div2);
    
    #ifdef DEBUG
            t3 = (double)clock();
    #endif /* #ifdef DEBUG */
            int size = Polysys.numsys();
    int ABSsize = BasicSupports.supsetArray.size();
    int Csize = ABSsize-size;
    
    vector<vector<int> > checkList(Csize);
    class supSet bSupSet;
    int bsize, tempsize, glsize;
    
    class supSet2 A(Polysys.dimvar());
    list<class sup>::iterator SupIte, SupRIte, SupWIte;
    
    int oldAsize = 0;
    for(int i=0;i<Csize;i++){
        bSupSet=BasicSupports.supsetArray[i+size];
        SupIte=bSupSet.begin();
        bsize=bSupSet.size();
        checkList[i].resize(bsize, 1);
        oldAsize += bsize;
        for(int j=0;j<bsize;j++){
            if(!Fe.doesExist(*SupIte)){
                A.addSup(i, j, (*SupIte));
            }
            ++SupIte;
        }
    }
    #ifdef DEBUG
            t4 = (double)clock();
    #endif /* #ifdef DEBUG */
            list<class sup2>::iterator SupIte2;
    vector<int> Indices, gl, no;
	#ifdef DEBUG
	int count = 0;
	#endif /* #ifdef DEBUG */
    int newAsize = oldAsize+1;
    vector<int>::iterator it1, it2, vt1, vt2;
    bool gonext, reduce;
    while(oldAsize < newAsize){
        newAsize = oldAsize;
        for(SupIte2=A.begin(); SupIte2 != A.end(); ++SupIte2){
            gl.clear();
            no.clear();
            (*SupIte2).RL(gl, no);
            if(gl.empty()==true){
                cout<<"error@reduceSupSets: gl.size is ZERO"<<endl;
                exit(EXIT_FAILURE);
            }
            gonext = true;
            reduce = false;
            glsize = gl.size();
            for(int i=0;i<glsize;i++){
                SupRIte=BasicSupports.supsetArray[gl[i]+size].begin();
                tempsize = BasicSupports.supsetArray[gl[i]+size].size();
                for(int j=0;j<tempsize;j++){
                    if(checkList[gl[i]][j]!=0 && j!=no[i]){
                        (*SupIte2).getSup(Sup);
                        Multi2(Sup);//2*alpha
                        gonext = SupMinusSup(Sup, (*SupRIte));
                        if(gonext == false){
                            ++SupRIte;
                            continue;
                        }
                        SupWIte  = BasicSupports.supsetArray[gl[i]+size].begin();
                        advance(SupWIte, j);
                        for(int k=j;k<tempsize;k++){
                            if(checkList[gl[i]][k]!=0 && k!=no[i]){
                                //(*SupRIte); //beta
                                //(*SupWIte); //gamma
                                reduce = includes(Sup.idx.begin(), Sup.idx.end(), (*SupWIte).idx.begin(), (*SupWIte).idx.end());
                                if(reduce){
                                    reduce = (Sup == (*SupWIte));
                                }
                                if(reduce){
                                    break;
                                }
                            }
                            ++SupWIte;
                        }
                    }
                    if(reduce){
                        break;
                    }
                    ++SupRIte;
                }
                if(reduce){
                    break;
                }
            }
            if(reduce == false){
                for(int i=0;i<glsize;i++){
                    checkList[gl[i]][no[i]]=0;
                }
            }
        }
        oldAsize = 0;
        for(int i=0;i<Csize;i++){
            for(int j=0;j<checkList[i].size();j++){
                if(checkList[i][j]!=0){
                    oldAsize = oldAsize +1;
                }
            }
        }
        #ifdef DEBUG
                count = count + 1;
        #endif /* #ifdef DEBUG */
    }
    #ifdef DEBUG
            t5 = (double)clock();
    #endif /* #ifdef DEBUG */
            class supSet dumSupSet;
    for(int i=0;i<Csize;i++){
        dumSupSet.clear();
        dumSupSet.setDimVar(Polysys.dimvar());
        SupIte=BasicSupports.supsetArray[i+size].begin();
        for(int j=0;j<checkList[i].size();j++){
            if(checkList[i][j]!=0){
                dumSupSet.addSup(*SupIte);
            }
            ++SupIte;
        }
        BasicSupports.supsetArray[i+size]=dumSupSet;
    }
    #ifdef DEBUG
            t6 = (double)clock();
    printf("%4.2f sec in reduceSupSets.\n", (double)(t2-t1)/(double)CLOCKS_PER_SEC);
    printf("%4.2f sec in reduceSupSets.\n", (double)(t3-t2)/(double)CLOCKS_PER_SEC);
    printf("%4.2f sec in reduceSupSets.\n", (double)(t4-t3)/(double)CLOCKS_PER_SEC);
    printf("%4.2f sec in reduceSupSets.\n", (double)(t5-t4)/(double)CLOCKS_PER_SEC);
    printf("# of iterations of while = %2d\n", count);
    printf("%4.2f sec in reduceSupSets.\n", (double)(t6-t5)/(double)CLOCKS_PER_SEC);
    #endif /* #ifdef DEBUG */
}

void get_subjectto_polys_and_basups(
        /* IN */  class polysystem & polysys, vector<list<int> > BaIndices, vector<class supSet> & basups, int stsize,
        /* OUT */ vector<class poly_info> & polyinfo_st, vector<class bass_info> & bassinfo_st) {
    
    int bdim;
    list<int>::iterator lit;
    for(int i=1;i<stsize+1;i++){
        initialize_polyinfo(polysys, i, polyinfo_st[i-1]);
        bdim = BaIndices[i].size();
        bassinfo_st[i-1].dim = bdim;
        bassinfo_st[i-1].deg = basups[i].deg();
        bassinfo_st[i-1].alloc_pattern(bdim);
        bdim = 0;
        lit = BaIndices[i].begin();
        for(;lit != BaIndices[i].end(); ++lit){
            bassinfo_st[i-1].pattern[bdim] = (*lit);
            bdim++;
        }
        initialize_spvecs(basups[i].supList, bassinfo_st[i-1].sup);
        polyinfo_st[i-1].no = i;
    }
}
void count_lexall_num_a_nnz(/*IN*/int dimvar, int deg, /*OUT*/int & num, int & nnz){
    
    num = 1;
    nnz = 1;
    
    double dum=1;
    
    if(deg != 0){
        
        for(double k=1;k<=(double)deg;k++){
            dum *= (dimvar+k)/k;
        }
        
        int idum = (int)dum;
        if( dum - (double)dum < 1.0){
            idum += 1;
        }
        
        num = idum;
        
        
        dum = 1;
        if(dimvar < deg - 1){
            for(double k=0; k <= (double)dimvar-1; k++){
                dum *= (k+(double)deg)/(k+1.0);
            }
        }
        else{
            for(double k=1; k <=(double)deg-1; k++){
                dum *= (dimvar+k)/k;
            }
        }
        
        idum = (int)dum;
        if( dum - (double)idum < 1.0 ){
            idum += 1;
        }
        
        nnz = dimvar * idum + 1;
        
    }
    else{
        num = 1;
        nnz = 0;
    }
    
    
}
void get_allsups(int dim, class poly_info & polyinfo_obj, int stsize, vector<class poly_info> & polyinfo_st, vector<class bass_info> & bassinfo_st, class spvec_array & allsups){

    allsups.dim = dim;
    if(stsize > 0){
	
	vector<Vec3*> InfoTable;
        for(int i=0; i<stsize; i++){
		Vec3* vec = new Vec3;
		vec->typeCone = polyinfo_st[i].typeCone;
		vec->dim      = bassinfo_st[i].dim;
		vec->deg      = bassinfo_st[i].deg;
		InfoTable.push_back(vec);
        }
	for(int i=0; i<stsize;i++){
		InfoTable[i]->no = i;
	}
	sort(InfoTable.begin(), InfoTable.end(), Vec3::compare);	
        
        vector<int> stand(3);
        vector<int> infolist(stsize);
        for(int i=0;i<stsize;i++){
            //infolist[i] = i;
		Vec3* vec = InfoTable[i];
            infolist[i] = vec->no;
        }
        //sortInfoTable(InfoTable, infolist);
        
        int nzele=0;
        int numele = 0;
        nzele  += polyinfo_obj.sup.pnz_size;
        numele += polyinfo_obj.sup.vap_size;
        
        int k=0;
	Vec3* cVec = InfoTable[0];
        while(k < stsize && cVec->typeCone == EQU){
            nzele += polyinfo_st[infolist[k]].sup.vap_size * bassinfo_st[infolist[k]].sup.pnz_size
                    + bassinfo_st[infolist[k]].sup.vap_size * polyinfo_st[infolist[k]].sup.pnz_size;
            numele += polyinfo_st[infolist[k]].sup.pnz_size * bassinfo_st[infolist[k]].sup.pnz_size;
            k++;
		cVec = InfoTable[k];
        }
        int bpsize, bvsize;
        while(k<stsize){
            count_lexall_num_a_nnz(cVec->dim, 2*cVec->deg, bpsize, bvsize);
            nzele += polyinfo_st[infolist[k]].sup.vap_size * bpsize
                    + bvsize * polyinfo_st[infolist[k]].sup.pnz_size;
            numele += polyinfo_st[infolist[k]].sup.pnz_size * bpsize;
            
            k++;
		if(k == stsize){
			break;
		}
		cVec = InfoTable[k];
        }
        
        allsups.alloc(numele*2, nzele*2);
        pushsups(polyinfo_obj.sup, allsups);
        
        k=0;
        class spvec_array minsups;
		cVec = InfoTable[k];
	while(k < stsize && cVec->typeCone == EQU){
		minkovsum(polyinfo_st[infolist[k]].sup, bassinfo_st[infolist[k]].sup, minsups);
		pushsups(minsups, allsups);
		k++;
		cVec = InfoTable[k];
	}
        
        class spvec_array lexallsups;
        
        bool issame = false;
        vector<int> onpattern(dim);
        while(k<stsize){
            //class spvec_array lexallsups;
            
            if(issame == false){
                genLexAll(cVec->dim, 2*cVec->deg, lexallsups);
                for(int i=0;i<cVec->dim;i++){
                    onpattern[i] = bassinfo_st[infolist[k]].pattern[i];
                }
            }
            else{
                for(int i=0;i<cVec->dim;i++){
                    onpattern[bassinfo_st[infolist[k-1]].pattern[i]] = bassinfo_st[infolist[k]].pattern[i];
                }
            }
            for(int i=0;i<lexallsups.vap_size;i++){
                lexallsups.vap[0][i] = onpattern[lexallsups.vap[0][i]];
            }
            minkovsum(polyinfo_st[infolist[k]].sup, lexallsups, minsups);
            pushsups(minsups, allsups);
            k++;
            if(k<stsize){
		 cVec = InfoTable[k];
		Vec3* dVec = InfoTable[k-1];
                if(cVec->dim == dVec->dim){
                    if(cVec->deg == dVec->deg){
                        issame = true;
                    }else{
                        issame = false;
                        count_lexall_num_a_nnz(cVec->dim, cVec->deg, lexallsups.pnz_size, lexallsups.vap_size);
                    }
                }
                else{
                    issame = false;
                }
            }
        }
	for(int i=0; i<stsize;i++){
		delete InfoTable[i];
	}
        InfoTable.clear();
        lexallsups.del();
        //allsups.disp();
        //simplification(allsups);
    }
    else{
        //case:POP constains no constraints
        //get maximal size and nonzeros of objective function's supports
        int nzele  = polyinfo_obj.sup.pnz_size;
        int numele = polyinfo_obj.sup.vap_size;
        
        //allocate memory
        allsups.alloc(nzele*2, numele*2);
        
        //get all supports of objective function
        pushsups(polyinfo_obj.sup, allsups);
    }
}

void genLexFixDeg(int k, int n, int W, vector<vector<int> > sup, int nnz, class spvec_array & rsups){
    int d;
    for(int i=W;i>0;i--){
        //Sup.push(k,i);
        sup[0][nnz] = k;
        sup[1][nnz] = i;
        nnz ++ ;
        if(W-i>0){
            for(int j=k+1;j<n;j++){
                genLexFixDeg(j, n, W-i, sup, nnz, rsups);
            }
        }
        else{
            rsups.pnz[0][rsups.pnz_size] = rsups.vap_size;
            rsups.pnz[1][rsups.pnz_size] = nnz;
            rsups.pnz_size ++ ;
            d = 0;
            while(d < nnz){
                rsups.vap[0][rsups.vap_size] = sup[0][d];
                rsups.vap[1][rsups.vap_size] = sup[1][d];
                rsups.vap_size ++ ;
                d++;
            }
            if(k==n-1){
                break;
            }
        }
        nnz -- ;
    }
}
void genLexAll(int totalOfVars, int Deg, class spvec_array & rsups){
    
    int nnz = 0;
    vector<vector<int> > sup(2);
    sup[0].clear();
    sup[1].clear();
    sup[0].resize(totalOfVars, 0);
    sup[1].resize(totalOfVars, 0);
    
    int psize, vsize;
    count_lexall_num_a_nnz(totalOfVars, Deg, psize, vsize);
    rsups.alloc(psize, vsize);
    
    rsups.pnz[0][0] = -1;
    rsups.pnz[1][0] =  0;
    
    rsups.pnz_size = 1;
    rsups.vap_size = 0;
    
    if(Deg > 1){
        for(int W=1;W<=Deg;W++){
            for(int k=0;k<totalOfVars;k++){
                genLexFixDeg(k, totalOfVars, W, sup, nnz, rsups);
            }
        }
    }
}

//int get_binarySup(class polysystem & polysys, class spvec_array &removesups){
void get_binarySup(class polysystem & polysys, vector<int> & binvec){
	int size=polysys.numsys();
	int idx;
	for(int i=1;i<size;i++){
		idx = polysys.polyIsBinary(i);
		if(idx != -1){
			binvec.push_back(idx);	
			//cout << "i = " << i << endl;
			polysys.removeIdx.push_back(i);
		}
	}
}
//int get_SquareOneSup(class polysystem & polysys, class spvec_array & removesups){
void get_SquareOneSup(class polysystem & polysys, vector<int> & Sqvec){
	//cout<<" ***> get_SquareOneSup ---> "<<endl;
	int size=polysys.numsys();
	int idx;
	for(int i=1;i<size;i++){
		idx = polysys.polyIsSquareOne(i);
		if(idx != -1){
			Sqvec.push_back(idx);	
			polysys.removeIdx.push_back(i);
		}
	}
}
void get_removesups(class polysystem & polysys, class spvec_array & removesups){
	list<class sup> czlist;
	int size=polysys.numsys();
	for(int i=1;i<size;i++){
		class sup Sup;
		double Coef;
		if(polysys.polyIsComplimentarity(i, Sup, Coef)==YES){
			//cout << "Coef = " << Coef << endl;
			if(fabs(Coef) < EPS){
				czlist.push_back(Sup);
				polysys.removeIdx.push_back(i);
			}
		}
	}
	removesups.dim = polysys.dimVar;
	initialize_spvecs(czlist, removesups);
	simplification(removesups);
}
void get_allsups_in_momentmatrix(int dimvar, int mmsize, vector<class bass_info> & bassinfo_mm, class spvec_array & mmsups){
    
    mmsups.dim = dimvar;
    int num = 0;
    int nnz = 0;
	for(int i=0;i<mmsize;i++){;
		num += (bassinfo_mm[i].sup.pnz_size * ( bassinfo_mm[i].sup.pnz_size + 1) ) << 1;
		nnz += bassinfo_mm[i].sup.pnz_size * bassinfo_mm[i].sup.get_nnz();
	}
    
    mmsups.alloc(num, nnz);
    mmsups.pnz_size = 0;
    mmsups.vap_size = 0;
    
    class spvec_array lexallsups;
    for(int i=0;i<mmsize;i++){
        minkovsum(bassinfo_mm[i].sup, lexallsups);
        pushsups(lexallsups, mmsups);
    }
    lexallsups.del();
    simplification(mmsups);
}

void get_momentmatrix_basups(class polysystem & polysys, vector<list<int> > BaIndices, vector<class supSet> & basups, vector<class bass_info> & bassinfo_mm) {
    int bdim;
    list<int>::iterator lit;
    int mmsize = basups.size() - polysys.numsys();
    for(int i=0;i<mmsize;i++){
        bdim = BaIndices[i].size();
        bassinfo_mm[i].dim = bdim;
        bassinfo_mm[i].deg = basups[i+polysys.numsys()].deg();
        bassinfo_mm[i].alloc_pattern(bdim);
        
        bdim = 0;
        lit = BaIndices[i].begin();
        for(;lit != BaIndices[i].end();++lit){
            bassinfo_mm[i].pattern[bdim] = (*lit);
            bdim++;
        }
        initialize_spvecs(basups[i+polysys.numsys()], bassinfo_mm[i].sup);
        //bassinfo_mm[i].sup.disp();
    }
}

void get_poly_a_bass_info(
        /* IN */  class polysystem & polysys, vector<class supSet> & BaSupVect, vector<class supSet> & mmBaSupVect,
        const int mat_size,
        /* OUT */ vector<class poly_info> & polyinfo, vector<class spvec_array> & bassinfo){
    
    int no_poly = 0;
 
    initialize_polyinfo(polysys, 0, polyinfo[no_poly]);
    no_poly++;
    for(int i=1;i<polysys.numsys();i++){
        if(polysys.polyTypeCone(i)==EQU){
            initialize_polyinfo(polysys, i, polyinfo[no_poly]);
            initialize_spvecs(BaSupVect[i], bassinfo[no_poly]);
            no_poly++;
        }
    }
    for(int i=1;i<polysys.numsys();i++){
        if(polysys.polyTypeCone(i)==INE && BaSupVect[i].size()==1){
            initialize_polyinfo(polysys, i, polyinfo[no_poly]);
            initialize_spvecs(BaSupVect[i], bassinfo[no_poly]);
            no_poly++;
        }
    }
    int mmsize=mmBaSupVect.size();
    for(int i=0;i<mmsize;i++){
        if(mmBaSupVect[i].size() == 1){
            polyinfo[no_poly].typeCone = -999;
            initialize_spvecs(mmBaSupVect[i], bassinfo[no_poly]);
            no_poly++;
        }
    }
    for(int i=0;i<mmsize;i++){
        if(mmBaSupVect[i].size() > 1){
            polyinfo[no_poly].typeCone = -999;
            initialize_spvecs(mmBaSupVect[i], bassinfo[no_poly]);
            no_poly++;
        }
    }
    for(int i=1;i<polysys.numsys();i++){
        if(polysys.polyTypeCone(i)==INE && BaSupVect[i].size()>1){
            initialize_polyinfo(polysys, i, polyinfo[no_poly]);
            initialize_spvecs(BaSupVect[i], bassinfo[no_poly]);
            no_poly++;
        }
    }
    for(int i=1;i<polysys.numsys();i++){
        if(polysys.polyTypeCone(i)==SDP){
            initialize_polyinfo(polysys, i, polyinfo[no_poly]);
            initialize_spvecs(BaSupVect[i], bassinfo[no_poly]);
            no_poly++;
        }
    }
}

void initialize_spvecs(/*IN*/class supSet & supset, /*OUT*/class spvec_array & spvecs){
    
    int size1 = supset.size();
    int size2 = supset.nnz();
    spvecs.dim  = supset.dimVar;
    spvecs.alloc(size1, size2);
    spvecs.pnz_size = size1;
    list<int> vars;
    
    list<class sup>::iterator ite = supset.begin();
    spvecs.vap_size=0;
    vector<int> Idx, Val;
    for(int i=0;i<size1;i++){
        if((*ite).nnz() ==0){
            spvecs.pnz[0][i] = -1;
            spvecs.pnz[1][i] =  0;
        }
        else{//if( (*ite).nnz > 0 )
            spvecs.pnz[0][i] =  spvecs.vap_size;
            spvecs.pnz[1][i] = (*ite).nnz();
            
            Idx.clear();
            Val.clear();
            
            (*ite).getIdxsVals(Idx, Val);
            
            for(int j=0; j<spvecs.pnz[1][i]; j++){
                spvecs.vap[0][spvecs.vap_size] = Idx[j];
                spvecs.vap[1][spvecs.vap_size] = Val[j];
                spvecs.vap_size++;
                vars.push_back(Idx[j]);
            }
        }
        ++ite;
    }
    vars.unique();
    spvecs.dim2 = vars.size();
}

void initialize_spvecs(/*IN*/list<class sup> & suplist, /*OUT*/class spvec_array & spvecs){
    
    int size1 = suplist.size();
    int size2=0;
    list<class sup>::iterator ite = suplist.begin();
    for(int i=0;i<size1;i++){
        size2 += (*ite).nnz();
        ++ite;
    }
    spvecs.alloc(size1, size2);
    spvecs.pnz_size = size1;
    ite = suplist.begin();
    spvecs.vap_size=0;
    vector<int> Idx, Val;
    for(int i=0;i<size1;i++){
        if((*ite).nnz() ==0){
            spvecs.pnz[0][i] = -1;
            spvecs.pnz[1][i] =  0;
        }
        else{//if( (*ite).nnz > 0 )
            spvecs.pnz[0][i] =  spvecs.vap_size;
            spvecs.pnz[1][i] = (*ite).nnz();
            
            Idx.clear();
            Val.clear();
            
            (*ite).getIdxsVals(Idx, Val);
            
            for(int j=0; j<spvecs.pnz[1][i]; j++){
                spvecs.vap[0][spvecs.vap_size] = Idx[j];
                spvecs.vap[1][spvecs.vap_size] = Val[j];
                spvecs.vap_size++;
            }
        }
        
        ++ite;
    }    
}

void initialize_polyinfo(/*IN*/class polysystem & polysys, int nop, /*OUT*/class poly_info & polyinfo){
    //set typeCone, sizeCone and number of Monomials.
    polyinfo.typeCone = polysys.polynomial[nop].typeCone;
    polyinfo.sizeCone = polysys.polynomial[nop].sizeCone;
    polyinfo.numMs = polysys.polynomial[nop].monoList.size();
    polyinfo.no = nop;
    
    //set data of all supports
    list<class sup> suplist;
    polysys.polynomial[nop].pushSupList(suplist);
    initialize_spvecs(suplist, polyinfo.sup);

    
    //alloc memory to set all nonzeros of the coefficients
    if(polyinfo.typeCone != SDP){
        polyinfo.alloc_coef(polyinfo.typeCone, polyinfo.sizeCone, polyinfo.numMs, 0);
        //cout << "size of polyinfo.coef = " << polyinfo.coef.size() << endl;
    }else{
        polyinfo.alloc_coef(polyinfo.typeCone, polyinfo.sizeCone, polyinfo.numMs, polysys.polyCoefNnz(nop));
    }
    
    //set data of all coefficients
    list<class mono>::iterator ite = polysys.polynomial[nop].monoList.begin();
    int nummonos = polysys.polynomial[nop].monoList.size();
    vector<double> Co;
    int lencoef;
    
    if(polyinfo.typeCone != SDP){
        Co.resize(polyinfo.sizeCone, 0);
        lencoef = polyinfo.sizeCone;
        //cout << "nummonos = " << nummonos << endl;
        for(int i=0;i<nummonos;i++){
            Co.clear();
            (*ite).copyCoef(Co);
            for(int j=0;j<lencoef;j++){
                polyinfo.coef[i][j] = Co[j];
            }
            ++ite;
        }
    }
    else{
        int idx = 0;
        int stidx, edidx;
        int matsize;
        
        for(int i=0;i<nummonos;i++){
            Co.clear();
            (*ite).copyCoef(Co);
            matsize = i*polyinfo.sizeCone;
            for(int j=0;j<polyinfo.sizeCone;j++){
                polyinfo.mc[matsize+j] = idx;
                stidx = j*polyinfo.sizeCone;
                edidx = stidx + j + 1;
                for(int k= stidx; k < edidx ; k ++){
                    if(fabs(Co[k]) > 1.0e-8){
                        polyinfo.mr  [idx]    = k - stidx;
                        polyinfo.coef[idx][0] = Co[k];
                        idx ++ ;
                    }
                }
            }
            ++ite;
        }
        polyinfo.mc[nummonos*polyinfo.sizeCone] = idx;
    }
}

void rescale_sol(int dimvar, vector<double> & pMat, vector<double> & bVec, double * & sol){
    vector<double> yyy(dimvar);
    for(int i=0; i<dimvar; i++){
        yyy[i] = sol[i];
    }

    for(int i=0;i<dimvar;i++){
        sol[i]=0.0;
        for(int j=0;j<dimvar;j++){
            if(pMat[i*dimvar+j] != 0.0 ){
                sol[i] += yyy[j]*pMat[i*dimvar+j];
            }
        }
        if(pMat[i*dimvar+i] != 0.0){
            sol[i] -= bVec[i];///pMat[i*dimvar+i];
        }
    }
}

void write_sdpa(/*IN*/class mysdp & psdp, /*OUT*/ string sdpafile){
    FILE *fp;
    fp = fopen(sdpafile.c_str(), "w+");
    if(fp == NULL){
        printf("file open error\n");
        exit(EXIT_FAILURE);
    }

    int size = 0;
    for(int i=1; i<psdp.nBlocks+1;i++){
        size = size + psdp.block_info[1][i];
    }
	if(psdp.mDim <= 0){
		cout << "# All variables are removed in sdpa format" << endl;
		return;
	}
	if(psdp.nBlocks <= 0){
		cout << "# All blocks are removed in sdpa format" << endl;
		return;
	}
    
    printf("* SDPA sparse format data\n");
    printf("* File name = %s\n", sdpafile.c_str());
    printf("* mDim = %3d, nBlock = %2d\n", psdp.mDim, psdp.nBlocks);
    printf("* size of bVect = 1 * %3d\n", psdp.mDim);
    printf("* size of sparseMatrix = %4d * 5\n", size);
    
    fprintf(fp, "* SDPA sparse format data\n");
    fprintf(fp, "* File name = %s\n", sdpafile.c_str());
    fprintf(fp, "* mDim = %3d, nBlock = %2d\n", psdp.mDim, psdp.nBlocks);
    fprintf(fp, "* size of bVect = 1 * %3d\n", psdp.mDim);
    fprintf(fp, "* size of sparseMatrix = %4d * 5\n", size);
    fprintf(fp, "%3d\n", psdp.mDim);
    fprintf(fp, "%3d\n", psdp.nBlocks);
    
    for(int i=1;i<psdp.nBlocks+1;i++){
        //fout<<" "<<psdp.bLOCKsTruct[i];
        fprintf(fp, "%2d ", psdp.bLOCKsTruct[i]);
    }
    fprintf(fp, "\n");
    //fout<<endl;
    
    vector<double> obj_coef(psdp.mDim+1);
    obj_coef.clear();
    obj_coef.resize(psdp.mDim+1, 0);
    
    for(int j=psdp.block_info[0][0];j<psdp.block_info[0][0]+psdp.block_info[1][0];j++){
        obj_coef[psdp.ele.sup.pnz[0][j]-1] = psdp.ele.coef[j];
    }
    
    for(int i=0;i<psdp.mDim;i++){
        //fout<<obj_coef[i]<<" ";
        fprintf(fp, "%15.10f ", obj_coef[i]);
    }
    fprintf(fp, "\n");
    //fout<<endl;
    
    for(int i=1;i<psdp.nBlocks+1;i++){
        for(int j=psdp.block_info[0][i];j<psdp.block_info[0][i]+psdp.block_info[1][i];j++){            
            fprintf(fp, "%3d ", psdp.ele.sup.pnz[0][j]);
            fprintf(fp, "%3d ", psdp.ele.bij[0][j]);
            fprintf(fp, "%3d ", psdp.ele.bij[1][j]);
            fprintf(fp, "%3d ", psdp.ele.bij[2][j]);
            fprintf(fp, "%15.10f\n", psdp.ele.coef[j]);
        }
    }
    fclose(fp);
}

void perturb_objective(class poly & objpoly, int dimvar, double eps){
    class spvec_array sups;
    //incomplete function!!
}
//class s3r's constructor
s3r :: s3r(){
    this->timedata1.clear();
    this->timedata1.resize(10, 0);
    this->timedata.clear();
    this->timedata.resize(21, 0);
}

void conversion_part1(
        /*IN*/  class s3r & sr,
        /*OUT*/ double & objconst,
        int & slen, vector<double> & scalevalue,
        int & blen, vector<double> & bvect,
        int & mlen, vector<double> & permmatrix) {
    if(sr.param.detailedInfFile.empty() == false){
        sr.param.write_parameters(sr.param.detailedInfFile);
    }
    sr.timedata1[0] = (double)clock();
    if(sr.param.detailedInfFile.empty() == false){
        sr.write_pop(0, sr.param.detailedInfFile);
    }
    sr.timedata1[1] = (double)clock();
    blen = sr.Polysys.dimvar();
    mlen = sr.Polysys.dimvar();
    
    sr.timedata1[2] = (double)clock();
    //sr.Polysys.writePolynomials();
	int tf = sr.Polysys.checkBMI();
	if(tf == 1){
		sr.param.scalingSW = 0;
	}
    if(sr.param.scalingSW == 1){
        sr.Polysys.itemp = 222;
        //double t1 = (double)clock();
        sr.Polysys.scalingPOP(permmatrix, bvect, scalevalue, sr.param.symbolicMath);
        slen = scalevalue.size();
    }
    else{
        int NumOfActiveBounds = 0;
        for(int i=0;i<sr.Polysys.dimvar();i++){
            sr.Polysys.boundsNew.setUp(i+1, sr.Polysys.bounds.ubd(i));
            sr.Polysys.boundsNew.setLow(i+1, sr.Polysys.bounds.lbd(i));
            if(sr.Polysys.bounds.ubd(i) > MIN && sr.Polysys.bounds.ubd(i) < MAX){
                NumOfActiveBounds++;
            }
            if(sr.Polysys.bounds.lbd(i) > MIN && sr.Polysys.bounds.lbd(i) < MAX){
                NumOfActiveBounds++;
            }
        }

        // // this section takes our original bounds and creates constraints from them, removing for our purposes
        // //add upper and lower bounds as constraints to constraints system
        // int numSys = sr.Polysys.numSys; // current num constraints
        // vector<poly> tmpPolys(numSys);
        // for(int i=0; i< numSys; i++){
        //     tmpPolys[i] = sr.Polysys.polynomial[i]; // copy each constraint
        // }
        // sr.Polysys.polynomial.resize(numSys+NumOfActiveBounds); // make room for new bounds
        // for(int i=0; i< numSys; i++){
        //     sr.Polysys.polynomial[i] = tmpPolys[i]; // put original constraints back 
        //     tmpPolys[i].clear(); // clean temp storage
        // }
        // tmpPolys.clear();
        // sr.Polysys.boundToIneqPolySys(); // create new polynomial constraints for each bound
        // //finish adding up and low bounds as constraints 

        //eliminate constant from objective function
        sr.Polysys.layawayObjConst();
        //set permutation matrix and constant vector
        permmatrix.resize(mlen, 0);
        for(int j=0;j<sr.Polysys.dimvar();j++){
            permmatrix[j]=1.0;
        }
        bvect.resize(blen, 0);
        slen = sr.Polysys.numSys;
        scalevalue.resize(slen, 1);
        for(int j=0;j<sr.Polysys.numSys;j++){
            scalevalue[j]=1.0;
        }
    }
    
    sr.timedata1[3] = (double)clock();

    //get data of objective function' constant
    objconst = sr.Polysys.objConst;
    sr.timedata1[4] = (double)clock();
    
    //perturbate objective function.
    if( sr.param.perturbation > 1.0E-12 ){
        sr.Polysys.perturbObjPoly(3201, sr.param.perturbation);
    }
    sr.timedata1[5] = (double)clock();
    
    if(fabs(sr.param.eqTolerance) > EPS){
        sr.Polysys.relax1EqTo2Ineqs(sr.param.eqTolerance);
    }
    sr.timedata1[6] = (double)clock();
    
    if(sr.param.detailedInfFile.empty() == false){
        if(sr.param.scalingSW == 1 || abs(sr.param.eqTolerance) > EPS || abs(sr.param.perturbation) > EPS){
            sr.write_pop(1, sr.param.detailedInfFile);
        }
    }
    sr.timedata1[7] = (double)clock();
    //system("top -b -n 1 | grep MATLAB | head -1 |awk '{printf(\"memory = %s\\n\"), $6}' ");
    //cout << "***End of conversion_part1" << endl;
    sr.timedata1[8] = sr.timedata1[7];
    sr.timedata1[9] = sr.timedata1[7];
    //sr.Polysys.writePolynomials();
}

// Print polynomial function for testing
void printPolynomial(const poly& p, const string& name = "Polynomial") {
    cout << "=== " << name << " ===" << endl;
    //cout << "System #: " << p.noSys << endl;
    cout << "Type: " << (p.typeCone == 1 ? "INEQUALITY" : p.typeCone == -1 ? "EQUALITY" : "OTHER");
    cout << ", Variables: " << p.dimVar;
    cout << ", Degree: " << p.degree;
    cout << ", Terms: " << p.noTerms << endl;
    cout << "Expression: ";
    if (p.noTerms == 0) {
        cout << "0" << endl;
        return;
    }
    list<mono>::const_iterator it = p.monoList.begin();
    bool first = true;
    for (; it != p.monoList.end(); ++it) {
        const mono& m = *it;
        double coef = m.Coef[0];  // Assuming single coefficient
        // Print coefficient
        if (coef >= 0 && !first) {
            cout << " + ";
        } else if (coef < 0) {
            cout << (first ? "-" : " - ");
            coef = fabs(coef);
        }
        if (coef != 1.0 || m.supIdx.empty()) {
            cout << coef;
        }
        // Print variables
        for (int i = 0; i < m.supIdx.size(); i++) {
            int varIdx = m.supIdx[i];
            int power = m.supVal[i];
            
            cout << "x" << varIdx;
            if (power > 1) {
                cout << "^" << power;
            }
        }
        first = false;
    }
    if (p.typeCone == -1) {
        cout << " = 0";
    } else if (p.typeCone == 1) {
        cout << " >= 0";
    }
    cout << endl;
    cout << "==================" << endl;
}

void conversion_part2(
        /*IN*/  class s3r & sr,
        vector<vector<double>>& fixedVar,
        vector<int>& varMapping,
        vector<int> & oriidx,
        class SparseMat & extofcsp,
        /*OUT*/ class mysdp & sdpdata) {

#ifdef DEBUG
	printf("conversion_part2 start\n");
#endif
    int stsize;
    int mmsize;
    int mmsetSize;
    
    vector<class supSet> mmBaSupVect;
    
    class supsetSet BasisSupports;
    class poly_info polyinfo_obj;
    class spvec_array allsups_st;
    class spvec_array removesups;
	vector<int> binvec, Sqvec;
    class supSet czSups, binSups, SqSups;
    class spvec_array mmsups;
    class spvec_array allsups;
    
	int val = 0;
    sr.timedata[0] = (double)clock();
	val = getmem();
    
    // Generate max cliques
    // These will be used to find variable sets with running intersection property
    if(sr.param.sparseSW == 0){
        sr.maxcliques.initialize(sr.Polysys.dimVar, 1);
        for(int i=0;i<sr.Polysys.dimVar ;i++){
            sr.maxcliques.clique[0].push_back(i);
        }
    }else if(sr.param.sparseSW == 1){
        gen_maxcliques3(sr.Polysys.dimVar, oriidx, extofcsp, sr.maxcliques);
    }else{
        printf("sparseSW should be 0 or 1.\n");
    }
    if(sr.param.detailedInfFile.empty() == false){
        sr.maxcliques.write_maxCliques(sr.param.detailedInfFile);
    }
    sr.timedata[1] = (double)clock();
	val = getmem();
    
    // Generate basis indices to be stored in sr.bindices
    // bindices[i] stores the variable indices that will be used in the moment matrix for the i-th constraint
    // note this may include variables that are not present in the constraint itself, depending on the variable set chosen
    // in dense mode:  use all variables
    // in sparse mode: find smallest clique that contains all variables present
    gen_basisindices(sr.param.sparseSW, sr.param.multiCliquesFactor, sr.Polysys, sr.maxcliques, sr.bindices);
    
    if(sr.param.detailedInfFile.empty() == false){
        sr.write_BasisIndices(sr.param.detailedInfFile);
    }
    sr.timedata[2] = (double)clock();
	val = getmem();

    //////////////////////////////////
    //// My SparsePOP Additions //////
    //////////////////////////////////
    cout << endl;
    cout << "Original system:" << endl;
    cout << "  Original variables: " << sr.OriPolysys.dimVar << endl;
    cout << "  Original polynomials: " << sr.OriPolysys.polynomial.size() << endl;
    cout << "Current system:" << endl;
    cout << "  Current variables: " << sr.Polysys.dimVar << endl;
    cout << "  Current polynomials: " << sr.Polysys.polynomial.size() << endl;
    // Print Variable Information 
    // uh this is being sussy 
    cout << "=== VARIABLE MAPPING ===" << endl;
    cout << "fixedVar[0].size() = " << fixedVar[0].size() << endl;
    cout << "fixedVar[1].size() = " << fixedVar[1].size() << endl;
    cout << "Variable to constants:" << endl;
    int eliminated = 0;
    for (int i = 0; i < fixedVar[0].size(); i++) {
        if (fixedVar[0][i] == 1) {
            cout << "  Variable " << i << ": ELIMINATED (set to " << fixedVar[1][i] << ")" << endl;
            eliminated++;
        } else {
            cout << "  Variable " << i << ": REMAINS" << endl;
        }
    }
    cout << "=== END MAPPING ===" << endl;
    cout << "=== Variable Renaming ===" << endl;
    for (int i = 0; i < varMapping.size(); i++){
        cout << "Variable " << varMapping[i] << " is now Variable " << i << endl; 
    }
    cout << "=== End Variable Renaming ===" << '\n' << endl;

    // Print Original Polynomials
    cout << "------Printing Original Polynomials-------" << endl;
    for (int i = 0; i < sr.Polysys.polynomial.size(); i++){
        printPolynomial(sr.Polysys.polynomial[i]);
    }
    cout << "------Done Printing Original Polynomials-------" << '\n' << endl;

    for (int i = 0; i < sr.Polysys.polynomial.size(); i++){
        cout << "Checking out polynomial " << i << endl;
        cout << "It includes the following monomials: "; 
        vector<int> monomials; 
        for (auto& mono : sr.Polysys.polynomial[i].monoList){
            for (int k = 0; k < mono.supIdx.size(); k++){
                cout << "(" << mono.supIdx[k] << ")";
                monomials.push_back(mono.supIdx[k]);
            }
            cout << " "; 
        }
        cout << endl;

        // Delete this later, check if 0 monomial is in polynomial 
        bool replace = false; 
        for (auto& elem : monomials){
            auto previous = varMapping[elem];
            if (previous == 1) {
                replace = true; 
                break;
            }
        }
        // if 0 monomial is present, replace that thing
        if (replace) {
            // instantiate new polynomial 
            class poly new_poly = sr.Polysys.polynomial[i];
            int newVarIndex = sr.Polysys.dimVar; // New variable is bindices[sr.Polysys.dimVar]
            // Add bounds for new variable (same as previous bounds)
            // copy lowerbounds from variable 0
            double lowerBound = sr.Polysys.boundsNew.lbd(0);  // 0-indexed access
            double upperBound = sr.Polysys.boundsNew.ubd(0);
            // // add new bounds
            sr.Polysys.boundsNew.setUp(newVarIndex + 1, upperBound);  // 1-indexed input, weird
            sr.Polysys.boundsNew.setLow(newVarIndex + 1, lowerBound);
            // increase dimvar because we add a new variable 
            // sr.Polysys.dimVar += 1;
            
            // replace all instances of variable 0 in new_poly with variable newVarIndex
            for (auto& mono : new_poly.monoList){
                for (int k = 0; k < mono.supIdx.size(); k++){
                    //mono.supIdx[k] = 2; 
                    if (mono.supIdx[k] == 0) mono.supIdx[k] = 1; 
                }
            }
            printPolynomial(new_poly);  // print newly formed polynomial 

            /// ADDING POLYNOMIAL AND CONSTRAINT ///
            // // update bindices
            // auto originalBind = sr.bindices[i]; // list<int>
            // auto newBind = originalBind; 
            // newBind.front() = 1; 
            // cout << "Obindize for " << i << ": " << originalBind.size() << " and front: " << originalBind.front() << endl;
            // cout << "Nbindize for " << i << ": " << newBind.size() << " and front: " << newBind.front() << endl;

            // // Add polynomial and new variable!
            // sr.Polysys.dimVar += 1; // update total num of variables
            // sr.Polysys.numSys += 1; // update total num of constraints

            // // Resize bounds arrays to match new dimVar
            // sr.Polysys.bounds.allocUpLo(sr.Polysys.dimVar);
            // sr.Polysys.boundsNew.allocUpLo(sr.Polysys.dimVar);

            // // update bound and newBounds for variable 1 (1-indexed) with the 
            // // same bounds as those for variable 0
            // sr.Polysys.bounds.setLow(1+1, sr.Polysys.bounds.lbd(0));      
            // sr.Polysys.bounds.setUp(1+1, sr.Polysys.bounds.ubd(0));
            // sr.Polysys.boundsNew.setLow(1+1, sr.Polysys.boundsNew.lbd(0));
            // sr.Polysys.boundsNew.setUp(1+1, sr.Polysys.boundsNew.ubd(0));

            // sr.Polysys.posOflbds.resize(sr.Polysys.dimVar);
            // sr.Polysys.posOfubds.resize(sr.Polysys.dimVar);

            // fixedVar[0].push_back(0); // adding entry for new variable, it is 0 because it was not eliminated during preprocessing
            // fixedVar[1].push_back(0.0); // dummy value

            // // add new polynomial to constraints
            // sr.Polysys.polynomial.push_back(new_poly);
            // // bindices have already been updated 
            // // fixedVar already extended as well

            // // Regenerate basis and supports
            // sr.genAllSups(BasisSupports);
            // /// FINISH ADDING POLYNOMIAL AND CONSTRAINT ///
            break;
        }
    }
    cout << endl;
    //////////////////////////////////
    //////////////////////////////////
    
    // Generate sets of basis supports
    // takes the variable sets created by gen_basisindices and generates the monomial supports
    // this is all combinations of monomials up to a certain degree
    // these suppports are used to create the localizing and moment matrices
    // BasisSupports.supsetArray is an array of supSet objects, with supsetArray[i] for constraint i
    // Each supSet object holds a list of sup objects, each of which indicate a monomial by 
    // by storing the index of the atoms involved and their respective exponents
    // This mostly utilizes bindices, and does not look at max cliques, so be careful about changing what is stored in bindices
    sr.genBasisSupports(BasisSupports);
    if(sr.param.detailedInfFile.empty() == false){
        sr.write_BasisSupports(0, sr.param.detailedInfFile, BasisSupports);
    }
    
    sr.timedata[3] = (double)clock();
	val = getmem();
    
    //get polyinfo_obj( array data-type to have polynomial form data )
    initialize_polyinfo(sr.Polysys, 0, polyinfo_obj);
    sr.timedata[4] = (double)clock();
	val = getmem();
    
    // Prepare for constraint processing
    stsize = sr.Polysys.numsys() -1 ;
    //generate all supports being consisted polynomial sdp without moment matrices
    vector<class poly_info> polyinfo_st;
    vector<class bass_info> bassinfo_st;
    if(stsize >= 1){
        polyinfo_st.resize(stsize);
        bassinfo_st.resize(stsize);
        get_subjectto_polys_and_basups(sr.Polysys, sr.bindices, BasisSupports.supsetArray, stsize, polyinfo_st, bassinfo_st);
    }
    sr.timedata[5] = (double)clock();
	val = getmem();
    // Create global set of supports, i.e. exhaustive set of monomials that can be referenced anywhere
    // built by taking union of all monomials from the objective, constraints, and all basis supports
    // these are stored in allsups_st
    get_allsups(sr.Polysys.dimvar(), polyinfo_obj, stsize, polyinfo_st, bassinfo_st, allsups_st);
    sr.timedata[6] = (double)clock();
	val = getmem();

    // The monomials gathered by get_allsups are stored in a spvec_array, however we desire them to be sup objects
    // this function will do that for us, adding each one to allSups
    // allSups is now a supSet: a set of all monomials, each as a sup object.
    class supSet allSups;
    initialize_supset(allsups_st, allSups);
    sr.timedata[7] = (double)clock();
	val = getmem();
    
    //reduce each basis supports
    if(sr.param.reduceMomentMatSW == 1){
        sr.reduceSupSets(BasisSupports, allSups);
    }
    if(sr.param.detailedInfFile.empty() == false){
        sr.write_BasisSupports(1, sr.param.detailedInfFile, BasisSupports);
    }
	sr.timedata[8] = (double)clock();
	val = getmem();

    // Filter Basis Supports for Redundancy // 
	//eliminate supports of each basis supports, using special complementarity x*y=0
	if(sr.param.complementaritySW==YES){
		get_removesups(sr.Polysys, removesups);
		initialize_supset(removesups, czSups);
		if(czSups.size()>0){
			sr.eraseCompZeroSups(czSups, BasisSupports.supsetArray);
			sr.eraseCompZeroSups(czSups);
		}
	}
	//eliminate supports of each basis supports, using  xi*(xi-1)=0
	if(sr.param.binarySW==YES){
		get_binarySup(sr.Polysys, binvec);
		if(binvec.empty() == false){
			sr.eraseBinarySups(binvec, BasisSupports.supsetArray);
			sr.eraseBinaryInObj(binvec);
		}
	}
	//eliminate supports of each basis supports, using xi^2 -1=0
	if(sr.param.SquareOneSW==YES){
		get_SquareOneSup(sr.Polysys, Sqvec);
		if(Sqvec.empty() == false){
			sr.eraseSquareOneSups(Sqvec, BasisSupports.supsetArray);
			sr.eraseSquareOneInObj(Sqvec);
		}
	}
	vector<int> remainIdx;
	sr.Polysys.removeEQU(remainIdx);

	int num = sr.Polysys.removeIdx.size(); // Analyze system of polynomials and determine which constraints can be eliminated
	BasisSupports.removeEQU(num, remainIdx); // Remove basis supports of removed constraints

	sr.timedata[9] = (double)clock();
	val = getmem();
    
    // Prepare for moment matrix creation
	mmsize = BasisSupports.supsetArray.size() - sr.Polysys.numsys(); // get number of moment matrices
	vector<class bass_info> bassinfo_mm(mmsize); // will hold basis information for each moment matrix
	// Fills bassinfo_mm with the variable sets and supports for each moment matrix, using the basis indices and supports from earlier steps.
	get_momentmatrix_basups(sr.Polysys, sr.bindices, BasisSupports.supsetArray, bassinfo_mm);

    sr.timedata[10] = (double)clock();
	val = getmem();

    // Generate all the monomials (supports) that will appear as entries in the moment matrices for the SDP relaxation.
    // 1) Takes the basis supports (monomials) for that matrix that are determined in genBasisSupports and stored in bassinfo_mm
    // 2) Forms all possible products of these monomials
    // 3) Collects all unique resulting monomials. these are the supports that will actually appear as entries in the moment matrix
    // supports generated are stored in a spvec_array called mmsups
    // they are not explicitly labeled or grouped by which constraint or basis vector they came from
	get_allsups_in_momentmatrix(sr.Polysys.dimvar(), mmsize, bassinfo_mm, mmsups);
	sr.timedata[11] = (double)clock();
	val = getmem();
    
	//generate all supports being consisted POP
    // Allocates a new spvec_array (allsups) big enough to hold all monomials from both
    // allsups_st (objective/constraints) and mmsups (moment matrices)
	allsups.alloc(allsups_st.pnz_size + mmsups.pnz_size,
	allsups_st.vap_size + mmsups.vap_size );
	allsups.pnz_size = 0;
	allsups.vap_size = 0;
	allsups.dim = allsups_st.dim;
	pushsups(allsups_st, allsups);
	if(mmsups.vap_size > 0){
		pushsups(mmsups, allsups);
	}
    // Simplifies the array (removes duplicates, sorts, etc) //
	simplification(allsups);
	sr.timedata[12] = (double)clock();
	val = getmem();
	//eliminate vain supports of allsupports, using special complementary supports x(a) = 0
	//binary constraints xi^2 -xi = 0 and SquareOne constraints xi^2 - 1 = 0 
	if(sr.param.complementaritySW == YES  && removesups.pnz_size > 0){
		remove_sups(removesups, allsups);
	}
	if(sr.param.binarySW == YES  && binvec.empty() == false){
		remove_Binarysups(binvec, allsups);
	}
	if(sr.param.SquareOneSW == YES  && Sqvec.empty() == false){
		remove_SquareOnesups(Sqvec, allsups);
	}
	sr.timedata[13] = (double)clock();
	val = getmem();
    
    // convert global supports(allsups) to supset format(allSups)
	initialize_supset(allsups, allSups);
	sr.timedata[14] = (double)clock();
	val = getmem();
    
	bool flag = true;
	for(int i=0;i<sr.Polysys.dimVar ;i++){
		if(fabs(sr.Polysys.boundsNew.lbd(i)) > EPS || fabs(sr.Polysys.boundsNew.ubd(i)-1) > EPS){
			flag = false;
			break;
		}
	}
	int numofbds=0;
	if(sr.param.boundSW == 1 && flag){
		allSups.unique();
		class supSet OneSup, ZeroSup;
		ZeroSup = allSups;
		OneSup  = allSups;
		sr.Polysys.addBoundToPOP(ZeroSup, OneSup, numofbds);
	}else if(sr.param.boundSW == 2 && flag){
		allSups.unique();
		class supSet OneSup, ZeroSup;
		sr.redundant_OneBounds(BasisSupports, allSups, OneSup);
		sr.redundant_ZeroBounds(BasisSupports, allSups, ZeroSup);
		sr.Polysys.addBoundToPOP(ZeroSup, OneSup, numofbds);
	}else if(sr.param.boundSW == 2 && !flag){
		allSups.unique();
		allSups.sort();
		sr.Polysys.addBoundToPOP_simple(allSups, numofbds);
	}else if(sr.param.boundSW == 1 && !flag){
		allSups.unique();
		allSups.sort();
		sr.Polysys.addBoundToPOP_simple(allSups, numofbds);
	}
	sr.timedata[15] = (double)clock();
	val = getmem();
	//cout << "15 " << sr.timedata[15] << endl;
    
	mmsetSize=BasisSupports.supsetArray.size()-(sr.Polysys.numsys()-numofbds);
	for(int i=0;i<mmsetSize;i++){
		mmBaSupVect.push_back(BasisSupports.supsetArray[i+(sr.Polysys.numsys()-numofbds)]);
	}
	//eliminate basis supports of moment matrices from BasisSupports data;
	BasisSupports.supsetArray.resize(sr.Polysys.numsys()-numofbds);
	if(sr.param.boundSW == 1 || sr.param.boundSW == 2){
		for(int i=0;i<numofbds;i++){
			class supSet SE1Set;
			class sup    constSup;
			SE1Set.setDimVar(sr.Polysys.dimvar());
			SE1Set.pushSup(constSup);
			BasisSupports.push(SE1Set);
		}
	}
	sr.timedata[16] = (double)clock();
	val = getmem();
	//cout << "16 " << sr.timedata[16] << endl;
    
	//polyinfo,bassinfo
	int msize = sr.Polysys.numsys() + mmBaSupVect.size();
	vector<class poly_info> polyinfo(msize);
	vector<class spvec_array> bassinfo(msize);
	get_poly_a_bass_info(sr.Polysys, BasisSupports.supsetArray, mmBaSupVect, msize, polyinfo, bassinfo);
	sr.timedata[17] = (double)clock();
	val = getmem();
	//cout << "17 " << sr.timedata[17] << endl;

	//generate olynomial sdp
	get_psdp(sr.Polysys.dimvar(), msize, polyinfo, bassinfo, sdpdata);
	sr.timedata[18] = (double)clock();
	val = getmem();
	//cout << "18 " << sr.timedata[18] << endl;
    
	if(sr.param.complementaritySW == YES && removesups.pnz_size > 0){
		remove_sups(sdpdata, removesups);
	}
	if(sr.param.binarySW == YES  && binvec.empty() == false){
		//cout << "Before" << endl;
    		//sdpdata.disp();
		remove_Binarysups(sdpdata, binvec);
		//cout << "After" << endl;
    		//sdpdata.disp();
	}else{
    		//sdpdata.disp();
	}
	if(sr.param.SquareOneSW == YES  && Sqvec.empty() == false){
		//cout << "Before" << endl;
		remove_SquareOnesups(sdpdata, Sqvec);
		//cout << "After" << endl;
	}
    sr.timedata[19] = (double)clock();
	val = getmem();

    //linearize polynomial sdp
    get_lsdp(allsups, sdpdata, sr.degOneTerms, sr.xIdxVec);
    
    //
    // 2009-06-09 Waki
    // Remove the function to write SDP as the sdpa sparse format
    sr.timedata[20] = (double)clock();
	val = getmem();
    //cout << "20 " << sr.timedata[20] << endl;
#ifdef DEBUG
	double t;
	for(int i=0; i<20; i++){
		t = (sr.timedata[i+1] - sr.timedata[i])/(double)CLOCKS_PER_SEC;
		if(t > 1){
			printf("[%2d] %4.2f sec\n", i,t);
		}
	}

	double total = (double)(sr.timedata[20]-sr.timedata[0])/(double)CLOCKS_PER_SEC;
	printf("total = %5.2f sec\n", total);
#endif
}


void copy_polynomialsdpdata(/*IN*/class mysdp & opsdp, /*OUT*/class mysdp & npsdp){
    
    npsdp.mDim = opsdp.mDim;
    npsdp.nBlocks = opsdp.nBlocks;
    
    npsdp.alloc(opsdp.nBlocks, opsdp.ele.sup.pnz_size, opsdp.ele.sup.vap_size);
    npsdp.ele.sup.pnz_size = opsdp.ele.sup.pnz_size;
    npsdp.ele.sup.vap_size = opsdp.ele.sup.vap_size;
    
    for(int i=0;i<npsdp.nBlocks;i++){
        npsdp.bLOCKsTruct[i]   = opsdp.bLOCKsTruct[i];
        npsdp.block_info[0][i] = opsdp.block_info[0][i];
        npsdp.block_info[1][i] = opsdp.block_info[1][i];
        npsdp.block_info[2][i] = opsdp.block_info[2][i];
    }
    
    for(int i=0;i<opsdp.nBlocks;i++){
        for(int j=opsdp.block_info[0][i];j<opsdp.block_info[0][i]+opsdp.block_info[1][i];j++){
            
            npsdp.ele.bij[0][j] = opsdp.ele.bij[0][j];
            npsdp.ele.bij[1][j] = opsdp.ele.bij[1][j];
            npsdp.ele.bij[2][j] = opsdp.ele.bij[2][j];
            
            npsdp.ele.sup.pnz[0][j] = opsdp.ele.sup.pnz[0][j];
            npsdp.ele.sup.pnz[1][j] = opsdp.ele.sup.pnz[1][j];
            
            npsdp.ele.coef[j]   = opsdp.ele.coef[j];
            
            for(int k=opsdp.ele.sup.pnz[0][j];k<opsdp.ele.sup.pnz[0][j]+opsdp.ele.sup.pnz[1][j];k++){
                npsdp.ele.sup.vap[0][k] = opsdp.ele.sup.vap[0][k];
                npsdp.ele.sup.vap[1][k] = opsdp.ele.sup.vap[1][k];
            }
            
        }
    }
    
    
}
//void qsort_normal(/*IN*/vector<int> a, int left, int right, /*OUT*/vector<int> sortedorder) {
//    int i, j, stand;
//    if (left < right){
//        i = left; j = right; stand = a[sortedorder[left]];
//        while(i <= j){
//            while(a[sortedorder[i]]<stand) i++;
//            while(a[sortedorder[j]]>stand) j--;
//            if(i <= j) {
//                swap2(sortedorder, i, j);
//                i++;
//                j--;
//            }
//        }
//        qsort_normal(a, left, j, sortedorder);
//        qsort_normal(a, i, right, sortedorder);
//    }
//}
/*
void swap2(vector<int> a, const int & i, const int & j){
    int dum ;
    dum = a[i];
    a[i] = a[j];
    a[j] = dum;
}
*/
void swap(int* a, const int & i, const int & j){
    int dum ;
    dum = a[i];
    a[i] = a[j];
    a[j] = dum;
}

