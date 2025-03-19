#include "Index.h"

Index::Index()
{
	key = -1;
	pageNumber = -1;
}

Index::Index(int k, int pN)
{
	key = k;
	pageNumber = pN;
}

int Index::getKey()
{
	return key;
}

int Index::getPageNumber()
{
	return pageNumber;
}

void Index::setKey(int k)
{
	key = k;
}

void Index::setPageNumber(int PN)
{
	pageNumber = PN;
}
