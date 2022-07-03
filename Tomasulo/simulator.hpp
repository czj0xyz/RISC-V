#include <bits/stdc++.h>

#include "prework.hpp"
using namespace std;

void Init(){
    pc_fetch=0;
    auto get_num=[](const char &c){return isdigit(c)?c-'0':c-'A'+10;};
    static char S[105];
    int st=0;
    while(~scanf("%s",S)){
        if(S[0]=='@'){
            st=0;
            for(int i=1;i<=8;i++)st=st*16+get_num(S[i]);
        }else{
            int num=(get_num(S[0])<<4)+get_num(S[1]);
            mem.modify(st++,num,1);
        }
    }
}

Node ins,update_LSB_in,update_LSB_out;
ROB_Node ins_ROB,Store;
bool ins_RS_fl=0,ins_ROB_fl=0,ins_LSB_fl=0;//是否插入

bool update_RS_fl=0;//RS是否有更新，若更新用calc_RS更新
bool ROB_pop_fl=0;//ROB是否需要pop

bool calc_RS_fl_in=0,calc_RS_fl_out=0;//RS中是否有需要被计算的，用RS更新
int calc_RS_in,calc_RS_out;//计算/更新 所对应的RS下标

bool update_LSB_fl_in=0,update_LSB_fl_out=0;//LSB 对应 ROB 是否需要更新

bool memory_store;

unsigned result_RS;
unsigned result_LSB_in,result_LSB_out;//计算结果

bool is_to_RS_fl=0;
int is_to_RS,is_to_RS_value;

bool com_to_RS_fl=0;
int com_to_RS;

bool is_fl=0;

void Reset(){
    RS_out.reset(),RS_in.reset();
    LSB_out.reset(),LSB_in.reset();
    ROB_out.reset(),ROB_in.reset();
    inst_out.reset(),inst_in.reset();
    ins_RS_fl=ins_ROB_fl=ins_LSB_fl=0;
    for(int i=0;i<reg_size;i++)
        RegStat_in[i].Busy=RegStat_out[i].Busy=0;
    calc_RS_fl_in=calc_RS_fl_out=0;
    update_RS_fl=0;
    update_LSB_fl_in=update_LSB_fl_out=0;
    memory_store=0;
    ROB_pop_fl=0;

    com_to_RS_fl=is_to_RS_fl=is_fl=0;
}

void update(){
    
    for(int i=0;i<reg_size;i++)
        reg_in[i]=reg_out[i];
    RS_in=RS_out;
    ROB_in=ROB_out;
    LSB_in=LSB_out;
    calc_RS_in=calc_RS_out;
    for(int i=0;i<reg_size;i++)
        RegStat_in[i]=RegStat_out[i];

    calc_RS_fl_in=calc_RS_fl_out;

    update_LSB_fl_in=update_LSB_fl_out;
    update_LSB_in=update_LSB_out;
    result_LSB_in=result_LSB_out;
    inst_in=inst_out;
}

void run_rob(){
    if(ins_ROB_fl)ROB_out.push(ins_ROB);

    if(ROB_pop_fl)ROB_out.pop();

    if(update_RS_fl){//根据result更新答案 
        unsigned opcode=RS_in[calc_RS_in].opcode;
        int h=RS_in[calc_RS_in].Dest;
        if(ROB_in[h].Type==4){
            ROB_out[h].Value=RS_in[calc_RS_in].imm;
            ROB_out[h].Dest|=result_RS<<1;
        }else ROB_out[h].Value=result_RS;
        ROB_out[h].Ready=1;
    }

    if(update_LSB_fl_in){
        Node now=update_LSB_in;
        if(ROB_in[now.Dest].Type==2){//load
            ROB_out[now.Dest].Value=result_LSB_in;
        }else{
            ROB_out[now.Dest].Value=now.Vk;
            ROB_out[now.Dest].Dest=result_LSB_in;
        }
        ROB_out[now.Dest].Ready=1;

    }
}

void run_regfile(){
    if(com_to_RS_fl)RegStat_out[com_to_RS].Busy=0;
    
    if(is_to_RS_fl){
        RegStat_out[is_to_RS].Busy=1;
        RegStat_out[is_to_RS].Reorder=is_to_RS_value;
    }
}

void run_reservation(){
    
    if(ins_RS_fl){
        if(update_RS_fl){
            int h=RS_in[calc_RS_in].Dest;
            if(ins.Qj==h)ins.Qj=0,ins.Vj=result_RS;
            if(ins.Qk==h)ins.Qk=0,ins.Vk=result_RS;
        }
        if(update_LSB_fl_in){
            int h=update_LSB_in.Dest;
            if(ROB_in[h].Type==2){
                if(ins.Qj==h)ins.Qj=0,ins.Vj=result_LSB_in;
                if(ins.Qk==h)ins.Qk=0,ins.Vk=result_LSB_in;
            }
        }
        RS_out[RS_in.insert()]=ins;
    }

    if(update_RS_fl){
        int h=RS_in[calc_RS_in].Dest;
        for(int i=0;i<RS_size;i++)
            if(RS_in[i].Busy){
                if(RS_in[i].Qj==h)RS_out[i].Qj=0,RS_out[i].Vj=result_RS;
                if(RS_in[i].Qk==h)RS_out[i].Qk=0,RS_out[i].Vk=result_RS;
            }
    }

    if(update_LSB_fl_in){
        int h=update_LSB_in.Dest;
        if(ROB_in[h].Type==2){
            for(int i=0;i<RS_size;i++)
                if(RS_in[i].Busy){
                    if(RS_in[i].Qj==h)RS_out[i].Qj=0,RS_out[i].Vj=result_LSB_in;
                    if(RS_in[i].Qk==h)RS_out[i].Qk=0,RS_out[i].Vk=result_LSB_in;
                }
        }
    }

    int id=RS_in.query();
    if(id==-1)calc_RS_fl_out=0;
    else {
        calc_RS_fl_out=1;
        RS_out[id].Busy=0;
        calc_RS_out=id;
    }
}

int get_store_byte(const unsigned &cmd){
    unsigned type2=ask(cmd,12,14);
    switch(type2){
        case 0b000:return 1;//sb
        case 0b001:return 2;//sh
        case 0b010:return 4;//sw
    }
    return 233;
}

void run_slbuffer(){ 
    update_LSB_fl_out=0;

    if(ins_LSB_fl){
        if(update_RS_fl){
            int h=RS_in[calc_RS_in].Dest;
            if(ins.Qj==h)ins.Qj=0,ins.Vj=result_RS;
            if(ins.Qk==h)ins.Qk=0,ins.Vk=result_RS;
        }
        if(update_LSB_fl_in){
            int h=update_LSB_in.Dest;
            if(ROB_in[h].Type==2){
                if(ins.Qj==h)ins.Qj=0,ins.Vj=result_LSB_in;
                if(ins.Qk==h)ins.Qk=0,ins.Vk=result_LSB_in;
            }
        }
        LSB_out.push(ins);
    }
    
    if(update_RS_fl){
        int h=RS_in[calc_RS_in].Dest;
        for(int i=LSB_in.head;i!=LSB_in.tail;i=(i+1==QUEUE_SIZE?1:i+1)){
            if(LSB_in[i].Qj==h)LSB_out[i].Vj=result_RS,LSB_out[i].Qj=0;
            if(LSB_in[i].Qk==h)LSB_out[i].Vk=result_RS,LSB_out[i].Qk=0;
        }
    }

    if(update_LSB_fl_in){
        int h=update_LSB_in.Dest;
        if(ROB_in[h].Type==2){
            for(int i=LSB_in.head;i!=LSB_in.tail;i=(i+1==QUEUE_SIZE?1:i+1)){
                if(LSB_in[i].Qj==h)LSB_out[i].Vj=result_LSB_in,LSB_out[i].Qj=0;
                if(LSB_in[i].Qk==h)LSB_out[i].Vk=result_LSB_in,LSB_out[i].Qk=0;
            }
        }
    }

    if(!LSB_in.empty()){
        Node now=LSB_in.front();
        if(ROB_in[now.Dest].Type==2){//load
            if(!now.Qj&&!now.Qk){
                if(now.Busy==1){//未计算
                    LSB_out[LSB_in.head].Busy=0;
                    result_LSB_out=calc(now.Vj,now.Vk,now.imm,now.opcode);
                }else {
                    unsigned type2=ask(now.opcode,12,14);
                    int bt=0;
                    switch(type2){
                        case 0b000:bt=1;break;
                        case 0b001:bt=2;break;
                        case 0b010:bt=4;break;
                        case 0b100:bt=1;break;
                        case 0b101:bt=2;break;
                    }
                    result_LSB_out=mem.Ask(result_LSB_in,bt);
                    if(type2==0b000||type2==0b001)result_LSB_out=sext(result_LSB_out,0,bt*8-1);
                    update_LSB_fl_out=1;
                    update_LSB_out=now;
                    LSB_out.pop();
                }
            }
        }else {//store 
            if(!now.Qj&&!now.Qk){
                if(now.Busy==1){
                    LSB_out[LSB_in.head].Busy=2;
                    result_LSB_out=calc(now.Vj,now.Vk,now.imm,now.opcode);
                }else if(now.Busy==2){
                    update_LSB_fl_out=1;
                    LSB_out[LSB_in.head].Busy=3;
                    update_LSB_out=now;
                }else if(memory_store){
                    mem.modify(Store.Dest,Store.Value,get_store_byte(Store.opcode));
                    LSB_out.pop();
                }
            }
        }
        
    }
}

void run_inst_fetch_queue(){
    if(!inst_in.full()){
        Command inst=get_command(mem.Ask(pc_fetch,4),pc_fetch);
        if(inst.Type==4){//branch
            if(cnt.Ask(pc_fetch)){
                pc_fetch+=inst.imm;
                inst.flag=1;
            }else {
                pc_fetch+=4;
                inst.flag=0;
            }
        }else if(inst.Type==5){

            if(ask(inst.opcode,0,6)==0b1101111){//jal
                int last=pc_fetch;
                pc_fetch=inst.imm;
                inst.imm=last+4 ;
                
            }else pc_fetch+=4;

        }else pc_fetch+=4;
        inst_out.push( inst );
    }

    if(is_fl)inst_out.pop();   
}

void run_ex(){
    update_RS_fl=0;
    result_RS=0;
    if(calc_RS_fl_in){
        Node now=RS_in[calc_RS_in];
        result_RS=calc(now.Vj,now.Vk,now.imm,now.opcode);
        update_RS_fl=1;
    }

}

int commit_opcode;

//0:rs1+rs2 1:rs1+imm 2:load 3:store 4:branch+rs1+rs2 5:branch+rd
void run_issue(){

    ins_RS_fl=ins_ROB_fl=ins_LSB_fl=is_to_RS_fl=is_fl=0;
    if(LSB_in.full()||RS_in.full()||ROB_in.full()){
        return;
    }
    Command inst=inst_in.front();
    is_fl=1;

    ins.Dest=ROB_in.tail;
    ins.opcode=inst.opcode;
    ins.Busy=1;
    ins.imm=inst.imm;
    ins.Qj=ins.Qk=0;

    ins_ROB.pc=inst.pc;
    ins_ROB.Ready=0;
    ins_ROB.opcode=inst.opcode;
    ins_ROB.Dest=inst.rd;
    ins_ROB.Type=inst.Type;
    ins_ROB_fl=1;
    
    if(inst.rs1!=-1){//rs1
        
        if(!RegStat_in[inst.rs1].Busy)ins.Vj=reg_in[inst.rs1],ins.Qj=0;
        else {
            int h=RegStat_in[inst.rs1].Reorder;
            if(ROB_in[h].Ready){
                ins.Vj=ROB_in[h].Value;
                ins.Qj=0;
            }else ins.Qj=h;
        }
        
    }
    if(inst.Type==0||inst.Type==3||inst.Type==4){
        if(!RegStat_in[inst.rs2].Busy)
            ins.Vk=reg_in[inst.rs2],ins.Qk=0;
        else {
            int h=RegStat_in[inst.rs2].Reorder;
            if(ROB_in[h].Ready){
                ins.Vk=ROB_in[h].Value;
                ins.Qk=0;
            }else ins.Qk=h;
        }
    }

    if(inst.Type==0||inst.Type==1){// calc
        ins_RS_fl=1;
        
        is_to_RS_fl=1;
        is_to_RS=inst.rd;
        is_to_RS_value=ROB_in.tail;
    }

    if(inst.Type==2){//load    mem[reg[rs1]+imm] ->reg[rd]
        ins_LSB_fl=1;
        ins.Qk=ins.Vk=0;

        is_to_RS_fl=1;
        is_to_RS=inst.rd;
        is_to_RS_value=ROB_in.tail;
    }

    if(inst.Type==3){//store reg[rs2]-> mem[ reg[rs1]+imm ]
        ins_LSB_fl=1;
    }

    if(inst.Type==4){//branch+rs1+rs2
        ins_ROB.Dest=inst.flag;
        ins_RS_fl=1;
    }

    if(inst.Type==5){//branch+rd
        ins_RS_fl=1;
        if(inst.rd){
            is_to_RS_fl=1;
            is_to_RS=inst.rd;
            is_to_RS_value=ROB_in.tail;
        }
    }
}

int CNT=0,failed=0;

void run_commit(){
    ROB_pop_fl=memory_store=commit_opcode=com_to_RS_fl=0;
    if(!ROB_in.empty()){
        ROB_Node now=ROB_in.front();
        if(!now.Ready)return;
        commit_opcode=now.opcode;
        bool flag=0;
        if(now.Type==0||now.Type==1)reg_out[now.Dest]=now.Value;//calc
        else if(now.Type==2)reg_out[now.Dest]=now.Value;//load
        else if(now.Type==3)memory_store=1,Store=now;//store
        else {//branch
            if(now.Type==4){
                CNT++;
                if(now.Dest&2)cnt.add(now.pc);
                else cnt.del(now.pc);
                if(now.Dest==1||now.Dest==2){
                    if(now.Dest==2)pc_fetch=now.pc+now.Value;
                    else pc_fetch=now.pc+4;
                    Reset();
                    flag=1;
                    failed++;
                }
            }else if(now.Type==5){
                if(now.Dest!=0) reg_out[now.Dest]=now.pc+4;
                if(ask(now.opcode,0,6)==0b1100111){
                    pc_fetch=now.Value;
                    Reset();
                    flag=1;
                }
            }
        }
        if(!flag){
            if(now.Type!=3&&RegStat_in[now.Dest].Reorder==ROB_in.head){
                if(now.Type==5&&now.Dest==0);
                else{
                    com_to_RS_fl=1;
                    com_to_RS=now.Dest;
                }
            }
            ROB_pop_fl=1;
        }
    }
}

void (*func[])() = { run_slbuffer, run_reservation, run_rob, run_inst_fetch_queue,run_regfile };

void run(){
    srand(19260817);
    for(int circle=0;;circle++){ 
        random_shuffle(func,func+5);
        for(int i=0;i<5;i++)func[i]();

        update();
        
        run_issue();
        run_ex();
        run_commit();
        
        if(commit_opcode==0x0ff00513){
            cerr<<circle+1<<endl;
            cerr<<1.0*(CNT-failed)/CNT<<endl;
            cout<<dec<<ask(reg_in[10],0,7)<<endl;
            return;
        }
    }
}