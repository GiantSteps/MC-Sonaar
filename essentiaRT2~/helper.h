//
//  helper.h
//  essentiaRT2~
//
//  Created by Martin Hermant on 18/06/2015.
//
//

#ifndef essentiaRT2__helper_h
#define essentiaRT2__helper_h

using namespace essentia;
using namespace std;
namespace Helper{
    
    
    flext::AtomList  floatVectorToList(vector<Real> const & v){
        flext::AtomList res(v.size());
        
        for(int i = 0 ; i < v.size() ; i++){
            flext::SetFloat(res[i], v[i]);
            
            //            flext::SetFloat(argv[i], v[i]);
        }
        //        res.Set(v.size(), argv);
        return res;
    }
    
    
    
    
    
    typedef struct ioStruct{
        string name;
        enum TYPE {
            REAL,VECTOR,STRING,MATRIX
        };
        TYPE type;
        
        
        enum AGGRTYPE{
            MEAN = 0,MEDIAN
        };
        AGGRTYPE aggrType;
        vector<Real> realValues;
        vector<vector<Real> > vectorValues;
        vector<Real> vectorRes;
        flext::AtomList atomList;
        int curVecSize;
        bool isConsistent;
        vector<string> stringValues;
        
        int countIdx;
        int beginIdx;
        int aggrSize;
        int totalCount;
        bool inited;
        ioStruct(){
            init(1);
        }
        ioStruct(const string &  _name,TYPE Typ ,int num = 1):name(_name),type(Typ){
            init(num);
            aggrType = MEAN;
            
        }
        void init(int num){
            aggrSize = num;
            curVecSize = 1;
            switch(type){
                case REAL:
                    realValues.resize(num);
                    break;
                case VECTOR:
                    vectorValues.resize(num);
                    isConsistent = true;
                    for(auto v:vectorValues){
                        v.resize(curVecSize);
                    }
                    break;
                case STRING:
                    stringValues.resize(num);
                    break;
                default:
                    break;
            }
            beginIdx = 0;
            countIdx = -1;
            inited = false;
            totalCount = 0;
            
            
            
        }
        
        
        vector<Real> & getVectorValue(){
            
            return vectorValues[countIdx];
        }
        string & getStringValue(){
            
            return stringValues[countIdx];
        }
        Real & getRealValue(){
            
            return realValues[countIdx];
        }
        vector<Real> & getNextVectorValue(){
            countIdx++;
            countIdx%=aggrSize;
            totalCount++;
            return vectorValues[countIdx];
        }
        vector<Real> & getNextVectorValue(int s){
            countIdx++;
            countIdx%=
            totalCount++;
            if(s!= curVecSize){
                isConsistent = false;
                curVecSize = s;
                vectorValues[countIdx].resize(curVecSize);
                
                //                post(("no cons : nextV :"+name).c_str());
//                for(auto & v:vectorValues){
//                    v.resize(s);
//                }
            }
            return vectorValues[countIdx];
        }
        string & getNextStringValue(){
            countIdx++;
            countIdx%=aggrSize;
            totalCount++;
            return stringValues[countIdx];
        }
        Real & getNextRealValue(){
            countIdx++;
            countIdx%=aggrSize;
            totalCount++;
            return realValues[countIdx];
        }
        
        
        vector<Real> aggregateVector(){
            inited|= totalCount == aggrSize;
            if(isConsistent){
                for(int i = 0 ; i <aggrSize ; i++){
                    if(vectorValues[i].size()!=curVecSize){
                        //                                    post(("no cons : aggr :"+name).c_str());
                        isConsistent = false;
                        curVecSize = vectorValues[i].size();
                        break;
                    }
                }
            }
            
            if(!isConsistent){
                int maxIdx = beginIdx;
                int maxSize = vectorValues[beginIdx].size();
                for(int i = 1  ; i < aggrSize ; i++){
                    int curI = (i+beginIdx)%aggrSize;
                    if(vectorValues[curI].size()>maxSize){
                        maxSize = vectorValues[curI].size();
                        maxIdx = curI;
                    }
                }
                curVecSize = maxSize;
                return vectorValues[maxIdx];
            }
            vectorRes.resize(curVecSize);
            switch(aggrType){
                case MEAN:
                {
                    bool initV = true;
                    for(int j=0 ; j<aggrSize;j++){
                        if(initV){
                            for(int i = 0 ; i< curVecSize ; i++){
                                vectorRes[i]=vectorValues[j][i];
                            }
                            initV = false;
                        }
                        else{
                            for(int i = 0 ; i< curVecSize ; i++){
                                vectorRes[i]+=vectorValues[j][i];
                            }
                        }
                    }
                    
                    for(auto & i:vectorRes){
                        i/=aggrSize;
                    }
                }
                    break;
                    
                case MEDIAN:
                {
                    vector<Real> tmpSort(curVecSize);
                    for(int i = 0 ; i< curVecSize ; i++){
                        for(int j  =  0 ; j < aggrSize ; j++){
                            tmpSort[j] = vectorValues[i][j];
                        }
                        sort(tmpSort.begin(),tmpSort.end());
                        if(aggrSize%2 == 0){
                            vectorRes[i]  = (tmpSort[aggrSize/2 - 1] +  tmpSort[aggrSize/2])/2;
                        }
                        else{
                            vectorRes[i] = tmpSort[aggrSize/2];
                        }
                    }
                    
                    break;
                }
                default:
                    break;
            }
            
            return vectorRes;
            
        }
        Real aggregateReal(){
            inited|= totalCount == aggrSize;
            switch(aggrType){
                case MEAN:
                    return mean(realValues);
                    break;
                case MEDIAN:
                {
                    vector<Real> tmpSort = realValues;
                    sort(tmpSort.begin(),tmpSort.end());
                    if(aggrSize%2==0){
                        return (tmpSort[aggrSize/2-1]+tmpSort[aggrSize/2]) /2;
                    }
                    else{
                        return tmpSort[aggrSize/2];
                    }
                    break;
                }
            }
            return 0;
        }
        
        string aggregateString(){
            inited|= totalCount == aggrSize;
            map<string , int> histo;
            for(auto & s:stringValues){
                if(histo.count(s)){
                    histo[s]++;
                }
                else{
                    histo[s] = 1;
                }
            }
            string res ;
            int maxC = 0;
            for(auto & kv:histo){
                if(kv.second>maxC){
                    res = kv.first;
                }
            }
            return res;
        }
        
        bool hasReachedHop(int hop){
            return inited && totalCount==hop;
        };
        void reset(int hop = 0){
            beginIdx +=hop!=0?hop:1;
            beginIdx%=aggrSize;
            totalCount =0;
            isConsistent = true;
        }
        bool isUpdated(){return totalCount!=0;};
        
        void setAtomNextVectorValue(int argc,const t_atom * argv){
            countIdx++;
            countIdx%=aggrSize;
            
            isConsistent = argc == curVecSize;
            
            curVecSize = argc;
            if(!isConsistent){
                vectorValues[countIdx].resize(curVecSize);
            }
            for(int i = 0 ; i < argc ; i++){
                vectorValues[countIdx][i] = flext::GetAFloat(argv[i]);
            }
        }
        
        
        void setNextDspvectorValue(t_sample * dspv,int maxN=0){
            countIdx++;
            countIdx%=aggrSize;
            
            isConsistent = dspv[0] == curVecSize;
            curVecSize = dspv[0];
            if(!isConsistent){
                vectorValues[countIdx].resize(curVecSize);
            }
            
            
            memccpy(&vectorValues[countIdx][0], dspv + 1,curVecSize, sizeof(float));
            
        }
        
        
        
    }ioStruct;
    
    
    class vectorCheck{
        
        public :
        
        
        short state;
        
        
//        bool checkVector
        
        
        
        
    };
}
#endif
