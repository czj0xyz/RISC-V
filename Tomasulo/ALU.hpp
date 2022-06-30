#include <iostream>
using namespace std;

const unsigned Full=-1;

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

struct Command{
    int rs1,rs2,rd,Type;
    unsigned imm,opcode;
    Command(){}
    Command(const int &rs1_,const int &rs2_,const int &rd_,const int &Type_,const unsigned &imm_,const unsigned &opcode_):
        rs1(rs1_),rs2(rs2_),rd(rd_),Type(Type_),imm(imm_),opcode(opcode_){}
};
//0:rs1+rs2 1:rs1+imm 2:load 3:store 4:branch+rs1+rs2 5:branch+rd

Command get_command(const unsigned &cmd,const int &pc){
    unsigned type=ask(cmd,0,6),imm,shamt,opcode=cmd;
    int rd,rs1,rs2,Type,type3;
    rd=ask(cmd,7,11);
    rs1=ask(cmd,15,19);
    rs2=ask(cmd,20,24);
    type3=ask(cmd,25,31);
    switch(type){
        case 0b0110011: Type=0;break;
        case 0b0010011:
            imm=sext(ask(cmd,20,31),0,11);
            Type=1;
        break;
        case 0b0100011:
            imm=sext((type3<<5)|rd,0,11);
            Type=3;
        break;
        case 0b0000011:
            imm=sext(ask(cmd,20,31),0,11);
            Type=2;
        break;
        case 0b1100011:
            imm=sext( ask(cmd,7,7)<<11|ask(cmd,8,11)<<1|ask(cmd,31,31)<<12|ask(cmd,25,30)<<5 ,1,12);
            Type=4;
        break;
        case 0b1100111://jalr
            imm=sext(ask(cmd,20,31),0,11);
            Type=5;
        break;
        case 0b1101111://jal
            imm=sext(ask(cmd,12,19)<<12|ask(cmd,20,20)<<11|ask(cmd,21,30)<<1|ask(cmd,31,31)<<20,1,20)+pc;
            Type=5;
        break;
        case 0b0010111://aupic
            imm=(ask(cmd,12,31)<<12)+pc;
            rs1=-1;
            Type=1;
        break;//lui
        case 0b0110111:
            Type=1;
            imm=ask(cmd,12,31)<<12;
            rs1=-1;
        break;
        default: opcode=0;
    }
    return Command(rs1,rs2,rd,Type,imm,opcode);
}

unsigned calc(const unsigned &rs1,const unsigned &rs2,const unsigned &imm,const unsigned &cmd){
    unsigned type=ask(cmd,0,6),type2=ask(cmd,12,14),type3=ask(cmd,25,31);
    switch(type){
        case 0b0110011: 
            switch(type2){
                case 0b000:return !type3?rs1+rs2:rs1-rs2;//add & sub
                case 0b001:return rs1<<ask(rs2,0,4);//sll
                case 0b010:return (int)rs1<(int)rs2;//slt
                case 0b011:return rs1<rs2;//sltu
                case 0b100:return rs1^rs2;//xor
                case 0b101:return !type3?rs1>>imm:(unsigned)( ((int)rs1)>>ask(rs2,0,4) );//srl & sra
                case 0b110:return rs1|rs2;//or
                case 0b111:return rs1&rs2;//and
            }
        break;
        case 0b0010011:
            switch(type2){
                case 0b001:return rs1<<imm;//slli
                case 0b101:return !type3?rs1>>imm:(unsigned)( ((int)rs1)>>imm );//srli & srai
                case 0b000:return rs1+imm;//addi
                case 0b010:return (int)rs1<(int)imm;//slti
                case 0b011:return rs1<imm;//sltiu
                case 0b100:return rs1^imm;//xori
                case 0b110:return rs1|imm;//ori
                case 0b111:return rs1&imm;//andi
            }
        break; 
        case 0b0100011:return rs1+imm;//load
        case 0b0000011:return rs1+imm;//store
        case 0b1100011://branch( s1,rs2 ) 
            switch(type2){
                case 0b000:return rs1==rs2;//beq
                case 0b001:return rs1!=rs2;//bne
                case 0b100:return (int)rs1<(int)rs2;//blt
                case 0b101:return (int)rs1>=(int)rs2;//bge
                case 0b110:return rs1<rs2;//bltu
                case 0b111:return rs1>=rs2;//bgeu
            }
        break;
        case 0b1100111:return (rs1+imm)&(~1);//jalr
        case 0b1101111:return imm;//jal
        case 0b0010111:return imm;//imm(auipc)
        case 0b0110111:return imm;//lui
    }
    return 0;
}