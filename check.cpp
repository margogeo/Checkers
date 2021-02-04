//
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "MyDB.h"

#include <iostream>
#include <fstream>
using namespace std;

#define MAX_MOVES 14

struct CMove {
	int iFrom, iTo[MAX_MOVES]; //Desk in dex of initial and end pos;
	int iEat[MAX_MOVES][6];  //Eated checker index (or 0), max 6 eats per move
	int nMoves, nBeatMoves;
	int mBest;
};

struct DeskState {
	short dsk[8][8];
	int isBeat(int y, int x, CMove *cm); 
	int isBeatOnly; //True if only beat moves are permitted
	int CalcAllMoves(int iStep, CMove *bestMove);
};

#define MAX_DEPTH 3

DeskState dse[MAX_DEPTH + 2];

//Find the best move from current desk state
int DeskState::CalcAllMoves(int iStep, CMove *bestMove) {  //Player index - 1 or 2
	if (iStep >= MAX_DEPTH) {
		iStep--;
		return 0;
	}
	int plind = 2 - (iStep & 1);
	isBeatOnly = false;
	CMove cmv[12];
	int nCmv = 0;
	for(int y = 0 ; y < 8 ; y++)
		for(int x=0; x < 8 ; x++)
			if ( dsk[y][x] && ((dsk[y][x] ^ plind) & 1) == 0)	{
				if (isBeat(y, x, cmv + nCmv))  //Check if beat only moves are accepted
					isBeatOnly = true;
				if (cmv->nMoves)
					nCmv++;
			}

	int maxVal = -9999, valDse = 0;
	for (int i = 0; i < nCmv; i++) {	//Check all moves
		CMove* cm = cmv + i;
		if (isBeatOnly && !cm->nBeatMoves)
			continue;
		int y1 = cm->iFrom / 8, x1 = cm->iFrom % 8;

		for (int j = 0; j < cm->nMoves; j++) {
			int val = 0;
			if (isBeatOnly && cm->iEat[j][0] == 0)
				continue;
			DeskState* dsn = dse + iStep + 1;
			memcpy(dsn, dse + iStep, sizeof(DeskState));
			int y2 = cm->iTo[j] / 8, x2 = cm->iTo[j] % 8;
			if (dsn->dsk[y1][x1] <= 3 && (y2 == 0 || y2 == 7))
				val += 2;
			dsn->dsk[y2][x2] = dsn->dsk[y1][x1];  //Moved checker
			dsn->dsk[y1][x1] = 0;
			int ie = cm->iEat[j][0];  //Eaten checkerif(ie)
			if(ie)
			{
				int ye = ie / 8, xe = ie % 8;
				dsn->dsk[ye][xe] = 0;
				val++;
			}
			valDse =
				val -= valDse;
			if (val >= maxVal) {
				maxVal = val;
				cm->mBest = j;
				if (bestMove)
					*bestMove = *cm;
			}
		}
	}

	iStep--;
	return maxVal;
}

//Check if checker on given pos must beat
int DeskState::isBeat(int y, int x, CMove *cm)
{
	int dy, dx, tx, ty, midc, k = 1, i, clc, isBeat = 0;
	clc = dsk[y][x];
	if (clc == 0)
		return false;
	if (clc > 2)
		k = 6;
	memset(cm, 0, sizeof(CMove));
	cm->iFrom = y * 8 + x;

	for(dy = -1 ; dy <= 1 ; dy += 2)
	  for (dx = -1; dx <= 1; dx += 2)
		for (i = 1; i <= k ; i++)
	    {
			tx = x + dx * i;
			ty = y + dy * i;
			if (tx < 0 || ty < 0 || ty > 7 || tx > 7)
				break;
			midc = dsk[ty][tx];
			if (midc == 0) {
				if (clc <= 2 && (clc == 2 && dy < 0 || clc == 1 && dy > 0))
					continue;
				cm->iTo[cm->nMoves++] = ty * 8 + tx;
			}
			else
			{
				if (((midc ^ clc) & 1) && ty + dy >= 0 && ty + dy <= 7 && tx + dx >= 0 && tx + dx <= 7)
					if (dsk[ty + dy][tx + dx] == 0)
					{
						cm->iEat[cm->nMoves][0] = ty * 8 + tx;
						cm->iTo[cm->nMoves++] = (ty + dy) * 8 + tx + dx;
						cm->nBeatMoves++;
						isBeat = true;
					}
				break;
			}
		}
	return isBeat;
}

char* dbName = "myd", * tabName = "users", * psw = "1Qe45$6";

void delSpaces(char* Query)
{
	int i = 0;
	for (int j = 0; Query[j]; j++) //Remove spaces
		if (Query[j] > ' ')
			Query[i++] = Query[j];
	Query[i] = 0;
}

int ULogin(char* Query)
{
	MyDB db;
	int cmd = 0;
	if (*Query == 'W')
		cmd = 1;
	if (*Query == 'D')
		cmd = -1;
	db.Load(dbName, tabName, psw);
	delSpaces(Query);
	if (db.FindRecord(Query + 1, cmd))
	{
		printf("+");
		printf("%s", db.getScore());
	}
	else
		printf("-");
	return 0;
}

int Register(char* Query)
{
	MyDB db;
	db.Load(dbName, tabName, psw);
	delSpaces(Query);
	char stn[100];  //To check existing name
	strcpy(stn, Query + 1);
	for (int i = 0; stn[i]; i++)
		if (stn[i] == '&')
			stn[i] = 0;
	if (db.FindRecord(stn)) {
		printf("-");		//Login already exists
		return 0;
	}
	db.InsertRecord(Query + 1);
	printf("+");
	return 0;
}

int main(int argc, char* argv[])
{
  char *s,ms[150];
  char* Query = getenv("QUERY_STRING");
  char cm = *Query;
  printf("Content-Type: text/html\n\n"); 
  if (cm == 'L' || cm=='W' || cm=='D')
	  return ULogin(Query);
  if (cm == 'R')
	  return Register(Query);

  char* dd = Query + 1;
  int isBeatOnly = false;
  for (int k = 0, y = 0; y < 8; y++)
	for (int x = 0; x < 8; x++)
	  dse[0].dsk[y][x] = dd[k++] - '0';
  CMove cmb;
  memset(&cmb, 0, sizeof(cmb));
  dse[0].CalcAllMoves(0, &cmb);
  printf("+%3d%3d", cmb.iFrom, cmb.iTo[cmb.mBest]);
  int *iEat = cmb.iEat[cmb.mBest];
  for (int j = 0; j < 4; j++)
	if (iEat[j] > 0)
	  printf("%3d", iEat[j]);
  char st[100];
  sprintf(st,"+%3d%3d", cmb.iFrom, cmb.iTo[cmb.mBest]);
  return 0;
}
