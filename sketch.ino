/*

Protocol structure:

1. uint16_t command
2. (uint8_t-uint64_t values)

Wiring used in this sketch:

Digital port 2: green LED
Digital port 3: red LED

*/

#define fs_t uint32_t
#define extendBufferSize(bufferSize,pos) while(pos>=bufferSize) bufferSize*=2

#include "commands.h"
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <stdint.h>
#include <string.h>

YunServer server(COMM_PORT);

char *buff;
uint32_t buffSize;
uint32_t buffPos;
uint16_t command;

void setup() {
  command=0;
  buffSize=1024;
  buffPos=0;
  buff=(char*)malloc(buffSize);
  
  pinMode(13,OUTPUT);
 
  Serial.begin(9600);
  Bridge.begin();
 
  server.begin();
  delay(2000);
}

void loop() {
  YunClient client=server.accept();
  if(client)
  {
    while(client.connected())
    {
      if(client.available())
      { 
        char c=client.read();
        writeRawCharToBuffer(buff,c,buffPos,buffSize);
        if(buffPos>=2)
        {
          if(command==0)
          {
            command=peekUInt16(buff,0);
            if(command==CMD_NOP) // 0
              buffPos=0;
          }
          else
          {
            // If length satisfies required length of command, act. Always set buffPos and command to 0 afterwards.
            if(buffPos==6)
            {
              if(command==CMD_HIGH)
              {
                digitalWrite(peekUInt32(buff,2),HIGH);
                buffPos=0;
                command=0;
              }
              else if(command==CMD_LOW)
              {
                digitalWrite(peekUInt32(buff,2),LOW);
                buffPos=0;
                command=0;
              }
              else if(command==CMD_TOGGLE)
              {
                uint32_t pin=peekUInt32(buff,2);
                digitalWrite(pin,!digitalRead(pin));
                buffPos=0;
                command=0;
              }
              else if(command==CMD_DELAY)
              {
                delay(peekUInt32(buff,2));
                buffPos=0;
                command=0;
              }
              else if(command==CMD_GET_ANALOG)
              {
                uint32_t sensor=peekUInt32(buff,2);
                uint32_t value=analogRead(sensor);
                uint32_t pos=0;
                uint32_t bufferSize=sizeof(uint16_t)+2*sizeof(uint32_t);
                char *out=(char*)malloc(bufferSize);
                writeUInt16ToBuffer(out,CMD_GET_ANALOG/*Response*/,pos,bufferSize);
                writeUInt32ToBuffer(out,sensor/*Sensor id*/,pos,bufferSize);
                writeUInt32ToBuffer(out,value,pos,bufferSize);
                client.write((uint8_t*)out,pos);
                buffPos=0;
                command=0;
              }
              else if(command==CMD_GET_DIGITAL)
              {
                uint32_t sensor=peekUInt32(buff,2);
                uint8_t value=digitalRead(sensor);
                uint32_t pos=0;
                uint32_t bufferSize=sizeof(uint16_t)+sizeof(uint32_t)+sizeof(uint8_t);
                char *out=(char*)malloc(bufferSize);
                writeUInt16ToBuffer(out,CMD_GET_DIGITAL/*Response*/,pos,bufferSize);
                writeUInt32ToBuffer(out,sensor/*Sensor id*/,pos,bufferSize);
                writeUInt8ToBuffer(out,value,pos,bufferSize);
                
                client.write((uint8_t*)out,pos);
                buffPos=0;
                command=0;
              }
            }
            else if(buffPos==7)
            {
              if(command==CMD_MODE)
              {
                pinMode(peekUInt32(buff,2),peekUInt8(buff,6)==CMD_MODE_IN?INPUT:OUTPUT);
                buffPos=0;
                command=0;
              }
              else if(command==CMD_SET_DIGITAL)
              {
                digitalWrite(peekUInt32(buff,2),peekUInt8(buff,6));
                buffPos=0;
                command=0;
              }
            }
            else if(buffPos==10)
            {
              if(command==CMD_SET_ANALOG)
              {
                analogWrite(peekUInt32(buff,2),peekUInt32(buff,6));
                buffPos=0;
                command=0;
              }
            }
          }
        }
      }
    }
    buffPos=0;
    command=0;
  }
}

uint8_t peekUInt8(char *data, fs_t pos)
{
    return (uint8_t)data[pos++];
}

uint16_t peekUInt16(char *data, fs_t pos)
{
    uint16_t out=data[pos++];
    out|=(uint16_t)data[pos++]<<8;
    return out;
}

uint32_t peekUInt32(char *data, fs_t pos)
{
    uint32_t out=data[pos++];
    out|=(uint32_t)data[pos++]<<8;
    out|=(uint32_t)data[pos++]<<16;
    out|=(uint32_t)data[pos++]<<24;
    return out;
}

uint64_t peekUInt64(char *data, fs_t pos)
{
    uint64_t out=data[pos++];
    out|=(uint64_t)data[pos++]<<8;
    out|=(uint64_t)data[pos++]<<16;
    out|=(uint64_t)data[pos++]<<24;
    out|=(uint64_t)data[pos++]<<32;
    out|=(uint64_t)data[pos++]<<40;
    out|=(uint64_t)data[pos++]<<48;
    out|=(uint64_t)data[pos++]<<56;
    return out;
}

uint32_t peekFsT(char *data, fs_t pos)
{
    return peekUInt32(data,pos);
}

void writeUInt8(char *data, uint8_t i, fs_t &pos)
{
    data[pos++]=i;
}

void writeUInt16(char *data, uint16_t i, fs_t &pos)
{
    data[pos++]=(uint8_t)i;
    data[pos++]=(uint8_t)(i>>8);
}

void writeUInt32(char *data, uint32_t i, fs_t &pos)
{
    data[pos++]=(uint8_t)i;
    data[pos++]=(uint8_t)(i>>8);
    data[pos++]=(uint8_t)(i>>16);
    data[pos++]=(uint8_t)(i>>24);
}

void writeUInt64(char *data, uint64_t i, fs_t &pos)
{
    data[pos++]=(uint8_t)i;
    data[pos++]=(uint8_t)(i>>8);
    data[pos++]=(uint8_t)(i>>16);
    data[pos++]=(uint8_t)(i>>24);
    data[pos++]=(uint8_t)(i>>32);
    data[pos++]=(uint8_t)(i>>40);
    data[pos++]=(uint8_t)(i>>48);
    data[pos++]=(uint8_t)(i>>56);
}

void writeFsT(char *data, fs_t i, fs_t &pos)
{
    writeUInt32(data,i,pos);
}

void writeFixedLengthData(char *data, fs_t length, char *in, fs_t &pos)
{
    writeFsT(data,length,pos);
    for(fs_t i=0;i<length;i++)
        data[pos++]=(uint8_t)in[i];
}

void writeZeroTerminatedData(char *data, char *in, fs_t &pos)
{
    fs_t length=strlen(in);
    for(fs_t i=0;i<length;i++)
        data[pos++]=(uint8_t)in[i];
    data[pos++]=0;
}

void writeRawData(char *data, char *in, fs_t length, fs_t &pos)
{
    for(fs_t i=0;i<length;i++)
        data[pos++]=(uint8_t)in[i];
}

void writeUInt8ToBuffer(char *&data, uint8_t i, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+sizeof(uint8_t);
    bufferCheck(data,newPos,bufferSize);
    writeUInt8(data,i,pos);
}

void writeUInt16ToBuffer(char *&data, uint16_t i, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+sizeof(uint16_t);
    bufferCheck(data,newPos,bufferSize);
    writeUInt16(data,i,pos);
}

void writeUInt32ToBuffer(char *&data, uint32_t i, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+sizeof(uint32_t);
    bufferCheck(data,newPos,bufferSize);
    writeUInt32(data,i,pos);
}

void writeUInt64ToBuffer(char *&data, uint64_t i, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+sizeof(uint64_t);
    bufferCheck(data,newPos,bufferSize);
    writeUInt64(data,i,pos);
}

void writeFsTToBuffer(char *&data, fs_t i, fs_t &pos, fs_t &bufferSize)
{
    writeUInt32ToBuffer(data,i,pos,bufferSize);
}

void writeFixedLengthDataToBuffer(char *&data, fs_t length, char *in, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+length+sizeof(fs_t);
    bufferCheck(data,newPos,bufferSize);
    writeFixedLengthData(data,length,in,pos);
}

void writeZeroTerminatedDataToBuffer(char *&data, char *in, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+strlen(in)+1;
    bufferCheck(data,newPos,bufferSize);
    writeZeroTerminatedData(data,in,pos);
}

void writeRawDataToBuffer(char *&data, char *in, fs_t length, fs_t &pos, fs_t &bufferSize)
{
    fs_t newPos=pos+length;
    bufferCheck(data,newPos,bufferSize);
    writeRawData(data,in,length,pos);
}

void writeRawDataToLongBuffer(char *&data, char *in, uint64_t length, uint64_t &pos, uint64_t &bufferSize)
{
    fs_t newPos=pos+length;
    longBufferCheck(data,newPos,bufferSize);
    memcpy(data+pos,in,length);
    pos+=length;
}

void writeRawCharToBuffer(char *&data, char in, fs_t &pos, fs_t &bufferSize)
{
    if(pos+1==bufferSize) // Because we're only adding one character, == is permissible here.
    {
        extendBufferSize(bufferSize,pos+1);
        data=(char*)realloc(data,bufferSize);
    }
    data[pos++]=in;
}

void terminateBuffer(char *buffer, fs_t &pos, fs_t bufferSize)
{
    bufferCheck(buffer,pos,bufferSize);
    buffer[pos]=0; // Do not use pos++! Many functions use the parameter passed in "pos" as a string length indicator!
}

bool bufferCheck(char *&buffer, fs_t pos, fs_t &bufferSize)
{
    bool ret=true;
    while(pos>=bufferSize) // Needed to check if sth would fit into the buffer.
    {
        extendBufferSize(bufferSize,pos);
        buffer=(char*)realloc(buffer,bufferSize);
        ret=false;
    }
    return ret;
}

bool longBufferCheck(char *&buffer, uint64_t pos, uint64_t &bufferSize)
{
    bool ret=true;
    while(pos>=bufferSize) // Needed to check if sth would fit into the buffer.
    {
        extendBufferSize(bufferSize,pos);
        buffer=(char*)realloc(buffer,bufferSize);
        ret=false;
    }
    return ret;
}
