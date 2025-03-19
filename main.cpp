#define _CRT_SECURE_NO_WARNINGS
#define PAGE_SIZE 152
#define OVERFLOW_MAX_ELEMENTS 4

#include<iostream>
#include<string>
#include<stdio.h>
#include <fstream>
#include <filesystem>
#include <stdlib.h>

#include"Index.h"
#include"Record.h"

int pagesNumber = 0;
int recordsInPA = 0;
int recordsInOA = 0;

int readNumber;
int writeNumber;

void reorganizeFiles(float alpha, Record* overflow);

int findPage(int key)
{
	FILE* file = fopen("indexFile.txt", "rb");

	int indexNumer = PAGE_SIZE / INDEX_SIZE;

	Index* indexReadPage = new Index[indexNumer];

	int indicesRead;
	int page = 0;

	fseek(file, 0, SEEK_END);
	bool isEmpty = ftell(file) == 0;
	if (isEmpty)
	{
		delete[] indexReadPage;
		return -1;
	}

	fseek(file, 0, SEEK_SET);

	while (1)
	{
		indicesRead = fread(indexReadPage, sizeof(Index), indexNumer, file);
		readNumber++;
		{
			if (indicesRead == 0)
			{
				delete[] indexReadPage;
				return page;
			}
			else
			{
				for (int i = 0; i < PAGE_SIZE/INDEX_SIZE; i++)
				{
					if (indexReadPage[i].getKey() < 0)
					{
						delete[] indexReadPage;
						return page;
					}

					if (key >= indexReadPage[i].getKey())
					{
						page++;
					}
					else
					{
						delete[] indexReadPage;
						return page;
					}
				}
			}

		}
	}
	delete[] indexReadPage;
	return 0;
}

void printOverFlowArea(Record* overflow)
{
	std::cout << "\n-----------Overflow Area-------------\n";
	for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
	{
		if (overflow[i].getKey() >= 0)
		{
			if(overflow[i].isMarekd())
			{
				std::cout << "Record Deleted: ";
			} 
			std::cout << "Key: " << overflow[i].getKey()
				<< ", Data: " << overflow[i].getArray()
				<< ", Pointer: " << overflow[i].getPointer() << std::endl;
		}
	}
	std::cout << "\n";
}

void printRecordFile()
{
	FILE* records = fopen("recordFile.txt", "rb");

	int maxRecordsPerPage = PAGE_SIZE / RECORD_SIZE;

	Record* buffer = new Record[maxRecordsPerPage];

	int page = 0;

	std::cout << "\n---------Primary Area---------";
	while (true)
	{
		int recordsRead = fread(buffer, sizeof(Record), maxRecordsPerPage, records);
		readNumber++;
		if (recordsRead == 0)
		{
			break;
		}

		std::cout << "\nPage: " << ++page << std::endl;
		for (int i = 0; i < maxRecordsPerPage; i++)
		{
			if (buffer[i].getKey() < 0)
			{
				std::cout << "[EMPTY SPACE]" << std::endl;
				continue;
			}
			if (buffer[i].isMarekd())
			{
				std::cout << "Record Deleted: ";
			}
			std::cout << "Key: " << buffer[i].getKey()
				<< ", Data: " << buffer[i].getArray()
				<< ", Pointer: " << buffer[i].getPointer() << std::endl;
		}
		std::cout << "-----------------------------------" << std::endl;
	}

	delete[] buffer;
	fclose(records);
}

void printIndexFile()
{
	FILE* indices = fopen("indexFile.txt", "rb");

	int maxIndicesPerPage = PAGE_SIZE / INDEX_SIZE;

	Index* buffer = new Index[maxIndicesPerPage];

	int page = 0;

	std::cout << "\n---------Indices---------";
	while (true)
	{
		int indicesRead = fread(buffer, sizeof(Index), maxIndicesPerPage, indices);
		readNumber++;
		if (indicesRead == 0)
		{
			break;
		}
		for (int i = 0; i < maxIndicesPerPage; i++)
		{
			if (buffer[i].getKey() < 0)
			{
				std::cout << "\n[EMPTY SPACE]";
				continue;
			}
			std::cout << "\nKey: " << buffer[i].getKey()
				<< ", Page: " << buffer[i].getPageNumber();
		}
		std::cout << "\n-----------------------------------" << std::endl;
	}

	delete[] buffer;
	fclose(indices);
}

void addRecord(int key, const char* data, Record* overflow, float alpha)
{
	int deltaWrite = writeNumber;
	int deltaRead = readNumber;
	for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
	{
		if (overflow[i].getKey() == key)
		{
			std::cout << "\njuz istnieje rekord z tym kluczem\n";
			return;
		}
	}

	int page = findPage(key);
	FILE* records = fopen("recordFile.txt", "rb+");

	// nie ma rekordow w pliku 
	if (page == -1)
	{
		FILE* index = fopen("indexFile.txt", "rb+");

		fwrite(&key, sizeof(int), 1, records);
		fwrite(data, sizeof(char), 30, records);
		int pointer = -1;
		fwrite(&pointer, sizeof(int), 1, records);
		writeNumber++;
		recordsInPA++;

		int indexRecord[2] = { key,1 };
		fwrite(indexRecord, sizeof(int), 2, index);
		writeNumber++;
		pagesNumber++;

		fclose(index);
	}
	else
	{
		if (page == 0)
		{
			page = 1;
		}
		//ustaw kursor na odpowniedniej stronie i przeczytaj strone
		fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);

		int maxRecordsNumber = PAGE_SIZE / RECORD_SIZE;

		Record* recordsOnPage = new Record[maxRecordsNumber];

		fread(recordsOnPage, RECORD_SIZE, maxRecordsNumber, records);
		readNumber++;

		for (int i = 0; i < maxRecordsNumber; i++)
		{
			if (recordsOnPage[i].getKey() == key)
			{
				std::cout << "\n juz istnieje rekord z podanym kluczem\n";
				fclose(records);
				std::cout << "\n wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
				delete[] recordsOnPage;
				return;
			}

		}
		
		bool isKeyBiggest = true;
		for (int i = 0; i < maxRecordsNumber; i++)
		{
			if (recordsOnPage[i].getKey() > key)
			{
				isKeyBiggest = false;
			}

		}

		//czy jest na stronie miejsce dla rekordu  i nie ma wiekszego klucza na stronie
		if (recordsOnPage[maxRecordsNumber - 1].getKey() < 0 && isKeyBiggest)
		{
			//znajdujemy pierwsze wolne miejsce i dodajemy rekord
			for (int i = 0; i < maxRecordsNumber; i++)
			{
				if (recordsOnPage[i].getKey() < 0)
				{
					recordsOnPage[i].setKey(key);
					recordsOnPage[i].setArr(data);
					recordsOnPage[i].setPointer(-1);

					fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
					fwrite(recordsOnPage, RECORD_SIZE, maxRecordsNumber, records);
					writeNumber++;
					recordsInPA++;
					break;
				}
			}
		}
		//dodajemy do overflow
		else
		{

			//znajdujemy poprzedni rekord w PA
			int prevRecordIndex = -1;

			for (int i = 0; i < maxRecordsNumber; i++)
			{
				if (recordsOnPage[i].getKey() >= 0)
				{
					if (recordsOnPage[i].getKey() < key)
					{
						prevRecordIndex++;
					}
				}
			}

			int OFIndex = 0;

			// znajdz indeks wolnego miejca w overflow area
			for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
			{
				if (overflow[i].getKey() >= 0)
				{
					OFIndex++;
				}
				else break;
			}

			//jezeli nie ma poprzedniego rekordu w PA (rekord mniejszy niz rekordy w PA)
			if (prevRecordIndex == -1)
			{
				// dodajemy rekord do OF
				Record newRecord(key, data, -1);
				overflow[OFIndex] = newRecord;
				recordsInOA++;

				int prevKey = -1;
				int prevIndex = -1;

				// jezeli istnieje mniejszy rekord w OF ustawiamy jego pointer na nowy rekord
				for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
				{
					if (overflow[i].getKey() < key && overflow[i].getKey() > prevKey)
					{
						prevKey = overflow[i].getKey();
						prevIndex = i;
					}
				}
				if (prevIndex != -1)
				{
					overflow[prevIndex].setPointer(OFIndex);
				}

				// jezeli istnieje rekord wiekszy od key i mniejszy od najmniejszego rekordu z PA
				int nextKey = INT_MAX;
				int nextIndex = -1;
				for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
				{
					if (overflow[i].getKey() > key && overflow[i].getKey() < recordsOnPage[0].getKey() && overflow[i].getKey() < nextKey)
					{
						nextKey = overflow[i].getKey();
						nextIndex = i;
					}
				}
				if (nextKey != -1)
				{
					overflow[OFIndex].setPointer(nextIndex);
				}

				bool reorganize = true;
				for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
				{
					if (overflow[i].getKey() < 0)
					{
						reorganize = false;
					}
				}
				if (reorganize)
				{
					int rDelta = readNumber - deltaRead;
					int wDelta = writeNumber - deltaWrite;
					fclose(records);
					reorganizeFiles(alpha, overflow);			
					std::cout << "\n wykonano " << wDelta << " operacji zapisu i " <<  rDelta << " operacji czytania \n";
					delete[] recordsOnPage;
					return;
				}
				fclose(records);
				std::cout << "\n wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
				delete[] recordsOnPage;
				return;
			}

			// jezeli poprzedni rekord nie ma wzkaznika to dodajemy rekord to OF i ustawiamy pointer 
			if (recordsOnPage[prevRecordIndex].getPointer() == -1)
			{
				// dodajemy rekord do OF
				Record newRecord(key, data, -1);
				overflow[OFIndex] = newRecord;

				// ustawiamy pointer z PA na dodany rekord
				recordsOnPage[prevRecordIndex].setPointer(OFIndex);

				//zapisujemy zmodyfikowana strone do PA
				fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
				fwrite(recordsOnPage, RECORD_SIZE, maxRecordsNumber, records);
				writeNumber++;
				recordsInOA++;
			}
			// jezeli jest wskaznik 
			else
			{
				// jezeli poprzedni rekord wskazuje na element mniejszy niz obecnie dodawany rekord
				if (overflow[recordsOnPage[prevRecordIndex].getPointer()].getKey() < key)
				{
					// szukamy w OF najwiêkszego z mniejszych elementów
					int maxPrevKey = -1;
					int maxPrevIndex = 0;
					for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
					{
						if (overflow[i].getKey() < key && overflow[i].getKey() > maxPrevKey)
						{
							maxPrevIndex = i;
							maxPrevKey = overflow[i].getKey();
						}
					}

					//dodajemy nowy rekord do OF
					Record newRecord(key, data, -1);
					overflow[OFIndex] = newRecord;

					int prevPointer = -1;
					prevPointer = overflow[maxPrevIndex].getPointer();

					// ustawiamy pointer z OF na dodany rekord
					overflow[maxPrevIndex].setPointer(OFIndex);
					overflow[OFIndex].setPointer(prevPointer);

					// ustawiamy nowemu rekordowi pointer poprzedniego rekordu

					recordsInOA++;
				}
				// jezeli poprzedni rekord wskazuje na element wiekszy niz obecnie dodawany rekord
				else
				{
					//dodajemy nowy rekord do OF
					Record newRecord(key, data, -1);
					overflow[OFIndex] = newRecord;

					//ustawiamy wskazznik nowego rekordu za wiekszy element z OF
					overflow[OFIndex].setPointer(recordsOnPage[prevRecordIndex].getPointer());

					//ustawiamy wskaznik z PA na nowo dodany rekord
					recordsOnPage[prevRecordIndex].setPointer(OFIndex);

					//zapisujemy zmodyfikowana strone do PA
					fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
					fwrite(recordsOnPage, RECORD_SIZE, maxRecordsNumber, records);
					writeNumber++;
					recordsInOA++;
				}
			}

			// jezeli nie ma miejsca w OA reorganizacja
			bool reorganize = true;
			for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
			{
				if (overflow[i].getKey() < 0)
				{
					reorganize = false;
				}
			}
			if (reorganize)
			{
				int rDelta = readNumber - deltaRead;
				int wDelta = writeNumber - deltaWrite;
				fclose(records);
				reorganizeFiles(alpha, overflow);
				std::cout << "\n wykonano " << wDelta << " operacji zapisu i " << rDelta << " operacji czytania \n";
				delete[] recordsOnPage;
				return;
			}
		}
	}
	fclose(records);
	std::cout << "\n wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
}

Record getNextRec(Record* overflow, Record* readBuffer, int* currentRecord, bool* inPA, int* currentOARecordIndex)
{
	bool smallestInPA = true;
	// jezeli to pierwszy rekord w danej reorganizacji sprawdzamy czy najmniejszy rekord jest w PA czy OA
	if (*currentRecord == -1)
	{
		for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
		{
			if (overflow[i].getKey() >= 0)
			{
				if (readBuffer[0].getKey() > overflow[i].getKey())
				{
					smallestInPA = false;
				}
			}
		}
		if (smallestInPA)
		{
			*currentRecord = 1;
			return readBuffer[0];

		}
		else
		{
			int smallestRecIndex = -1;
			int smallestRecKey = INT_MAX;
			// szukamy najmniejszego rekordu z OA i zwracamy go
			for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
			{
				if (overflow[i].getKey() < smallestRecKey)
				{
					smallestRecIndex = i;
					smallestRecKey = overflow[i].getKey();
				}
			}
			// jezeli rekord ten ma wskaznik
			if (overflow[smallestRecIndex].getPointer() >= 0)
			{
				*currentOARecordIndex = overflow[smallestRecIndex].getPointer();
				*currentRecord = 0;
				*inPA = false;
			}
			else
			{
				*inPA = true;
				*currentRecord = 0;
			}
			return overflow[smallestRecIndex];
		}
	}
	else
	{
		if (*inPA == true)
		{
			// jezeli rekord ma wskaznik na OA
			if (readBuffer[*currentRecord].getPointer() >= 0)
			{
				*currentRecord += 1;
				*inPA = false;
				*currentOARecordIndex = readBuffer[*currentRecord - 1].getPointer();
				return readBuffer[*currentRecord - 1];
			}
			else
			{
				*currentRecord += 1;
				return readBuffer[*currentRecord - 1];
			}
		}
		// rekord jest w OA
		else
		{
			if (overflow[*currentOARecordIndex].getPointer() >= 0)
			{
				int oldRecIndex = *currentOARecordIndex;
				*currentOARecordIndex = overflow[*currentOARecordIndex].getPointer();
				return overflow[oldRecIndex];
			}
			else
			{
				*inPA = true;
				return overflow[*currentOARecordIndex];
			}
		}
	}
}

void deleteRecord(int key, Record* overflow)
{
	int deltaRead = readNumber;
	int deltaWrite = writeNumber;

	// szukamy rekordu w OA
	for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
	{
		if (key == overflow[i].getKey())
		{
			overflow[i].markRecord();
			std::cout << "\n nie wykonano operacji zapisu ani czytania\n";
			return;
		}
	}

	int page = findPage(key);
	FILE* records = fopen("recordFile.txt", "rb+");
	Record* readBuffer = new Record[PAGE_SIZE / RECORD_SIZE];

	fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
	fread(readBuffer, RECORD_SIZE, PAGE_SIZE / RECORD_SIZE, records);
	readNumber++;

	for (int i = 0; i < PAGE_SIZE / RECORD_SIZE; i++)
	{
		if (readBuffer[i].getKey() == key)
		{
			readBuffer[i].markRecord();
			fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
			fwrite(readBuffer, RECORD_SIZE, PAGE_SIZE / RECORD_SIZE, records);
			writeNumber++;
			fclose(records);
			std::cout << "\n wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
			delete[] readBuffer;
			return;
		}
	}
}

void modifyRecordKey(Record* overflow, int newKey, int key, float alpha)
{
	char* arr;
	// szukamy rekordu w OA
	int deltaRead = readNumber;
	int deltaWrite = writeNumber;
	
	for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
	{
		if (key == overflow[i].getKey())
		{
			arr = overflow[i].getArray();
			deleteRecord(key, overflow);
			addRecord(newKey, arr, overflow, alpha);
			std::cout << "\n nie wykonano operacji zapisu ani czytania\n";
			return;
		}
	}

	int page = findPage(key);
	FILE* records = fopen("recordFile.txt", "rb+");
	Record* readBuffer = new Record[PAGE_SIZE / RECORD_SIZE];

	fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
	fread(readBuffer, RECORD_SIZE, PAGE_SIZE / RECORD_SIZE, records);
	readNumber++;

	fclose(records);

	for (int i = 0; i < PAGE_SIZE / RECORD_SIZE; i++)
	{
		if (readBuffer[i].getKey() == key)
		{
			arr = readBuffer[i].getArray();
			deleteRecord(key, overflow);
			addRecord(newKey, arr, overflow, alpha);
			std::cout << "\n wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
			delete[] readBuffer;
			return;
		}
	}
}

void modifyRecordArr( Record* overflow, const char* newData, int key)
{
	int deltaRead = readNumber;
	int deltaWrite = writeNumber;
	// szukamy rekordu w OA
	for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
	{
		if (key == overflow[i].getKey())
		{
			overflow[i].clearArr();
			for (int j = 0; j < MAX_REC_LEN; j++)
			{
				if (newData[j] >= '0' && newData[j] <= '9')
				{
					overflow[i].setArrValue(j, newData[j]);
				}
			}
			std::cout << "\n nie wykonano operacji zapisu ani czytania\n";
			return;
		}
	}

	int page = findPage(key);
	FILE* records = fopen("recordFile.txt", "rb+");
	Record* readBuffer = new Record[PAGE_SIZE / RECORD_SIZE];

	fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
	fread(readBuffer, RECORD_SIZE, PAGE_SIZE / RECORD_SIZE, records);
	readNumber++;
	
	for (int i = 0; i < PAGE_SIZE / RECORD_SIZE; i++)
	{
		if (readBuffer[i].getKey() == key)
		{
			readBuffer[i].clearArr();
			for (int j = 0; j < MAX_REC_LEN; j++)
			{
				if (newData[j] >= '0' && newData[j] <= '9')
				{
					readBuffer[i].setArrValue(j, newData[j]);
				}
			}
			fseek(records, PAGE_SIZE * (page - 1), SEEK_SET);
			fwrite(readBuffer, RECORD_SIZE, PAGE_SIZE / RECORD_SIZE, records);
			writeNumber++;
			fclose(records);
			std::cout << "\n wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
			delete[] readBuffer;
			return;
		}
	}
}

void reorganizeFiles(float alpha, Record* overflow)
{
	int deltaRead = readNumber;
	int deltaWrite = writeNumber;

	int bf = PAGE_SIZE / RECORD_SIZE;
	int bi = PAGE_SIZE / INDEX_SIZE;


	FILE* newRecordFile = fopen("newRecordFile.txt", "wb+");

	// Bufor do odczytu
	int maxRecordsPerPage = PAGE_SIZE / RECORD_SIZE;

	int pageCounter = 0;

	FILE* oldRecordFile = fopen("recordFile.txt", "rb");
	FILE* indexFile = fopen("indexFile.txt", "wb");

	Record* readBuffer = new Record[maxRecordsPerPage];
	Record* writeBuffer = new Record[maxRecordsPerPage];
	Index* newIndexBuffer = new Index[bi];

	bool smallestInPA = true;

	bool firstPage = true;
	int currentRecordIndex = -1;
	int currentOARecordIndex = 0;

	bool inPA = true;
	// czytamy pierwszy blok i sprawdzamy czy najmniejszy rekord jest w PA czy w OA
	int recordsRead = fread(readBuffer, sizeof(Record), maxRecordsPerPage, oldRecordFile);
	readNumber++;

	int RecordsProccessed = 0;
	int RecordsInWriteBuffer = 0;

	int indexFileIndex = 0;
	int indexFilePages = 0;

	bool firstRec = true;

	while (true)
	{
		Record currentRec = getNextRec(overflow, readBuffer, &currentRecordIndex, &inPA, &currentOARecordIndex);
		if (currentRec.getKey() < 0)
		{
			recordsRead = fread(readBuffer, sizeof(Record), maxRecordsPerPage, oldRecordFile);
			readNumber++;
			currentRecordIndex = 0;
			continue;
		}
		currentRec.setPointer(-1);
		if (currentRec.isMarekd())
		{
			RecordsProccessed++;
			continue;
		}
		if (currentRecordIndex >= maxRecordsPerPage)
		{
			recordsRead = fread(readBuffer, sizeof(Record), maxRecordsPerPage, oldRecordFile);
			readNumber++;
			currentRecordIndex = 0;
		}
		RecordsProccessed++;
		if (firstRec)
		{
			writeBuffer[RecordsInWriteBuffer] = currentRec;
			RecordsInWriteBuffer++;

			// dodajemy nowy indeks
			newIndexBuffer[indexFileIndex].setKey(currentRec.getKey());
			newIndexBuffer[indexFileIndex].setPageNumber(indexFileIndex + 1);

			indexFileIndex++;
			if (indexFileIndex >= PAGE_SIZE / INDEX_SIZE)
			{
				fwrite(newIndexBuffer, INDEX_SIZE, PAGE_SIZE / INDEX_SIZE, indexFile);
				writeNumber++;
				for (int i = 0; i < PAGE_SIZE / INDEX_SIZE; i++)
				{
					newIndexBuffer[i] = Index();
				}
				indexFileIndex = 0;
			}
			firstRec = false;
		}
		else
		{
			// jezeli w writeBuffer jest mniej rekordow niz bf*alpha
			if (RecordsInWriteBuffer < bf * alpha)
			{
				writeBuffer[RecordsInWriteBuffer] = currentRec;
				RecordsInWriteBuffer++;
			}
			else
			{
				// zapisujemy writebuffer do pliku
				fwrite(writeBuffer, RECORD_SIZE, maxRecordsPerPage, newRecordFile);
				writeNumber++;
				// dodajemy nowy indeks
				newIndexBuffer[indexFileIndex].setKey(currentRec.getKey());
				newIndexBuffer[indexFileIndex].setPageNumber((PAGE_SIZE/INDEX_SIZE)*indexFilePages +  indexFileIndex + 1);
				// czyscimy writebuffer dodajemy do niego element
				for (int i = 0; i < maxRecordsPerPage; i++)
				{
					writeBuffer[i] = Record();
				}
				writeBuffer[0] = currentRec;
				RecordsInWriteBuffer = 1;
				indexFileIndex++;
				if (indexFileIndex >= PAGE_SIZE / INDEX_SIZE)
				{
					fwrite(newIndexBuffer, INDEX_SIZE, PAGE_SIZE / INDEX_SIZE, indexFile);
					writeNumber++;
					for (int i = 0; i < PAGE_SIZE / INDEX_SIZE; i++)
					{
						newIndexBuffer[i] = Index();
					}
					indexFileIndex = 0;
					indexFilePages++;
				}
			}
		}


		if (RecordsProccessed == recordsInOA + recordsInPA)
		{
			// zapisujemy resztki buffera do pliku
			fwrite(writeBuffer, RECORD_SIZE, maxRecordsPerPage, newRecordFile);
			fwrite(newIndexBuffer, INDEX_SIZE, PAGE_SIZE / INDEX_SIZE, indexFile);
			writeNumber++;
			writeNumber++;
			break;
		}
	}

	//czyscimy overflow
	for (int i = 0; i < OVERFLOW_MAX_ELEMENTS; i++)
	{
		overflow[i] = Record();
	}

	fclose(newRecordFile);
	fclose(oldRecordFile);
	fclose(indexFile);

	remove("recordFile.txt");
	rename("newRecordFile.txt", "recordFile.txt");

	std::cout<< "\n wykonano reorganizacje";
	std::cout << " wykonano " << writeNumber - deltaWrite << " operacji zapisu i " << readNumber - deltaRead << " operacji czytania \n";
	delete[] readBuffer;
	delete[] writeBuffer;
	delete[] newIndexBuffer;
}

void processProgramFile(Record* overflow, float alpha)
{
	FILE* programFile = fopen("program.txt", "r");
	if (programFile == nullptr)
	{
		printf("Nie mozna otworzyc pliku program.txt\n");
		return;
	}

	char line[256];
	char command;
	int key;
	char data[31];
	int tempNewKey = 0;

	while (fgets(line, sizeof(line), programFile) != nullptr)
	{
		command = '\0';
		key = 0;
		memset(data, 0, sizeof(data));

		if (sscanf(line, " %c %d %30[^\n]", &command, &key, data) == 3)
		{
			if (command == 'a')
			{
				printf("\nDodawanie rekordu: Klucz=%d, Data=%s", key, data);
				addRecord(key, data, overflow, alpha);
			}
			else if (command == 'm')
			{
				for (int i = 0; data[i] != '\0'; i++)
				{
					if (data[i] >= '0' && data[i] <= '9')
					{
						tempNewKey = tempNewKey * 10 + (data[i] - '0');
					}
				}
				printf("\nModyfikacja klucza: %d na nowa wartosc: %d", key, tempNewKey);
				modifyRecordKey(overflow, tempNewKey, key, alpha);
				tempNewKey = 0;
			}
			else if (command == 'M')
			{
				printf("\nModyfikacja danych rekordu: %d na nowa wartosc: %s", key, data);
				modifyRecordArr(overflow, data, key);
			}
		}
		else if (sscanf(line, " %c %d", &command, &key) == 2)
		{
			if (command == 'd')
			{
				printf("\nUsuwanie rekordu: Klucz=%d", key);
				deleteRecord(key, overflow);
			}
		}
		else if (sscanf(line, " %c", &command) == 1)
		{
			if (command == 'r')
			{
				printf("\nReorganizacja plikow");
				reorganizeFiles(alpha, overflow);
			}
			else if (command == 'p')
			{
				printf("\nWyswietlanie plikow");
				printIndexFile();
				printRecordFile();
				printOverFlowArea(overflow);
			}
		}
		else
		{
			printf("\nNieznana komenda lub nieprawidlowy format linii: %s", line);
		}
	}

	fclose(programFile);
}

int main()
{
	float alpha = 0.75;

	srand(time(NULL));

	FILE* recordFile = fopen("recordFile.txt", "wb+");
	FILE* indexFile = fopen("indexFile.txt", "wb+");

	fclose(recordFile);
	fclose(indexFile);

	Record* overflowArea = new Record[OVERFLOW_MAX_ELEMENTS];

	int pagesNumber = 0;

	int tempKey;
	int tempNewKey;
	std::string recordContent;

	std::string operation = "";
	char option = 0;
	char printFiles = 0;

	std::cout << "\nwczytac instrukcje z pliku [1] czy z klawiatury [2]\n";
	std::cin >> option;
	if (option == '1')
	{
		processProgramFile(overflowArea, alpha);
		return 0;
	}
	else
	{
		while (1)
		{
			std::cout << "\nwybierz operacje [a] - add [mk] - modyfikacja klucza"
				<< "[md] - modyfikacja danych [r] - reorganizacja [d] - usuwanie rekordu [e] - exit\n";
			std::cin >> operation;
			if (operation == "a")
			{
				std::cout << "\npodaj klucz\n";
				std::cin >> tempKey;
				std::cout << "\npodaj dane\n";
				std::cin >> recordContent;
				addRecord(tempKey, recordContent.c_str(), overflowArea, alpha);
				std::cout << "\nwyswietlic zawartosc plikow [Y/N]\n";
				std::cin >> printFiles;
				if (printFiles == 'Y' || printFiles == 'y')
				{
					printIndexFile();
					printRecordFile();
					printOverFlowArea(overflowArea);
				}
				tempKey = -1;
				recordContent = "";
			}
			else if (operation == "mk")
			{
				std::cout << "\npodaj klucz modyfikowanego rekordu\n";
				std::cin >> tempKey;
				std::cout << "\npodaj nowy klucz\n";
				std::cin >> tempNewKey;
				modifyRecordKey(overflowArea, tempNewKey, tempKey, alpha);
				std::cout << "\nwyswietlic zawartosc plikow [Y/N]\n";
				std::cin >> printFiles;
				if (printFiles == 'Y' || printFiles == 'y')
				{
					printIndexFile();
					printRecordFile();
					printOverFlowArea(overflowArea);
				}
				tempKey = -1;
				tempNewKey = -1;
			}
			else if (operation == "md")
			{
				std::cout << "\npodaj klucz modyfikowanego rekordu\n";
				std::cin >> tempKey;
				recordContent = "";
				std::cout << "\npodaj nowe dane\n";
				std::cin >> recordContent;
				modifyRecordArr(overflowArea, recordContent.c_str(), tempKey);
				std::cout << "\nwyswietlic zawartosc plikow [Y/N]\n";
				std::cin >> printFiles;
				if (printFiles == 'Y' || printFiles == 'y')
				{
					printIndexFile();
					printRecordFile();
					printOverFlowArea(overflowArea);
				}
				tempKey = -1;
				recordContent = "";
			}
			else if (operation == "r")
			{
				reorganizeFiles(alpha, overflowArea);
				std::cout << "\nwyswietlic zawartosc plikow [Y/N]\n";
				std::cin >> printFiles;
				if (printFiles == 'Y' || printFiles == 'y')
				{
					printIndexFile();
					printRecordFile();
					printOverFlowArea(overflowArea);
				}
			}
			else if (operation == "d")
			{
				std::cout << "\npodaj klucz usuwanego rekordu\n";
				std::cin >> tempKey;
				deleteRecord(tempKey, overflowArea);
				std::cout << "\nwyswietlic zawartosc plikow [Y/N]\n";
				std::cin >> printFiles;
				if (printFiles == 'Y' || printFiles == 'y')
				{
					printIndexFile();
					printRecordFile();
					printOverFlowArea(overflowArea);
				}
				tempKey = -1;
			}
			else if (operation == "e")
			{
				break;
			}
			else
			{
				std::cout << "\n nie rozpoznano polecenia \n";
			}
		}

	}
	return 0;
}