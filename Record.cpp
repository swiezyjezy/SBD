#include "Record.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>

Record::Record()
{
	//Rec_Len = 1 + rand() % MAX_REC_LEN; // losowanie dlugosci rekordu

	//for (int i = 0; i < MAX_REC_LEN; i++)
	//{
	//	arr[i] = 0;
	//}

	//for (int i = 0; i < Rec_Len; i++)
	//{
	//	char digit = rand() % 10 + '0'; // losujemy cyfre i zapisujemy jako char
	//	arr[i] = digit;
	//}
}

Record::Record(int k, const char* data, int p)
{
	key = k;
	for (int i = 0; i < MAX_REC_LEN; i++)
	{
		arr[i] = data[i];
	}
	pointer = p;
}

int Record::getRec_Len()
{
	return 0;
}

// zwraca chara znajdujacego sie w tablicy pod danym indeksem
char Record::getRecordArr(int index)
{
	return arr[index];
}

int Record::getKey()
{
	return key;
}

int Record::getPointer()
{
	return pointer;
}

char* Record::getArray()
{
	return arr;
}

char Record::getArrayValue(int index)
{
	return arr[index];
}

void Record::markRecord()
{
	if (arr[0] >= '0' && arr[0] <= '9')
	{
		arr[0] = arr[0] + 10;
	}
}

bool Record::isMarekd()
{
	if (arr[0] > '9')
	{
		return true;
	}
	else return false;
}

void Record::setKey(int k)
{
	key = k;
}

void Record::setArr(const char* a)
{
	for (int i = 0; i < MAX_REC_LEN; i++)
	{
		if (a[i] > 0)
		{
			arr[i] = a[i];
		}
		else break;
	}
}

void Record::setPointer(int p)
{
	pointer = p;
}

void Record::clearArr()
{
	for (int i = 0; i < MAX_REC_LEN; i++)
	{
		arr[i] = 0;
	}
}

void Record::setArrValue(int index, char character)
{
	arr[index] = character;
}
