#include <iostream>
#include "ALU.hpp"

const int reg_size=32;
const int RS_size=32;

int pc_fetch;

struct RegStation{
    bool Busy;
    int Reorder;
}RegStat_in[reg_size],RegStat_out[reg_size];

struct Node{
    int Qj,Qk,Dest;
    unsigned Vj,Vk,imm,opcode;
    int Busy;
};

struct ROB_Node{
    unsigned Value,opcode;
    int Dest,Type,pc;
    bool Ready;
};

const int QUEUE_SIZE=32;
template<class T>
class Queue{
private:
public:
    T data[QUEUE_SIZE];
    int head,tail;
    Queue(){
        head=tail=1;
    }
    void push(const T &ins){
        data[tail++]=ins;
        if(tail==QUEUE_SIZE)tail=1;
    }
    T front(){
        return data[head];
    }
    void pop(){
        head++;
        if(head==QUEUE_SIZE)head=1;
    }
    bool empty(){
        return head==tail;
    }
    T operator [](const int &x)const{
        return data[x];
    }
    T &operator [] (const int &x){
        return data[x];
    }
    void reset(){
        tail=head=1;
    }
    bool full(){
        int next=tail+1;
        if(next==QUEUE_SIZE)next=1;
        return next==head;
    }
};

Queue<ROB_Node> ROB_in,ROB_out;

Queue<Node> LSB_in,LSB_out;

Queue<Command> inst_in,inst_out;

void print_rob(){
    cerr<<"ROB_opcode: ";
    for(int i=ROB_in.head;i!=ROB_in.tail;i=(i+1==QUEUE_SIZE?1:i+1)){
        cerr<<hex<<ROB_in[i].opcode<<' ';
    }
    cerr<<endl;
}

class Reservation_Station{
private:
public:
    Node RS[RS_size];

    Reservation_Station(){}
    
    int insert(){
        for(int i=0;i<RS_size;i++)
            if(!RS[i].Busy)return i;
        puts("insert wrong");
        return -1;
    }
    
    int query(){
        for(int i=0;i<RS_size;i++)
            if(RS[i].Busy&&!RS[i].Qj&&!RS[i].Qk)return i;
        return -1;
    }
    Node operator [] (const int &x)const{
        return RS[x];
    }
    Node &operator [](const int &x){
        return RS[x];
    }

    void reset(){
        for(int i=0;i<RS_size;i++)RS[i].Busy=0;
    }

    void print(){
        puts("------");
        for(int i=0;i<RS_size;i++)
            if(RS[i].Busy)cout<<dec<<i<<':'<<hex<<RS[i].opcode<<' '<<dec<<RS[i].Qj<<' '<<RS[i].Qk<<endl;
    }

    bool full(){
        for(int i=0;i<RS_size;i++)
            if(RS[i].Busy==0)return 0;
        return 1;
    }

}RS_in,RS_out;

class Memory{
private:    
    unsigned mem[500005];
public:
    Memory(){}
    unsigned Ask(int pos,int byt){
        if(byt==1)return mem[pos];
        else if(byt==2)return (mem[pos+1]<<8)|mem[pos];
        return (mem[pos+3]<<24)|(mem[pos+2]<<16)|(mem[pos+1]<<8)|mem[pos];
    }

    void modify(int pos,unsigned to,int byt){ 
        mem[pos]=ask(to,0,7);
        if(byt==2)mem[pos+1]=ask(to,8,15);
        else if(byt==4){
            mem[pos+1]=ask(to,8,15);
            mem[pos+2]=ask(to,16,23);
            mem[pos+3]=ask(to,24,31);
        }
    }
}mem;

unsigned reg_in[reg_size],reg_out[reg_size];

class Counter{
public:
    bool cnt[500005][2];
    Counter(){
        for(int i=0;i<500005;i++)cnt[i][1]=1;
    }
    bool Ask(int pos){
        return cnt[pos][1];
    }
    void del(int pos){
        if(!cnt[pos][0]){
            cnt[pos][0]=cnt[pos][1];
            cnt[pos][1]=0;
        }else cnt[pos][0]=0;
    }
    void add(int pos){
        if(cnt[pos][0]){
            cnt[pos][0]=cnt[pos][1];
            cnt[pos][1]=1;
        }else cnt[pos][0]=1;
    } 
}cnt;