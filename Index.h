#pragma once
#define INDEX_SIZE 8

class Index
{
private:
	int key = -1;
	int pageNumber = -1;
public:
	Index();
	Index(int key, int pageNumber);
	int getKey();
	int getPageNumber();

	void setKey(int key);
	void setPageNumber(int PageNumber);
};

