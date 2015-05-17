/*
 * Author:      Rakesh Sharad Navale.
 * @(#)$Id: TransactionProcessing.c,v 1.0 2015/01/15 20:15:30 $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "cs402.h"
#include "TransactionProcessing.h" // private header file
char buf[1026];

typedef struct transaction {  /* data structure */
	char ttype[1];
	char tdate[16];
	char tdescription[24];
	double tamount;
	time_t ttimecheck;
	int tbalance;
}Transact;

//-------------------------------------------Functions-----------------------------------------
static
void BubbleForward(My402List *pList, My402ListElem **pp_elem1, My402ListElem **pp_elem2) { // bubble sort elements
	My402ListElem *elem1=(*pp_elem1), *elem2=(*pp_elem2);
	Transact *T11 = (Transact*)elem1->obj;
	Transact *T22 = (Transact*)elem2->obj;

	My402ListElem *elem1prev=My402ListPrev(pList, elem1);
	My402ListElem *elem2next=My402ListNext(pList, elem2);

	My402ListUnlink(pList, elem1);
	My402ListUnlink(pList, elem2);
	if (elem1prev == NULL) 	{
        	(void)My402ListPrepend(pList, T22);
        	*pp_elem1 = My402ListFirst(pList);
	} 
	else 	{
	        (void)My402ListInsertAfter(pList, T22, elem1prev);
	        *pp_elem1 = My402ListNext(pList, elem1prev);
	}
	if (elem2next == NULL) {
	        (void)My402ListAppend(pList, T11);
	        *pp_elem2 = My402ListLast(pList);
	} 
	else {
	        (void)My402ListInsertBefore(pList, T11, elem2next);
	        *pp_elem2 = My402ListPrev(pList, elem2next);
	}
}
//------------------------------------------------------------------------------------
static
void BubbleSortForwardList(My402List *pList, int num_items) { /* getting list to sort and compare transaction time. */
	My402ListElem *elem=NULL;
	int ii=0;
	if (My402ListLength(pList) != num_items) {
		//fprintf(stderr, "\nList length is not %1d in BubbleSortForwardList().\n", num_items);
		exit(1);
	}
	for (ii=0; ii < num_items; ii++) {
		int j=0, something_swapped=FALSE;
		My402ListElem *next_elem=NULL;

		for (elem=My402ListFirst(pList), j=0; j < num_items-ii-1; elem=next_elem, j++) {
			Transact *T1 = (Transact *)elem->obj;			
			time_t cur_val = (T1->ttimecheck), next_val=0;
			next_elem=My402ListNext(pList, elem);
			Transact *Tn = (Transact *)next_elem->obj;
			next_val = (Tn->ttimecheck);
			if (cur_val == next_val) { 			/* Duplicate entried based on timestamp*/
				fprintf(stderr, "Error : Multiple transaction entries at the same time.\n");
				exit(1);
			}
			if (cur_val > next_val) {
				BubbleForward(pList, &elem, &next_elem);
				something_swapped = TRUE;
			}
		} if (!something_swapped) break;
	}
}
///-----------------------------------------------------------------------------------
static void PrintTestList(My402List *pList)
{
	int num_items = 0, q=0,q2,z=0; 
	double bal = 0, copyprev=0, bal1=0;
	num_items = pList->num_members;	My402ListElem *elem=NULL;
	if (My402ListLength(pList) != num_items) 
	{
		fprintf(stderr, "\nList length is not %1d in PrintTestList()\n", num_items);
		exit(1);
	}
	fprintf(stdout,"+-----------------+--------------------------+----------------+----------------+\n");
	fprintf(stdout,"|       Date      | Description              |         Amount |        Balance |\n");
	fprintf(stdout,"+-----------------+--------------------------+----------------+----------------+\n");
//---------------------------------------------------------------------------------------------------------------
		for (elem = My402ListFirst(pList); elem != NULL; elem = My402ListNext(pList, elem))  /* Traverse the list */
		{
			char buffer[12],balbuf[12], finalamount[12]="?,???,???.??",copyamount[13],finalbal[12]="?,???,???.??";
			Transact *T1 = (Transact*)elem->obj;
			copyprev=T1->tamount;
			char copybal[13];
//---------------------------------------------------------------------------------------------------------------
			if (T1->tamount > 999999999) { /* amount greater than 10 million*/
				fprintf(stderr, "Error : Transaction line [%d] : Transaction amount more than 10million\n", num_items);
				exit(1);
			}
			else {
				for(q2=0;q2<12;q2++) {
					buffer[q2]=' ';
					finalamount[q2]=' ';
				}
				int convert_amount = T1->tamount >= 0 ? (long)(T1->tamount+0.5) : (long)(T1->tamount+0.5); /* precision conversion */
				snprintf(buffer, 12, "%d",convert_amount);
				q=11,z=strlen(buffer)-1;
				while(z >= 0) { /* format amount to display */
					if(q != 1 &&q != 5  && q != 9 && strlen(buffer) -z > 0 ) {
						finalamount[q]=buffer[z];
						z--;
						if(strlen(buffer) == 2)
							{finalamount[9]='.';finalamount[8]='0';}
					}
					if(q==9) {
						if(strlen(buffer) > 2)
							finalamount[q]='.';
						else 
							finalamount[q]=' ';
					}
					if(q==5) {
						if(strlen(buffer) > 5)
							finalamount[q]=','; 
						else 
							finalamount[q]=' ';
					}
					if(q==1) {
						if(strlen(buffer) > 8)
							finalamount[q]=','; 
						else 
							finalamount[q]=' ';
					}q--;
				}
			}
			snprintf(copyamount, 13, "%s",finalamount);
//-----------------------------------------------------------------------------------------------------------------------------------
		if( *T1->ttype == '+') { /* calculate balance according to the current transaction and the previous balance amount*/
			bal1 = copyprev + bal1;
			if(bal1 < 0)
				bal = (-1)*bal1;
			else 
				bal = bal1;
		}
		else {
			bal1 = (bal1 - copyprev);
			if(bal1 < 0)
				bal = (-1)*bal1;
			else 
				bal = bal1;
		}
		q=0;
		if (bal > 999999999) /* do nothing for display as its "?,???,???.??" */
		{ }	
		else /* Format balance for display*/
		{
			for(q2=0;q2<12;q2++) {
				balbuf[q2]=' ';
				finalbal[q2]=' ';
			}
			snprintf(balbuf, 12, "%.0f",bal);
			q=11 , z = strlen(balbuf)-1;
			while(z >= 0 ) {
				if(q != 1 &&q != 5  && q != 9 && strlen(balbuf) -z > 0 && balbuf[z]!='-' ) {
					finalbal[q]=balbuf[z]; z--;
					if(strlen(balbuf) == 2)
						{finalbal[9]='.';finalbal[8]='0';}
				}

				if(q==9) {
					if(strlen(balbuf) > 2)
						finalbal[q]='.';					
					else 
						finalbal[q]=' ';
				}
				if(q==5) {
					if(strlen(balbuf) > 5)
						finalbal[q]=','; 
					else 
						finalbal[q]=' ';
				}
				if(q==1) {
					if(strlen(balbuf) > 8)
						finalbal[q]=','; 
					else 
						finalbal[q]=' ';
				}q--;
			}
			while(q>=0) {	
				finalbal[q]=' ';q--;
			}
		}
		snprintf(copybal, 13, "%s",finalbal);
//-----------------------------------------------------------------------------------------------------
		if( *T1->ttype == '+') { /* printing the whole list in given format */
			if(bal1 >= 0)
				fprintf( stdout , "| %13s | %-24s | %13s  | %13s  |\n", T1->tdate,T1->tdescription,copyamount,copybal);
			else 
				fprintf( stdout , "| %13s | %-24s | %13s  | (%12s) |\n", T1->tdate,T1->tdescription,copyamount,copybal);
		}
		else {
			if(bal1 >= 0)
				fprintf( stdout ,"| %13s | %-24s | (%12s) | %13s  |\n", T1->tdate,T1->tdescription,copyamount,copybal);
			else 
				fprintf( stdout ,"| %13s | %-24s | (%12s) | (%12s) |\n", T1->tdate,T1->tdescription,copyamount,copybal);
		}
   	}
}
//----------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) 
{
	char temp2[16],temp3[16],temp4[80], check_access=0;
	int i=0,j=0,x=0,f=0;
	FILE *fp;	
	My402List list;
	memset(&list, 0, sizeof(My402List));
	My402ListInit(&list);
//----------------------- open file and error handling -----------------------------------------------------------------------------------------
	if (argc == 1) {
		fprintf (stderr,"[malformed command]\n");
		exit(1);
	}
	if (argc > 3) {
		fprintf (stderr,"[malformed command]\n");
		exit(1);
	}
	if (argc == 2) {
		if (strcmp(argv[1],"sort")!=0) {
			fprintf (stderr,"[malformed command]\n");
			exit(1);
		}
		else {
			fp= stdin;
		}
	}
	if (argc == 3) {
		if (strcmp(argv[1],"sort")!=0) {
			fprintf (stderr,"[malformed command]\n");
			exit(1);
		}
		fp = fopen(argv[2], "r");	
	}
	if(chdir(argv[2])==0) {
		fprintf (stderr,"Error : %s is directory!\n",argv[2]);
		fclose(fp);
		exit(1);
	}
	if (fp == NULL) {
		fprintf (stderr,"Error : Input file %s does not exist!\n",argv[2]);
		exit(1);
	}
//----------------------- parse line by line -----------------------------------------------------------
	while (1) {
	        if (fgets(buf, sizeof(buf), fp) == NULL ) {			
			if(check_access==0) {
				fprintf (stderr,"Error %s - is a directory\n",argv[2]);
				fclose(fp);
				return -1;//exit(1);
			}
			else {
				break;
			}
		}
		else { // get line and parse
			i++; //number of transactions
			check_access=1;	j=0; 
			Transact* T = (Transact*)malloc(sizeof(Transact));
			memset(T, 0, sizeof(Transact));
			char *start_ptr = buf, *tab_ptr, *startfield[5] = {"","","","",""}, temptime[30];
			while(start_ptr != NULL) {
				tab_ptr = strchr(start_ptr, '\t');
				if(tab_ptr != NULL) {
					*tab_ptr++ = '\0';
				}
				startfield[j]=(char *)start_ptr;
				if(strlen(startfield[j])>90) {
					fprintf(stderr, "Error : Transaction Line [%d] : Invalid transaction details or format\n", i);
					fclose(fp);
					exit(1);
				}
				start_ptr = tab_ptr;
				j++;
			}
			/* checkk if the seperated fields are blanck or contains additional tabs*/
			if(strcmp(startfield[4],"")!=0 || strcmp(startfield[3],"")==0 || strcmp(startfield[2],"")==0 ||strcmp(startfield[1],"")==0 ||strcmp(startfield[0],"")==0) {
				fprintf(stderr, "Error : Transaction Line [%d] : Invalid transaction details or format\n", i);
				fclose(fp);
				exit(1);
			}
//------------------------------------transaction type----------------------------------------------------
			strncpy(T->ttype,startfield[0],16);
			if(strcmp(T->ttype,"-")!=0 && strcmp(T->ttype,"+")!=0) {  // check for empty description or double tab '\t'
				fprintf(stderr, "Error : Transaction Line [%d] : Invalid transaction type!\n", i);
				fclose(fp);
				exit(1);
			}
//-------------------------------------------time---------------------------------------------------------
			strncpy(temp2,startfield[1],16);
			time_t timestamp = atoi(temp2);
			strncpy(temptime,ctime(&timestamp),30);
			T->ttimecheck = timestamp;
			if ( timestamp < 0 || timestamp > time(NULL) ) { /* wrong timestamp for transaction */
				time_t current_time;
				char* c_time_string;
				current_time = time(NULL);
				c_time_string = ctime(&current_time);
				fprintf(stderr,"Error : Transaction line [%d]\nTransaction time: %sCurrent time %s\n",i,temptime,c_time_string);
				fclose(fp);
				exit(1);
			}
			x=0,f=0;
			while(x<24) {
				if(x<11 || x> 19) {
					T->tdate[f] = temptime[x];
					f++; 
				} x++;
			}
//-------------------------------------------amount---------------------------------------------------------
			double tempamount;
			strncpy(temp3,startfield[2],16);
			if(strcmp(temp3,"\n")==0) {
				fprintf(stderr, "Error : Transaction line [%d] : Invalid transaction amount!\n", i);
				fclose(fp);
				exit(1);
			}
			tempamount = atof(temp3)*100;
			/* check if the input transaction contains wrong formate for amount, correct format : "$$$$$$$.$$" */
			int qaz=0,out=0,in=0;
			for (qaz=0;qaz<strlen(temp3);qaz++) {
				out++;	
				if( (temp3[qaz++]) == '.' ) {	
					for (qaz=out;qaz<strlen(temp3)-out+1;qaz++) {
						in++;
						if( (temp3[qaz])=='\0')
							break;
					}
				}				
			}	
			if(in > 2 ) {	/* transaction amount contains more than two digits to the right of '.' e.g. $$$$."$$$" */
				fprintf(stderr, "Error : Transaction line [%d] : Invalid transaction amount!\n", i);
				fclose(fp);
				exit(1);
			}
			T->tamount = tempamount;
			if(T->tamount > 999999999) {
				fprintf(stderr, "Error : Transaction line [%d] :Transaction amount more than 10million\n",i);
				fclose(fp);
				exit(1);
			}
/*-------------------------------------------description---------------------------------------------------------------------------*/
			char *d, *empty = "";
			d = startfield[3];
			if(*d == '\n' || *d == *empty ) {  // check for empty description or double tab '\t'
				fprintf(stderr, "\nError : Transaction Line [%d] : Invalid Description!\n", i);
				fclose(fp);
				exit(1);
			}
			if(*d == ' ') {
				while(*d == ' ')
					d++;
			}
			strncpy(temp4,d,24);
			for(x=0;x<24;x++) {
				if(temp4[x]=='\n') {
					temp4[x]='\0'; // if desc is smaller append all '\0'
				}
				if(temp4[x]!='\n' || temp4[x]!='\0' ) {
					T->tdescription[x]=temp4[x];
				}
			}
			T->tbalance = T->tamount;
/*--------------Append element to the list--------------------------------------------------------------------------------------------*/
		My402ListAppend(&list,T);
/*------------------------------------------------------------------------------------------------------------------------------------*/
		}
	}
/*------Sort the list before printing-------------------------------------------------------------------------------------------------*/
	BubbleSortForwardList(&list, i);
/*------Display records and detail----------------------------------------------------------------------------------------------------*/
	PrintTestList(&list);
	fprintf(stdout,"+-----------------+--------------------------+----------------+----------------+\n");
	fclose(fp);
return 0;
}
