#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define EXIT 3

int pokaziMeni();
int puniFifo();
int prazniFifo();

void main(void)
{
	int op = 0;	
	
	do
	{
		switch(pokaziMeni())
		{
			case 1:	
				puniFifo();
					break;

			case 2:
				prazniFifo();
					break;

			case 3:
				op = EXIT;
					break;
		}
	}while(op != EXIT);
}

int pokaziMeni()
{
	int opcija = 0;

	printf("1: Upisi u FIFO bafer\n2:Procitaj iz FIFO bafera\n3:Izadji\n");
	do
	{
		scanf("%d", &opcija);
		if(opcija < 1 || opcija > 3)
			printf("Greska! Ne postoji ta opcija. Unesite ponovo.");
	}while(opcija < 1 || opcija > 3);

	printf("Izabrana opcija: %d\n", opcija);

	return opcija;
}

int puniFifo()
{
	FILE* fp;
	int upis[16], i = 0, j = 0, k = 0;
	char *unos, fifo[16][11], *echo, pom, flag;

	for(i = 0; i < 15; i++)
		upis[i] = 0;

	i = 0;
	unos = (char *)malloc(4);
	echo = (char *)malloc(180);
	printf("Unesite elemnt/e u FIFO: ");
	do
	{
		scanf("%s", unos);
		if((strcmp(unos, "q") == 0) || (strcmp(unos, "Q") == 0))
			flag = 1;

		upis[i] = atoi(unos);
		i++;
		
	}while(flag != 1);
	
	for(j = 0; j < i-1; j++)
		for(k = 0; k < 10; k++)
			fifo[j][k] = '0';

	for(j = 0; j < i-1; j++)
    {
        fifo[j][0] = '0';
        fifo[j][1] = 'b';
		fifo[j][10] = '\0';
 
        if(upis[j] - 128 >= 0)
        {
            fifo[j][2] = '1';
            upis[j] -= 128;
        }
		if(upis[j] - 64 >= 0)
        {
            fifo[j][3] = '1';
            upis[j] -= 64;
        }
		if(upis[j] - 32 >= 0)
        {
            fifo[j][4] = '1';
            upis[j] -= 32;
        }
		if(upis[j] - 16 >= 0)
        {
            fifo[j][5] = '1';
            upis[j] -= 16;
        }
		if(upis[j] - 8 >= 0)
        {
             fifo[j][6] = '1';
             upis[j] -= 8;
        }
		if(upis[j] - 4 >= 0)
        {
            fifo[j][7] = '1';
            upis[j] -= 4;
        }
		if(upis[j] - 2 >= 0)
        {
            fifo[j][8] = '1';
			upis[j] -= 2;
        }
		if(upis[j] - 1 >= 0)
        {
            fifo[j][9] = '1';
            upis[j] -= 1;
        }
	}

//	for(j = 0; j < i-1; j++)
//		printf("upis[%d] = %d\n ", j, upis[j]);

	

	strcpy(echo, fifo[0]);

	for(j = 1; j < i-1; j++)
	{
		
		strcat(echo, ";");
		strcat(echo, fifo[j]);

	}
	echo[(i-1)*10+2] = '\0';
	printf("\n%s", echo);
	printf("\n");
	fp = fopen("/dev/fifo", "w");
	if(fp == NULL)
	{
		printf("Problem pri otvaranju /dev/fifo\n");
		return -1;
	}

	fputs(echo, fp);	

	if(fclose(fp))
	{
		printf("Problem pri zatvaranju /dev/fifo\n");
		return -1;
	}
	
	return 0;
}

int prazniFifo()
{
	FILE *fp;
	int n = 1, i;
	char *cat;

	printf("\nKoliko brojeva zelite da citate: ");
	do
	{
		scanf("%d", &n);
	}while(n < 1 && n > 16);

	if(n > 1)
	{
		fp = fopen("/dev/fifo", "w");
		if(fp == NULL)
		{
			puts("Problem pri otvaranju /dev/fifo");
			return -1;
		}

		fprintf(fp, "num=%d", n);//ovde ne radi, ne salje komandu kako treba

		if(fclose(fp))
		{
			puts("Problem pri zatvaranju /dev/fifo");
			return -1;
		}
	}

	fp = fopen("/dev/fifo", "r");
	if(fp == NULL)
	{
		puts("Problem pri otvaranju /dev/fifo");
		return -1;
	}

	cat = (char *)malloc(180);
	getline(&cat, &n, fp);

	if(fclose(fp))
	{
		puts("Problem pri zatvaranju /dev/fifo");
		return -1;
	}

	printf("%s \n", cat);


	return 0;
}
