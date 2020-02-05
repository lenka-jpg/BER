#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

struct berResult
{
    unsigned long long tot; //total number of bits
    long long err; //number of bits in error
    float ber;  //ber calc result
    clock_t t1; //calc start clock
    clock_t t2; //calc stop clock
};
typedef struct berResult berResult;

void    openLog    (void);
void    saveLog    (char* pMsg);
void    closeLog   (void);

void createFile1(const char *pName, const long count, const char value);

long long size;

static FILE *pLogFile = NULL;
berResult calculateBer(char* fpath1, char* fpath2);

int main(int argc, char *argv[])
{
    char* fpath1 = "file1.bin";
    char* fpath2 = "file2.bin";
    berResult wnk;

    openLog();
    saveLog("BER v1.0 Start");

    if(3 != argc)
    {
        saveLog("Brak args. Tworzenie pliku");
        createFile1("file1.bin", 400000000u, 0x55);
        createFile1("file2.bin", 400000000u, 0x50);
    }
    else
    {
        saveLog("Wczytywanie plików...");
    }
    saveLog("Obliczam BER");
    wnk = calculateBer(fpath1, fpath2);
    printResult(wnk);

    closeLog();
    return 0;
}
void printResult(berResult resultOfCalc){
     printf("Rozmiar: %d\n", size );
     printf("Error: %d\n",resultOfCalc.err );
     printf("Ber: %f\n", resultOfCalc.ber);
     //printf("t1 = %ld", resultOfCalc.t1);
    // printf("t2 = %ld", resultOfCalc.t2);
     printf("time: %Lf \n", (double)(resultOfCalc.t2-resultOfCalc.t1)) /CLOCKS_PER_SEC;
}


berResult calculateBer(char* fpath1, char* fpath2){
    berResult resultOfCalc;
    resultOfCalc.err = 0;
    resultOfCalc.tot = 0;
    resultOfCalc.ber = 0;

    long long size2;
    FILE *fptr;
    FILE *fptr2;
    fptr = fopen(fpath1,"rb");
        if(fptr == NULL){
            printf("Error!");
            exit(1);
            }
    fptr2 = fopen(fpath2,"rb");
        if(fptr2 == NULL){
            printf("Error!");
            exit(1);
            }

    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    fseek(fptr,0, SEEK_SET);


    fseek(fptr2, 0, SEEK_END);
    size2 = ftell(fptr2);
    fseek(fptr2,0, SEEK_SET);

    if(size != size2){
        printf("Pilki maja zly rozmiar");
        fclose(fptr);
        fclose(fptr2);
        exit(0);
    }
    //printf("Count to %ld", size);
    resultOfCalc.t1 = clock();

    for (unsigned  long long i = 0; i < size; i++){
        char buff[1];
        char buff2[1];
        fread(buff, 1, 1, fptr); //1-> czytamy 1 element i ma rozmiar 1 bajta
        fread(buff2, 1, 1, fptr2);


        char helf = buff[0] ^ buff2[0];
        int inByte= 0;
        for(int f=0;f<8;f++){
            inByte+= helf&1;
            helf >>= 1;}

        resultOfCalc.err = resultOfCalc.err + inByte;
}

    resultOfCalc.t2 = clock();
    resultOfCalc.tot = size*8;
    resultOfCalc.ber = resultOfCalc.err / (float)resultOfCalc.tot;

    fclose(fptr);
    fclose(fptr2);
    char buffer [50];
    sprintf(buffer, "Ber to : %f", resultOfCalc.ber);
    saveLog(buffer);

    return resultOfCalc;
}

void createFile1(const char *pName, const long count, const char value)
{
    FILE *pFile = NULL;
    long i = 0;
    size_t s = 0;
   // printf("Count to %ld\n", count);
    pFile = fopen(pName, "w+");
    if (NULL == pFile) perror("fopen file failed");
    saveLog("Save to file started");
    for(i=0u; i < count ; i++)
    {
        s = fwrite(&value, sizeof(char), 1u, pFile);
        if(s != 1) perror("fwrite failed");
    }
    if(0 != fclose(pFile)) perror("fclose file failed");
    saveLog("File ready");
}

void openLog(void)
{
    pLogFile = fopen("log.txt", "a+");
    if (NULL == pLogFile) perror("fopen file failed");
}

void saveLog(char *pMsg)
{
    //Write the log header
    time_t now = time(NULL);
    size_t headerSize = snprintf(NULL, 0, asctime(localtime(&now)));

    char *pLogHeader = malloc(headerSize);
    headerSize = snprintf(pLogHeader, headerSize, asctime(localtime(&now)));
    size_t s = fwrite(pLogHeader, sizeof(char), headerSize - 1u, pLogFile);
    if(s != headerSize - 1u) perror("fwrite failed");
    s = fwrite(" ", sizeof(char), 1u, pLogFile);
    if(s != 1u) perror("fwrite failed");
    free(pLogHeader);

    //Write user pMsg
    s = fwrite(pMsg, sizeof(char), strlen(pMsg), pLogFile);
    s += fwrite("\n", sizeof(char), 1u, pLogFile);
    if(s != strlen(pMsg) + 1u) perror("fwrite failed");
    if (0 != fflush(pLogFile)) perror("fflush failed");
}

void closeLog (void)
{
    if(0 != fclose(pLogFile)) perror("fclose file failed");
}
