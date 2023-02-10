#include<iostream>
#include<string>
#include<cmath>
#include<vector>
#include<fstream>
#include<iomanip>
#include<time.h>
using namespace std;

struct Node{
    int index, ASAP_Stage, ALAP_Stage, mobility, final_Stage;
    char type;
    bool ASAP_Check, ALAP_Check, final_Check;
    vector <int> predecessors, successor;
    float* op_probability;
    float* self_force;
    float* ps_force;
    float* total_force;
};
typedef Node *NodePtr;

int delay_cal(char type){
    if(type == 'i' || type == '+'){ return 1; }
    else if(type == '*'){ return 3; }
    else{ cout<<"This is output"; return 0;}
}

int read_file(char* argv, int* max_latency, int* node_number, int* edge_number, NodePtr* node){
    ifstream ifile;
    ifile.open(argv);
    float Start = clock();
    if(ifile.fail()){
        cout<<"File opening fail!";
        return 0;
    }
    cout << endl << "----------- Reading File -----------" << endl << endl ;
    cout << "Parsing " << argv << endl;
    ifile>> *max_latency; ifile>> *node_number; ifile>> *edge_number;
    *node = new Node[*node_number];
    int IO_count = 0;
    for(int i=0; i<*node_number; i++){
        ifile>>(*node)[i].index;
        ifile>>(*node)[i].type;
        (*node)[i].ASAP_Check = 0;
        (*node)[i].ALAP_Check = 0;
        (*node)[i].final_Check = 0;
        if( (*node)[i].type == 'i' || (*node)[i].type == 'o' ){ IO_count++; }
    }
    for(int i=0; i<*edge_number; i++){
        int edge1, edge2;
        ifile>>edge1>>edge2;
        (*node)[edge2-1].predecessors.push_back(edge1);
        (*node)[edge1-1].successor.push_back(edge2);
    }
    ifile.close();
    float Finish = clock();
    cout << "Execution time: " << ( Finish - Start ) / CLOCKS_PER_SEC << " s" <<endl;
    cout << endl << "------------------------------------" << endl << endl;
    return IO_count ;
};

void ASAP(Node node[], int current_index, int n){
    int count = current_index;
    int last_node = n ;
    while(1){
        int max_time=0;
        if(node[last_node].ASAP_Check){
            if(node[last_node].final_Check){
                node[last_node].ASAP_Stage = node[last_node].final_Stage ;
                node[last_node].ASAP_Check = 1;
                last_node--;
            }
            else{
		        break;
            }
	    }
        if( node[count].final_Check){
            node[count].ASAP_Stage = node[count].final_Stage ; 
            node[count].ASAP_Check = 1;
        }
        else if( !node[count].ASAP_Check ){
            for(int i=0; i<node[count].predecessors.size(); i++){
                // check if predecessor has been scheduled
                int time = 0;
                if( node[ node[count].predecessors[i] - 1].final_Check ){
                    node[ node[count].predecessors[i] - 1].ASAP_Stage =  node[ node[count].predecessors[i] - 1].final_Stage ;
                    time = node[ node[count].predecessors[i] -1 ].final_Stage + delay_cal(node[ node[count].predecessors[i] -1 ].type);
                }
                else if(!node[ node[count].predecessors[i] -1 ].ASAP_Check){ 
                    ASAP( node , ( node[count].predecessors[i] - 1 ), n ); 
                }
                // calculate max t
                else{
                    time = node[ node[count].predecessors[i] -1 ].ASAP_Stage + delay_cal(node[ node[count].predecessors[i] -1 ].type);
                }
                if(time > max_time) { max_time = time; }
            }
            node[count].ASAP_Stage = max_time;
            node[count].ASAP_Check = 1;
        }
        count++;
    }
};

void ALAP(Node node[], int current_index, int max_latency){
    int count = current_index;
    int last_node = 0 ;
    while(1){
        if(node[last_node].ALAP_Check){
            if(node[last_node].final_Check){
                node[last_node].ALAP_Stage = node[last_node].final_Stage ;
                node[last_node].ALAP_Check = 1;
                last_node++;
            }
            else{
                break;
            }
        }
        int min_time = max_latency + 1;
        if ( node[count].final_Check ){
            node[count].ALAP_Stage = node[count].final_Stage ;
            node[count].ALAP_Check = 1;
        }
        else if(!node[count].ALAP_Check){    
            for(int i = 0; i < node[count].successor.size(); i++){
                int time = 0;
                if( node[ node[count].successor[i] - 1].final_Check ){
                    node[ node[count].successor[i] - 1].ALAP_Stage =  node[ node[count].successor[i] - 1].final_Stage ;
                    time = node[ node[count].successor[i]-1 ].final_Stage - delay_cal(node[count].type);
                }
                else if(!node[ node[count].successor[i]-1 ].ALAP_Check  && !node[ node[count].successor[i] - 1].final_Check){ 
                    ALAP(node, (node[count].successor[i]-1), max_latency); 
                }
                else{
                    time = node[ node[count].successor[i]-1 ].ALAP_Stage - delay_cal(node[count].type);
                }
                if(time < min_time){ min_time = time;}
            }
            node[count].ALAP_Stage = min_time;
            node[count].ALAP_Check = 1;
        }
        if(count == 0) { break; }
        count--;
    }
};

void parameter(Node node[], int node_number, int max_latency, float Q_multiplier[], float Q_adder[]){
    for(int i=0; i<max_latency; i++){ Q_multiplier[i] = 0; Q_adder[i] = 0; } // initialize type probability
    for(int i=0; i<node_number; i++){
        if( node[i].type == 'i' || node[i].type == 'o' ){ 
            node[i].op_probability = NULL ;
            continue; 
        }
        node[i].op_probability = new float [max_latency];
        node[i].mobility = node[i].ALAP_Stage - node[i].ASAP_Stage;
        for(int j = 1; j <= max_latency; j++){
            if( (j >= node[i].ASAP_Stage) && (j <= node[i].ALAP_Stage) ){
                node[i].op_probability[j-1] = 1 / float( node[i].mobility + 1 ) ;
                node[i].op_probability[j-1] = round(node[i].op_probability[j-1] * 10000.0 ) / 10000.0 ; // rounding numbers
            }
            else{
                node[i].op_probability[j-1] = 0;
            }
            if( node[i].type == '+' ){
                Q_adder[j-1] += node[i].op_probability[j-1];
                Q_adder[j-1] = round( Q_adder[j-1] * 10000.0 ) / 10000.0 ;
            }
            else if(node[i].type == '*' ){
                Q_multiplier[j-1] += node[i].op_probability[j-1];
                Q_multiplier[j-1] = round( Q_multiplier[j-1] * 10000.0 ) / 10000.0 ;
            }
        }
    }
}

void self_force(Node node[], int node_number, int max_latency, float Q_adder[], float Q_multiplier[]){
    for(int i=0; i<node_number; i++){
        if( node[i].type == 'i' || node[i].type == 'o' || node[i].final_Check == 1){ 
            node[i].self_force = NULL ;
            node[i].total_force = NULL ; 
            continue;
        }  
        node[i].self_force = new float[ max_latency ];
        node[i].total_force = new float[ max_latency ];
        for(int l=node[i].ASAP_Stage; l<=node[i].ALAP_Stage; l++){
            if(node[i].type == '+'){
                node[i].self_force[l-1] = Q_adder[l-1];
                for(int m=node[i].ASAP_Stage; m<=node[i].ALAP_Stage; m++){
                    node[i].self_force[l-1] -= Q_adder[m-1] / (node[i].mobility + 1);
                }
            }
            else if(node[i].type == '*'){
                node[i].self_force[l-1] = Q_multiplier[l-1];
                for(int m=node[i].ASAP_Stage; m<=node[i].ALAP_Stage; m++){
                    node[i].self_force[l-1] -= Q_multiplier[m-1] / (node[i].mobility + 1);
                }
            }
            else{ node[i].self_force[l-1] = 0; }
            node[i].self_force[l-1] = round( node[i].self_force[l-1] * 10000.0 ) / 10000.0 ;
            node[i].total_force[l-1] = node[i].self_force[l-1] ;
        }
    }
}

void ps_force(Node node[], int node_number, int max_latency, float Q_adder[], float Q_multiplier[], int* scheduled_node, int* scheduled_stage ){
    float min_force = INFINITY ;
    for(int i=0; i<node_number; i++){
        if( node[i].type == 'i' || node[i].type == 'o'  || node[i].final_Check == 1 ){
            node[i].ps_force = NULL;
            continue; 
        }
        node[i].ps_force = new float [max_latency];
        for(int l = node[i].ASAP_Stage ; l <= node[i].ALAP_Stage ; l++ ){
            node[i].ps_force[l-1] = 0;
            
            //// Predecessor Force ////
            for(int pre = 0 ; pre < node[i].predecessors.size() ; pre++ ){
                int pre_index = node[i].predecessors[pre] -1 ;
                if( node[pre_index].type == 'i' || node[pre_index].type == 'o' || node[pre_index].final_Check == 1 ){ continue; }
                if( l >= node[pre_index].ASAP_Stage && l <= node[pre_index].ALAP_Stage ){
                    int new_mobility = node[pre_index].ALAP_Stage - l ;
                    if( node[pre_index].type == '+' ){
                        for(int  m = l ; m <= node[pre_index].ALAP_Stage ; m++ ){
                            node[i].ps_force[l-1] += Q_adder[m-1] / ( new_mobility + 1 ) ;
                        }
                        for(int m = node[pre_index].ASAP_Stage ; m <= node[pre_index].ALAP_Stage ; m++){
                            node[i].ps_force[l-1] -= Q_adder[m-1] / ( node[pre_index].mobility + 1 ) ;
                        }
                    }
                    else if( node[pre_index].type == '*' ){
                        for(int m=l ; m <= node[pre_index].ALAP_Stage ; m++){
                            node[i].ps_force[l-1] += Q_multiplier[m-1] / ( new_mobility + 1 ) ;
                        }
                        for(int m = node[pre_index].ASAP_Stage ; m <= node[pre_index].ALAP_Stage ; m++){
                            node[i].ps_force[l-1] -= Q_multiplier[m-1] / ( node[pre_index].mobility + 1 ) ;
                        }
                    }
                    node[i].ps_force[l-1] = round( node[i].ps_force[l-1] * 10000.0 ) / 10000.0 ;
                }
            }
            //// Successor Force ////
            for( int suc = 0 ; suc < node[i].successor.size() ; suc++ ){
                int suc_index = node[i].successor[suc] - 1;
                if( node[ suc_index ].type == 'i' || node[ suc_index ].type == 'o' ){ continue; }
                if( l >= node[suc_index].ASAP_Stage && l <= node[suc_index].ALAP_Stage ){
                    int new_mobility = l - node[suc_index].ASAP_Stage ;
                    if( node[suc_index].type == '+' ){

                        for(int  m =  node[suc_index].ASAP_Stage ; m <= l ; m++ ){
                            node[i].ps_force[l-1] += Q_adder[m-1] / ( new_mobility + 1 ) ;
                        }
                        for(int m = node[suc_index].ASAP_Stage ; m <= node[suc_index].ALAP_Stage ; m++ ){
                            node[i].ps_force[l-1] -= Q_adder[m-1] / ( node[suc_index].mobility + 1 ) ;
                        }
                    }
                    else if ( node[suc_index].type == '*' ){
                        for(int m= node[suc_index].ASAP_Stage ; m <= l ; m++){
                            node[i].ps_force[l-1] += Q_multiplier[m-1] / ( new_mobility + 1 ) ;
                        }
                        for(int m = node[suc_index].ASAP_Stage ; m <= node[suc_index].ALAP_Stage ; m++ ){
                            node[i].ps_force[l-1] -= Q_multiplier[m-1] / ( node[suc_index].mobility + 1 ) ;
                        }
                    }
                    node[i].ps_force[l-1] = round( node[i].ps_force[l-1] * 10000.0 ) / 10000.0 ;
                }
            }
            node[i].total_force[l-1] += node[i].ps_force[l-1] ;
            node[i].total_force[l-1] = round( node[i].total_force[l-1] * 10000.0 ) / 10000.0 ;
            if( node[i].total_force[l-1] < min_force ){
                min_force = node[i].total_force[l-1] ;
                *scheduled_node = i ;
                *scheduled_stage = l ;
            }
        }
        node[i].ASAP_Check = 0;
        node[i].ALAP_Check = 0;
    }
}

void Schedule(Node node[], int node_number, int max_latency, int IO_count, vector<int> Result[] , int adder_count[], int multiplier_count[]){
    float Q_multiplier[max_latency] , Q_adder[max_latency] ;
    int scheduled_count = 0 ;
    int scheduled_node, scheduled_stage ;
    
    cout << endl << "------------ Scheduling ------------" << endl << endl;
    cout <<"Please wait..." << endl;
    float start = clock();
    
    for(int i=0; i<max_latency; i++){
        adder_count[i] = 0 ;
        multiplier_count[i] = 0;
    }
    while ( scheduled_count != (node_number-IO_count) ){
        ASAP( node , 0 , (node_number - 1) );
        ALAP( node , (node_number - 1) , max_latency );
        parameter( node , node_number , max_latency , Q_multiplier , Q_adder );
        self_force( node , node_number , max_latency , Q_adder , Q_multiplier );
        ps_force( node , node_number , max_latency , Q_adder , Q_multiplier , &scheduled_node , &scheduled_stage );

        node[scheduled_node].final_Stage = scheduled_stage ;
        node[scheduled_node].final_Check = 1 ;
        Result[ scheduled_stage - 1 ].push_back( scheduled_node + 1 ) ;
        if ( node[scheduled_node].type == '*' ){
            Result[ scheduled_stage ].push_back( scheduled_node + 1 ) ;
            Result[ scheduled_stage + 1 ].push_back( scheduled_node + 1 ) ;
        }
        if(node[scheduled_node].type == '+'){
            adder_count[ scheduled_stage - 1 ] += 1;
        }
        else if(node[scheduled_node].type == '*'){
            multiplier_count[ scheduled_stage - 1 ] += 1;
            multiplier_count[ scheduled_stage ] += 1;
            multiplier_count[ scheduled_stage + 1 ] += 1;
        }

        scheduled_count++;
        if( scheduled_count == ( node_number - IO_count ) ){ break; }

    }

    float finish = clock();
    cout << "Execution time: " << ( finish - start ) / CLOCKS_PER_SEC << " s" << endl;
    cout<< endl << "------------------------------------" << endl << endl;
    
}

void write_file(char* argv, int max_latency, vector<int> Result[], int adder_count[], int multiplier_count[]){
    int max_adder = 0 , max_multiplier = 0 ;
    for(int i=0; i<max_latency; i++){
        if( adder_count[i] > max_adder ){
            max_adder = adder_count[i] ;
        }
        if( multiplier_count[i] > max_multiplier ){
            max_multiplier = multiplier_count[i] ;
        }
    }
    ofstream ofile;
    ofile.open(argv);
    if(ofile.fail()){
        cout<<"File opening fail!";
        return;
    }
    else{
        cout << endl << "------------ Writing File ----------" << endl << endl ;
    }
    float Start = clock();
    
    ofile << max_adder << endl ;
    ofile << max_multiplier << endl ;
    for(int stage = 0; stage < max_latency; stage++){
        for(int i=0; i < Result[stage].size(); i++){
            ofile << Result[stage][i] ;
            if( i != (Result[stage].size()-1) ){
                ofile << " ";
            }
        }
        if( stage != max_latency - 1){
            ofile << endl ;
        }
    }
    ofile.close();

    float Finish = clock();
    cout << "Execution time: " << ( Finish - Start ) / CLOCKS_PER_SEC << " s" <<endl;

    cout << endl << "------------------------------------" << endl << endl;
}

void print_result(int max_latency, vector<int> Result[], int adder_count[], int multiplier_count[]){
    cout<<endl<<"************** Print Result ***************"<<endl<<endl;
    for(int i=0; i<max_latency; i++){
        cout<<"Nodes that scheduled at stage "<< (i+1) <<" : ";
        for(int j=0; j<Result[i].size(); j++){
            cout<< Result[i][j] << " ";
        }
        cout<<endl;
    }
    int max_adder = 0, max_multiplier = 0 ;
    for(int i=0; i<max_latency; i++){
        if( adder_count[i] > max_adder ){
            max_adder = adder_count[i] ;
        }
        if( multiplier_count[i] > max_multiplier ){
            max_multiplier = multiplier_count[i] ;
        }
    }
    cout<<"Max adder: "<< max_adder << endl ;
    cout<<"Max multiplier: " << max_multiplier << endl;
    cout<<endl<<"*******************************************"<<endl;

}

int main(int argc, char* argv[]){
    int max_latency = 0, node_number = 0, edge_number = 0;
    NodePtr node = NULL;
    cout<<endl<<"**********Start of program**********"<< endl << endl ;
    int IO_count = read_file( argv[1] , &max_latency , &node_number , &edge_number , &node );
    float Q_multiplier[max_latency], Q_adder[max_latency] ;
    int adder_count[max_latency], multiplier_count[max_latency] ;
    vector<int> Result [max_latency] ;
    Schedule( node , node_number , max_latency , IO_count, Result, adder_count, multiplier_count ) ;
    write_file(argv[2] , max_latency , Result , adder_count , multiplier_count );
    //print_result( max_latency, Result, adder_count, multiplier_count );
    cout<<endl<<"********** End of program **********"<< endl << endl ;
    return 0;
}