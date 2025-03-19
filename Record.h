#pragma once
#include<iostream>
#pragma pack(push, 1)
#define MAX_REC_LEN 30
#define RECORD_SIZE 38

class Record
{
private:
	int key = -1;
	char arr[MAX_REC_LEN] = {};
	int pointer = -1;

public:
	Record();
	Record(int key, const char* data, int pointer); // rekord wczytany z klawiatury
	int getRec_Len();
	char getRecordArr(int index);
	int getKey();
	int getPointer();
	char* getArray();
	char getArrayValue(int index);
	void markRecord();
	bool isMarekd();
	void setKey(int key);
	void setArr(const char* arr);
	void setPointer(int pointer);
	void clearArr();
	void setArrValue(int index, char character);
};

#pragma pack(pop)