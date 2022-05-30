#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#define MAX_PATH_LENGTH 50

typedef struct wordnode {
  char* word;
  struct wordnode* next;
  int timeMod;
} wordnode;



int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

int isFile(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

wordnode* insertFront(wordnode* wordlist, char* nextWord, time_t time) {
    wordnode* current = (wordnode*)malloc(sizeof(wordnode));
    current->word = (char*)malloc(strlen(nextWord) + 1);
    current->word[strlen(nextWord)] = 0;
    strncpy(current->word, nextWord, strlen(nextWord));
    current->timeMod = time;
    current->next = wordlist;
    return current;
}

wordnode* insertWordAlpha(wordnode* wordlist, char* nextWord, int tFlag, time_t insertTime) {
    //declare current and prev to insert in between 
    wordnode* current = wordlist;
    wordnode* prev = wordlist;
    //keeps track of which letter is currently being used to sort
    int letter  = 0;
    time_t currTime = wordlist->timeMod;
    while((!tFlag || (tFlag && (currTime == insertTime))) && (current != NULL) &&
    current->word[letter] <= nextWord[letter]) {
        if(current->word[letter] == nextWord[letter]) {
            letter++;
        }
        else {
            letter = 0;
            prev = current;
            current = current->next;
            if(current) currTime = current->timeMod;
        }
    }
    //if word to be inserted is longer than current word and contains all same letters
    //then it must be inserted after current
    if(current != NULL && current->word[letter] == 0) {
        prev = current;
        current = current->next;
    }
    //if being inserted in front of list
    if(current == wordlist) {
        wordlist = insertFront(wordlist, nextWord, insertTime);
    }
    else {
        prev->next = insertFront(current, nextWord, insertTime);
    }
    return wordlist;
}

wordnode* insertWordTime(wordnode* wordlist, char* nextWord, time_t time) {  
    wordnode* current = wordlist;
    wordnode* prev = wordlist;
    time_t currTime = wordlist->timeMod;
    //keeps track of which letter is currently being used to sort
    while(current != NULL && time < currTime) {
        prev = current;
        current = current->next;
        if(current) 
            currTime = current->timeMod;
    }
    //if being inserted in front of list
    if(currTime == time) {
        if(current == wordlist) {
            wordlist = insertWordAlpha(wordlist, nextWord, 1, time);
        }
        else {
            prev->next = insertWordAlpha(current, nextWord, 1, time);
        }
    }
    else {
        if(current == wordlist) {
            wordlist = insertFront(wordlist, nextWord, time);
        }
        else {
            prev->next = insertFront(current, nextWord, time);
        }
    }
    return wordlist;
}

wordnode* insertWord(wordnode* wordlist, char* nextWord, int tFlag, time_t timeMod) {
    if(wordlist == NULL) {
        wordlist = insertFront(wordlist, nextWord, timeMod);
    }
    else {
        if(tFlag) {
            wordlist = insertWordTime(wordlist, nextWord, timeMod);
        }
        else {
            wordlist = insertWordAlpha(wordlist, nextWord, 0, 0);
        }
    }
    return wordlist;
}

void printList(wordnode* wordlist) {
    wordnode* current = wordlist;
    while(current != NULL) {
        printf("%s", current->word);
        if(current->next != NULL) {
            printf("    ");
        }
        else printf("\n");
        current = current->next;
    }
}

void freemem(wordnode* wordlist) {
    wordnode* current = wordlist;
    wordnode* prev = wordlist;
    while(current) {
        prev = current;
        current = current->next;
        free(prev->word);
        free(prev);
    }
}

void setFlags(int* aFlag, int* tFlag, char* flags) {
    while(*flags) {
        if(*flags == 'a') {
            *aFlag = 1;
        }
        else if (*flags == 't') {
            *tFlag = 1;
        }
        else {
            printf("%c is not a valid flag", *flags);
        }
        flags++;
    }
}

time_t getTime(char* dir, char* d_name) {
    struct stat statbuf;
    char path[MAX_PATH_LENGTH] = {0};
    strncpy(path, dir, strlen(dir));
    if(dir[strlen(dir) - 1] != '/') {
        path[strlen(dir)] = '/';
    }
    strncpy(path + strlen(path), d_name, strlen(d_name));
    if (stat(path, &statbuf)) {
        printf("There was an error, stat failed");
        return 0;
    }
    return statbuf.st_mtime + (double) statbuf.st_mtim.tv_nsec/(1000000000);
}

void readContents(char* dir, int sFlag, int tFlag) {
    struct dirent *de;  
    DIR *dp;
    dp = opendir(dir);
    wordnode* fileList = NULL;
    while( (de = readdir(dp)) ) {
        if((!sFlag && *(de->d_name) != '.') || sFlag) {
            time_t time = tFlag ? getTime(dir, de->d_name) : 0;

            char* strn = (char*) malloc(strlen(de->d_name) + 1);
            strncpy(strn, de->d_name, strlen(de->d_name));
            strn[strlen(de->d_name)] = 0;
            
            fileList = insertWord(fileList, strn, tFlag, time);
            free(strn);
        }
    }
    closedir(dp);
    printList(fileList);
    freemem(fileList);
}
void printDirList (wordnode* dirlist, int sFlag, int tFlag) {
    while(dirlist) {
        printf("\n");
        printf("%s: \n", dirlist->word);
        readContents(dirlist->word, sFlag, tFlag);
        dirlist = dirlist->next;
    }
}
int main(int ac, char** av) {
    wordnode* filelist = NULL;
    wordnode* dirlist = NULL;
    int sFlag = 0;
    int tFlag = 0;
    int vars = 1;
    for(int i = 1; i < ac; i++) {
        char* temp = av[i];
        if(temp[0] == '-') {
            setFlags(&sFlag, &tFlag, av[1] + 1);
        }
        else {
            vars = 0;
            struct stat statbuf;
            time_t time = statbuf.st_mtime + (double)statbuf.st_mtim.tv_nsec/1000000000;
            if (stat(av[i], &statbuf) != 0) {
                printf("There was an error, stat failed");
                return 0;
            }
            if(isDirectory(temp)){
                dirlist = insertWord(dirlist, temp, tFlag, time);
            }
            else if(isFile(temp)) {
                filelist = insertWord(filelist, temp, tFlag, time);
            }
            else printf("%s does not exist", temp);
        }
    }
    if(vars) {
        readContents(".", sFlag, tFlag);
    }
    //print list
    printList(filelist);
    printDirList(dirlist, sFlag, tFlag);

    // clean up memory
    freemem(filelist);
    freemem(dirlist);
}
