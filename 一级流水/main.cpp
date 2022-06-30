#include <bits/stdc++.h>
using namespace std;

unsigned reg[32];
unsigned int mem[500005];

const unsigned Full=-1;

unsigned pc;

inline unsigned sext(const unsigned &x,const int &l,const int &r){
    bool c=((unsigned)1<<r)&x;
    if(!c)return x;
    return x|(Full<<(r+1));
}

inline unsigned ask(const unsigned &x,const int &l,const int &r){
    return (x&( (Full>>(31-r+l))<<l ))>>l;
}

inline int change(unsigned x){
    if(ask(x,31,31)==0)return x;
    return -(x^(1ll<<31));
}

class load_store{
public:
    //load
    void lb(const int &rd,const unsigned &imm,const int &rs1){
        reg[rd]=sext( mem[ reg[rs1]+imm ] ,0,7);
    }

    void lh(const int &rd,const unsigned &imm,const int &rs1){
        reg[rd]=sext( mem[ reg[rs1]+imm ]|mem[reg[rs1]+imm+1]<<8 ,0,15 );
    }

    void lw(const int &rd,const unsigned &imm,const int &rs1){
        reg[rd]=mem[reg[rs1]+imm]|(mem[reg[rs1]+imm+1]<<8)|(mem[reg[rs1]+imm+2]<<16)|(mem[reg[rs1]+imm+3]<<24);
    }

    void lbu(const int &rd,const unsigned &imm,const int &rs1){
        reg[rd]= mem[reg[rs1]+imm];
    }

    void lhu(const int &rd,const unsigned &imm,const int &rs1){
        reg[rd]= mem[ reg[rs1]+imm ]|(mem[reg[rs1]+imm+1]<<8);
    }
    
    //store
    void sb(const int &rs1,const int &rs2,const unsigned &imm){
        mem[reg[rs1]+imm] = ask(reg[rs2],0,7);
    }

    void sh(const int &rs1,const int &rs2,const unsigned &imm){
        mem[reg[rs1]+imm] = ask(reg[rs2],0,7);
        mem[reg[rs1]+imm+1] = ask(reg[rs2],8,15);
    }

    void sw(const int &rs1,const int &rs2,const unsigned &imm){
        mem[reg[rs1]+imm] = ask(reg[rs2],0,7);
        mem[reg[rs1]+imm+1] = ask(reg[rs2],8,15);
        mem[reg[rs1]+imm+2] = ask(reg[rs2],16,23);
        mem[reg[rs1]+imm+3] = ask(reg[rs2],24,31);
    }
}load;

class Calculation{
public:
//int calc
    void add(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]+reg[rs2];
    }

    void addi(const int &rd,const int &rs1,const unsigned &imm){
        reg[rd]=reg[rs1]+imm;
    }

    void sub(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]-reg[rs2];
    }

    void lui(const int &rd,const unsigned &imm){
        reg[rd]=imm;
    }

    void auipc(const int &rd,const unsigned &imm){
        reg[rd]=imm+pc;
    }
//logic calc
    void xor_(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]^reg[rs2];
    }

    void xori(const int &rd,const int &rs1,const unsigned &imm){
        reg[rd]=reg[rs1]^imm;
    }

    void or_(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]|reg[rs2];
    }
    
    void ori(const int &rd,const int &rs1,const unsigned &imm){
        reg[rd]=reg[rs1]|imm;
    }

    void and_(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]&reg[rs2];
    }
    
    void andi(const int &rd,const int &rs1,const unsigned &imm){
        reg[rd]=reg[rs1]&imm;
    }
//move calc
    void sll(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]<<ask(reg[rs2],0,4);
    }

    void slli(const int &rd,const int &rs1,const int &shamt){
        reg[rd]=reg[rs1]<<shamt;
    }
    
    void srl(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]>>ask(reg[rs2],0,4);
    }
    
    void srli(const int &rd,const int &rs1,const int &shamt){
        reg[rd]=reg[rs1]>>shamt;
    }

    void sra(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=(unsigned)( ((int)reg[rs1])>>ask(reg[rs2],0,4) );
    }

    void srai(const int &rd,const int &rs1,const int &shamt){
        reg[rd]=(unsigned)( ((int)reg[rs1])>>shamt );
    }

}calc;

class Compare{
public:
    void slt(const int &rd,const int &rs1,const int &rs2){
        if(ask(reg[rs1],31,31)!=ask(reg[rs2],31,31))
            reg[rd]=ask(reg[rs1],31,31)>ask(reg[rs2],31,31);
        else
            reg[rd]=(reg[rs1]<reg[rs2])^ask(reg[rs1],31,31);
        
    }

    void slti(const int &rd,const int &rs1,const unsigned &imm){ 
        if(ask(reg[rs1],31,31)!=ask(imm,31,31))
            reg[rd]=ask(reg[rs1],31,31)>ask(imm,31,31);
        else
            reg[rd]=(reg[rs1]<imm)^ask(reg[rs1],31,31);
    }

    void sltu(const int &rd,const int &rs1,const int &rs2){
        reg[rd]=reg[rs1]<reg[rs2];
    }

    void sltiu(const int &rd,const int &rs1,const unsigned &imm){
        reg[rd]=reg[rs1]<imm;
    }
}cmp;

class Control{
public:
    void beq(const int &rs1,const int &rs2,const unsigned &offset){
        if(reg[rs1]==reg[rs2])pc+=offset;
    }

    void bne(const int &rs1,const int &rs2,const unsigned &offset){
        if(reg[rs1]!=reg[rs2])pc+=offset;
    }

    void blt(const int &rs1,const int &rs2,const unsigned &offset){
        if(change(reg[rs1])<change(reg[rs2]))pc+=offset;
    }

    void bge(const int &rs1,const int &rs2,const unsigned &offset){
        if(change(reg[rs1])>=change(reg[rs2]))pc+=offset;
    }

    void bltu(const int &rs1,const int &rs2,const unsigned &offset){
        if(reg[rs1]<reg[rs2])pc+=offset;
    }

    void bgeu(const int &rs1,const int &rs2,const unsigned &offset){
        if(reg[rs1]>=reg[rs2])pc+=offset;
    }

    void jal(const int &rd,const unsigned &offset){
        if(rd)reg[rd]=pc+4;
        pc+=offset;
    }

    void jalr(const int &rd,const int &rs1,const unsigned &offset){
        unsigned t=pc+4;
        pc=(reg[rs1]+offset)&(~1);
        if(rd)reg[rd]=t;
    }

}ctl;

void Process(const unsigned &cmd){
    unsigned type=ask(cmd,0,6),rd,rs1,rs2,type2,type3,imm,shamt;
    rd=ask(cmd,7,11);
    type2=ask(cmd,12,14);
    rs1=ask(cmd,15,19);
    rs2=ask(cmd,20,24);
    type3=ask(cmd,25,31);
    switch(type){
        case 0b0110011: 
            switch(type2){
                case 0b000:
                    if(!type3)calc.add(rd,rs1,rs2);
                    else calc.sub(rd,rs1,rs2);
                break;

                case 0b001:calc.sll(rd,rs1,rs2);break;
                case 0b010:cmp.slt(rd,rs1,rs2);break;
                case 0b011:cmp.sltu(rd,rs1,rs2);break;
                case 0b100:calc.xor_(rd,rs1,rs2);break;
                case 0b101:
                    if(!type3)calc.srl(rd,rs1,rs2);
                    else calc.sra(rd,rs1,rs2);
                break;
                case 0b110:calc.or_(rd,rs1,rs2);break;
                case 0b111:calc.and_(rd,rs1,rs2);break;
            }
        break;
        case 0b0010011:
            imm=sext(ask(cmd,20,31),0,11);
            switch(type2){
                case 0b001:calc.slli(rd,rs1,rs2);break;
                case 0b101:
                    if(!type3)calc.srli(rd,rs1,rs2);
                    else calc.srai(rd,rs1,rs2);
                break;
                case 0b000:calc.addi(rd,rs1,imm);break;
                case 0b010:cmp.slti(rd,rs1,imm);break;
                case 0b011:cmp.sltiu(rd,rs1,imm);break;
                case 0b100:calc.xori(rd,rs1,imm);break;
                case 0b110:calc.ori(rd,rs1,imm);break;
                case 0b111:calc.andi(rd,rs1,imm);break;
            }
        break;
        case 0b0100011:
            imm=sext((type3<<5)|rd,0,11);
            switch(type2){
                case 0b000:load.sb(rs1,rs2,imm);break;
                case 0b001:load.sh(rs1,rs2,imm);break;
                case 0b010:load.sw(rs1,rs2,imm);break;
            }
        break;
        case 0b0000011:
            imm=sext(ask(cmd,20,31),0,11);
            switch(type2){
                case 0b000:load.lb(rd,imm,rs1);break;
                case 0b001:load.lh(rd,imm,rs1);break;
                case 0b010:load.lw(rd,imm,rs1);break;
                case 0b100:load.lbu(rd,imm,rs1);break;
                case 0b101:load.lhu(rd,imm,rs1);break;
            }
        break;
        case 0b1100011:
            imm=sext( ask(cmd,7,7)<<11|ask(cmd,8,11)<<1|ask(cmd,31,31)<<12|ask(cmd,25,30)<<5 ,1,12);
            switch(type2){
                case 0b000:ctl.beq(rs1,rs2,imm);break;
                case 0b001:ctl.bne(rs1,rs2,imm);break; 
                case 0b100:ctl.blt(rs1,rs2,imm);break;
                case 0b101:ctl.bge(rs1,rs2,imm);break;
                case 0b110:ctl.bltu(rs1,rs2,imm);break;
                case 0b111:ctl.bgeu(rs1,rs2,imm);break;
            }
        break;
        case 0b1100111:ctl.jalr(rd,rs1,sext(ask(cmd,20,31),0,11));break;
        case 0b1101111:
            imm=sext(ask(cmd,12,19)<<12|ask(cmd,20,20)<<11|ask(cmd,21,30)<<1|ask(cmd,31,31)<<20,1,20);
            ctl.jal(rd,imm);
        break;
        case 0b0010111:calc.auipc(rd,ask(cmd,12,31)<<12);break;
        case 0b0110111:calc.lui(rd,ask(cmd,12,31)<<12);break;
    }
}


int get_num(const char &c){
    return isdigit(c)?c-'0':c-'A'+10;
}

void Init(){
    static char S[105];
    int st=0;
    while(~scanf("%s",S)){
        if(S[0]=='@'){
            st=0;
            for(int i=1;i<=8;i++)st=st*16+get_num(S[i]);
        }else{
            int num=(get_num(S[0])<<4)+get_num(S[1]);
            mem[st++]|=num;
        }
    }
}

unsigned get(){
    return (unsigned)mem[pc]+((unsigned)mem[pc+1]<<8)+((unsigned)mem[pc+2]<<16)+((unsigned)mem[pc+3]<<24);
}

void solve(){
    int CNT=0;
    while(1){
        if(get()==0x0ff00513){
            cout<<dec<<ask(reg[10],0,7)<<endl;
            return;
        }
        int last=pc;
        // cout<<hex<<get()<<endl;
        // CNT++;
        // if(CNT==10000)return;
        // if(!get())return;
        Process(get());
        if(last==pc)pc+=4;
    }

}
int main(){
    // freopen("qsort.data","r",stdin); 
    // freopen("test.out","w",stdout);
    Init();
    solve();
    return 0;
}