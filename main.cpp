#include "mbed.h"

I2C i2c(P0_0,P0_1);         //SDA,SCL oled
SPI spi(P0_9,P0_8,P0_7);    //VGA
DigitalOut cs1(P0_6);
DigitalOut cs2(P0_23);
DigitalOut cs3(P0_24);
DigitalOut cs4(P2_0);

DigitalIn sw0(P1_21);   //rotary1 a
DigitalIn sw1(P1_22);   //rotary1 b
DigitalIn sw2(P1_23);   //rotary2 a
DigitalIn sw3(P1_24);   //rotary2 b
DigitalIn sw4(P1_25);   //rotary3 a
DigitalIn sw5(P1_26);   //rotary3 b
DigitalIn sw6(P1_27);   //rotary4 a
DigitalIn sw7(P1_28);   //rotary4 b

//OLED
const uint8_t oled_adr = 0x78;   //oled i2c adr 0x7C
void oled_init(uint8_t adr);     //lcd init func
void char_disp(uint8_t adr, int8_t position, char data);    //char disp func
void val_disp(uint8_t adr, int8_t position, int8_t val);  //val disp func
void cont(uint8_t adr,uint8_t);     //contrast set

//Rotary state and display
uint8_t r1_state, r2_state, r3_state, r4_state;       //rotary state
uint8_t val_state=0;
int8_t att[4]={18,18,18,18};
uint8_t i;

//SPI
uint8_t spi_buf;

int main(){
    spi.format(6,0);   //spi mode setting. 2byte(16bit) transfer, mode 0
    cs1=1;  //CS high
    cs2=1;  //CS high
    cs3=1;  //CS high
    cs4=1;  //CS high

    thread_sleep_for(100);  //wait for lcd power on
    oled_init(oled_adr);
    cont(oled_adr,0xff);
    char_disp(oled_adr,0,'1');
    char_disp(oled_adr,1,':');
    char_disp(oled_adr,5,'d');
    char_disp(oled_adr,6,'B');
    char_disp(oled_adr,9,'2');
    char_disp(oled_adr,10,':');
    char_disp(oled_adr,14,'d');
    char_disp(oled_adr,15,'B');
    char_disp(oled_adr,0+0x20,'3');
    char_disp(oled_adr,1+0x20,':');
    char_disp(oled_adr,5+0x20,'d');
    char_disp(oled_adr,6+0x20,'B');
    char_disp(oled_adr,9+0x20,'4');
    char_disp(oled_adr,10+0x20,':');
    char_disp(oled_adr,14+0x20,'d');
    char_disp(oled_adr,15+0x20,'B');

    while (true){
        //rotary scan
        r1_state=((r1_state<<1)+sw0)&0b0011;
        if((r1_state==2)&&(sw1==1))val_state=1;      //r1 incr
        else if((r1_state==2)&&(sw1==0))val_state=2; //r1 decr

        r2_state=((r2_state<<1)+sw2)&0b0011;
        if((r2_state==2)&&(sw3==1))val_state=3;      //r2 incr
        else if((r2_state==2)&&(sw3==0))val_state=4; //r2 decr    

        r3_state=((r3_state<<1)+sw4)&0b0011;
        if((r3_state==2)&&(sw5==1))val_state=5;      //r3 incr
        else if((r3_state==2)&&(sw5==0))val_state=6; //r3 decr

        r4_state=((r4_state<<1)+sw6)&0b0011;
        if((r4_state==2)&&(sw7==1))val_state=7;      //r4 incr
        else if((r4_state==2)&&(sw7==0))val_state=8; //r4 decr      

        switch(val_state){
            case 0:
                ;break;
            case 1:
                --att[0];break;
            case 2:
                ++att[0];break;
            case 3:
                --att[1];break;
            case 4:
                ++att[1];break;
            case 5:
                --att[2];break;
            case 6:
                ++att[2];break;
            case 7:
                --att[3];break;
            case 8:
                ++att[3];break;
        }
        val_state=0;

        //overflow check & store past value
        for(i=0;i<4;++i){
            if(att[i]<=0)att[i]=0;
            else if(att[i]>=31)att[i]=31;
        }
        
        //disp refresh & SPI transfer
        val_disp(oled_adr,2,18-att[0]);
        cs1=0;
        spi_buf=(~att[0])<<1;
        spi.write(spi_buf);
        cs1=1;

        val_disp(oled_adr,11,18-att[1]);
        cs2=0;
        spi_buf=(~att[1])<<1;
        spi.write(spi_buf);
        cs2=1;
        
        val_disp(oled_adr,2+0x20,18-att[2]);
        cs3=0;
        spi_buf=(~att[2])<<1;
        spi.write(spi_buf);
        cs3=1;
        
        val_disp(oled_adr,11+0x20,18-att[3]);
        cs4=0;
        spi_buf=(~att[3])<<1;
        spi.write(spi_buf);
        cs4=1;

        thread_sleep_for(1);
    }
}

//LCD init func
void oled_init(uint8_t adr){
    char lcd_data[2];
    lcd_data[0] = 0x0;
    lcd_data[1]=0x01;           //0x01 clear disp
    i2c.write(adr, lcd_data, 2);
    thread_sleep_for(20);
    lcd_data[1]=0x02;           //0x02 return home
    i2c.write(adr, lcd_data, 2);
    thread_sleep_for(20);
    lcd_data[1]=0x0C;           //0x0c disp on
    i2c.write(adr, lcd_data, 2);
    thread_sleep_for(20);
    lcd_data[1]=0x01;           //0x01 clear disp
    i2c.write(adr, lcd_data, 2);
    thread_sleep_for(20);
}

void char_disp(uint8_t adr, int8_t position, char data){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]=0x40;            //ahr disp cmd
    buf[1]=data;
    i2c.write(adr,buf, 2);
}

//disp val func
void val_disp(uint8_t adr, int8_t position, int8_t val){
    char buf[2];
    char data[2];
    int8_t i;
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]=0x40; 
    if(val<0){
        buf[1]='-';
    }
    else if(val>=0){
        buf[1]='+';
    }
    i2c.write(adr,buf, 2);
    data[1]=0x30+abs(val)%10;        //1
    data[0]=0x30+(abs(val)/10)%10;   //10
    buf[0]=0x40;
    buf[1]=data[0];
    i2c.write(adr,buf, 2);
    buf[1]=data[1];
    i2c.write(adr,buf, 2);
}

void cont(uint8_t adr,uint8_t val){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x2a;
    i2c.write(adr,buf,2);
    buf[1]=0x79;    //SD=1
    i2c.write(adr,buf,2);
    buf[1]=0x81;    //contrast set
    i2c.write(adr,buf,2);
    buf[1]=val;    //contrast value
    i2c.write(adr,buf,2);
    buf[1]=0x78;    //SD=0
    i2c.write(adr,buf,2);
    buf[1]=0x28;    //0x2C, 0x28
    i2c.write(adr,buf,2);
}