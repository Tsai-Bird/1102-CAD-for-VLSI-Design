#include<iostream>
#include<string>
#include<fstream>
#include<iomanip>
using namespace std;

// Data Structure(Linked-List)
struct Node{
    int address;
    string name, type;
    Node** from;
    int fanout, fanin;
    Node *nextPtr;
};

typedef Node *NodePtr;
void InsertNode(NodePtr *node, int address, string name, string type,int fanout, int fanin, NodePtr from[]);
NodePtr findNode(NodePtr node, int from_address);
void PrintNode(NodePtr node);

int main(int argc, char* argv[]){
    //input file
    ifstream ifile;
    ifile.open(argv[1]);
    if(ifile.fail()){
        cout<<"File opening fail!";
        return 0;
    }

    //start reading file
    cout<<endl<<"Reading file"<<endl;
    cout<<"Input file name: "<<argv[1]<<endl;
    NodePtr StartNode = NULL;
    NodePtr *from;
    string a, name, type;
    int address, fanout, fanin;
    int input_cnt=0, total_gate_cnt=0, wire_cnt=0, output_cnt=0;
    while(true){
        ifile>>a;
        if(ifile.eof()) break;
        else if(a.find("*") != string::npos) getline(ifile, a);
        else{
            if(a == ">sa0"){ ifile>>a; ifile>>a;}
            else if(a == ">sa1"){ ifile>>a;}
            address = stoi(a);
            ifile>>name;
            ifile>>type;
            if(type == "inpt"){  // if node is input
                input_cnt+=1;
                ifile>>fanout;
                ifile>>fanin;
                getline(ifile, a);
                from = new NodePtr;
		        from=NULL;
            }
            else if(type == "from"){  // if node is fanout from another wire
                string From;
                ifile>>From;
                fanin = 1;
                fanout = 1;
		        from = new NodePtr[fanin];
               	from[0]=findNode(StartNode, stoi(From.erase(From.find("gat"), 3)));
                getline(ifile, a);
            }
            else{  // if node is logic gate or output
                total_gate_cnt+=1;
                ifile>>fanout;
                if(fanout == 0) output_cnt+=1;
                else wire_cnt+=1;
                ifile>>fanin;
                int From[fanin];
                from = new NodePtr[fanin];
                getline(ifile, a);
                for(int i=0; i<fanin; i++){
                    ifile>>From[i];
                    from[i]=findNode(StartNode, From[i]);
                }
            }
            InsertNode(&StartNode, address, name, type, fanout, fanin, from);  // node insertion
        }
    }
    ifile.close();
    total_gate_cnt = wire_cnt + output_cnt;
    NodePtr input[input_cnt], output[output_cnt], wire[wire_cnt], gate[total_gate_cnt];
    int in_cnt=0, G_cnt=0, out_cnt=0, W_cnt=0;
    for(NodePtr start=StartNode; start!=NULL; start=start->nextPtr){ // node classification
        if(start->type == "inpt"){ //find input node
            input[in_cnt]=start;
            in_cnt+=1;
        }
        else if(start->type != "from"){ // classify normal gate or output gate
            gate[G_cnt]=start;
            G_cnt+=1;
            if(start->fanout == 0){
                output[out_cnt]=start;
                out_cnt+=1;
            }
            else{
                wire[W_cnt]=start;
                W_cnt+=1;
            }
        }
    }
    //PrintNode(StartNode); // not used
    ifile.close();

    //output file
    ofstream ofile;
    ofile.open(argv[2]);
    if(ofile.fail()){
        cout<<endl<<"Output file opening fail!"<<endl;
        return 0;
    }
    cout<<endl<<"Writing file"<<endl;
    string Modulename = argv[2];
    cout<<"File name: "<<Modulename<<endl;
    cout<<"Module name: "<<Modulename.erase(Modulename.find(".v"), 2)<<endl;
    ofile<<"`timescale 1ns/1ps "<<endl;
    ofile<<"module "<<Modulename<<" (";  //module declaration
    for(int i=0; i<input_cnt; i++) ofile<<"gat"<<input[i]->address<<", ";
    for(int i=0; i<output_cnt; i++){
        if(i == output_cnt-1) ofile<<"gat_out"<<output[i]->address<<");"<<endl;
        else ofile<<"gat_out"<<output[i]->address<<", ";
    }
    ofile<<"input "; // input declaration
    for(int i=0; i<input_cnt; i++){
        if(i == input_cnt-1) ofile<<"gat"<<input[i]->address<<";"<<endl;
        else ofile<<"gat"<<input[i]->address<<", ";
    }
    ofile<<"output ";  // output declaration
    for(int i=0; i<output_cnt; i++){
        if(i == output_cnt-1) ofile<<"gat_out"<<output[i]->address<<";"<<endl;
        else ofile<<"gat_out"<<output[i]->address<<", ";
    }
    ofile<<"wire ";  // wire declaration
    for(int i=0; i<wire_cnt; i++){ 
        if(i == wire_cnt-1) ofile<<"gat_out"<<wire[i]->address<<";"<<endl;
        else ofile<<"gat_out"<<wire[i]->address<<", ";
    }
    for(int i=0; i<total_gate_cnt; i++){ // logic gate declaration
        if(gate[i]->type == "buff") gate[i]->type.erase(3,1);
        ofile<<gate[i]->type<<" gat"<<gate[i]->address<<" (";
        ofile<<"gat_out"<<gate[i]->address<<", ";
        for(int j=0; j<gate[i]->fanin; j++){
            if(gate[i]->from[j]->type != "inpt" && gate[i]->from[j]->type != "from"){
                if(j == gate[i]->fanin-1)
                    ofile<<"gat_out"<<gate[i]->from[j]->address<<");"<<endl;
                else
                    ofile<<"gat_out"<<gate[i]->from[j]->address<<", ";
            }
            else{
                if(j == gate[i]->fanin-1)
                    ofile<<"gat"<<gate[i]->from[j]->address<<");"<<endl;
                else
                    ofile<<"gat"<<gate[i]->from[j]->address<<", ";
            }
                
        }
    }
    ofile<<"endmodule";
    cout<<"End of writing file"<<endl<<endl;
    ofile.close();
    return 0;
}

// function of insertion
void InsertNode(NodePtr *node, int address, string name, string type,int fanout, int fanin, NodePtr from[]){
    NodePtr next;
    NodePtr current;
    NodePtr previous;
    next = new Node;
    if(next != NULL){    // memory space available
        // create new node
        next->nextPtr = NULL;
        next->address = address;
        next->name = name;
        next->type = type;
        next->fanout = fanout;
        next->fanin = fanin;
        next->from = new NodePtr[fanin];
        for(int i=0; i<fanin; i++) *(next->from+i)=from[i];

        previous = NULL;
        current = *node;
        // find the place to insert
        while(current!=NULL){
            previous = current;
            current = current->nextPtr;
        }
        if(previous == NULL){
            next->nextPtr = *node;
            *node = next;
        }
        else{
            previous->nextPtr = next;
            next->nextPtr = NULL;
        }
    }
    else cout<<"No memory space!";
};

// finind the address of node we want
NodePtr findNode(NodePtr node, int from_address){
    for(; node!=NULL; node=node->nextPtr){
        if(node->address == from_address){
            if(node->type == "from") return *(node->from);
            else return node;
        }
    }
    return NULL;
}

void PrintNode(NodePtr node){ // print out all the node
    cout<<"Print all nodes"<<endl;
    for(; node!=NULL; node=node->nextPtr){
        cout<<node->name;
        if(node->type == "from") cout<<" fanout from "<<(*node->from)->name<<endl;
        else if(node->type == "inpt") cout<<" is input"<<endl;
        else{ 
            cout<<" is "<<node->type<<" with inputs ";
            for(int i=0; i<node->fanin; i++){
                cout<<node->from[i]->name<<" ";
            }
            cout<<endl;
        }
    }
}
