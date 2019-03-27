/*****************
Store one .POSTFIX file to MongoDB
Usage: ./pdbqt2mongo file
The format is like
{"_id":ObjectId("55ef43766cb5f36a3bae6ee4"),
 "idxd": 9999,
 "file_name": "DCADRN.xeu.pdbqt"
 "index_name": "ZINC001002370192",
 "pdbqt_file": "REMARK ... TORSDOF 5\n",
 "num_atoms": 23
}
Wu 2019.3.26
******************/

#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

#include "dirent.h" 

#include <bson.h>
#include <mongoc.h> 

#define POSTFIX ".pdbqt"

// splite string according to delimiters
// ATTENTION: this function modifies stringp
char * splite(char ** stringp, const char * delimiter){
	char * key_point= NULL;
	while(*stringp){
	    while ( key_point = strsep(stringp," ")){
	    if (*key_point == 0)
	        continue;
	    else
	        break;
	    }
	    return key_point;
	}
}


// concentrate two string
//usage:  
// char* s = concat("derp", "herp");
// ATTENTION:free(s); // deallocate the string
char* concat(const char * s1, const char *s2){
	char *result = NULL;
	if (s1 == NULL) {
		result = malloc(strlen(s2) + 1);
		strcpy(result, s2);
	}
	else {
		result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
	    if (result == NULL) exit(EXIT_FAILURE);
		strcpy(result, s1);
	    strcat(result, s2);
	}
    return result;
}

// check whether given str is in a str arrary
bool isIn(const char * arrary[99][99], int wordNum, const char * str){
	for (int i = 0; i < wordNum; i++){
		if (strcmp(str, "ENDMDL") == 0){
			if (strncmp(arrary[i], str, 6) == 0) return true;
		}
		else if(strcmp(arrary[i], str) == 0) return true;
	}
	return false;
}

int main(int   argc,
        char *argv[])
{
	
	char *fileName = argv[1];

	// initiate mongo db
	mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc;

	// connect to collection in db
    mongoc_init ();
    client = mongoc_client_new ("mongodb://12.11.70.12:10101/");
	collection = mongoc_client_get_collection (client, "zinc_data", "zinc15");

	// only read .POSTFIX file 
	const char *pFile;
	pFile = strrchr(fileName, '.'); // fint the last '.' position
	if (pFile == NULL || strcmp(pFile, POSTFIX) != 0) {
		printf("Warning: %s is not a valid file\n", fileName);
	}

	// read .POSTFIX file 
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
			
	fp = fopen(fileName, "r");
	if (fp == NULL) {
		printf("ERROR: Failed in reading %s!\n", fileName);
		exit(EXIT_FAILURE);
	}
	else printf("Read %s successfully!\n", fileName);
				
	// splite text
	int atomId = 0;
	int heavyAtomCount = 0;
	char * content = NULL;
	char * zincName = NULL;
	
	// ATTENTION: getline does not delete '\n' in the end of each line
	while ((read = getline(&line, &len, fp)) != -1){
		char * word = NULL;
		char * lineCopy = malloc(strlen(line) + 1);
		if (lineCopy == NULL) exit(EXIT_FAILURE);
		strcpy(lineCopy, line);
		bool flag = false;
					
		// allocate memory for 99 lines
		// each lines can contain 99 words
		// it is sufficient to store text between MODEL and ENDMDL
		char * spliteLine[99][99];
		int wordNum = 0;
	
		// store each word into spliteLine arrary
		for (wordNum = 1; (word = splite(&line, " ")); wordNum++){
			strcpy(spliteLine[wordNum - 1], word);
		}
		
		// remove last iteration count
		wordNum --;
			        
		// processing text
		if (isIn(spliteLine, wordNum, "MODEL")){
			atomId++;
		}
		else if (isIn(spliteLine, wordNum, "Name")){
			zincName = malloc(strlen(spliteLine[wordNum - 1]) + 1);
			if (zincName == NULL) exit(EXIT_FAILURE);
			strcpy(zincName, spliteLine[wordNum - 1]);
			zincName[strlen(zincName) - 1] = '\0'; 
		}
			        
		// ATTENTION: if file is modified in Windows, when type 'enter' \r\n appears 
		// in linux, only \n
		else if (isIn(spliteLine, wordNum, "ENDMDL")){
			        	
			// insert to collection
			doc = bson_new ();
			bson_oid_init (&oid, NULL);
			BSON_APPEND_OID (doc, "_id", &oid);
			BSON_APPEND_INT32 (doc, "idxd", atomId);
			BSON_APPEND_UTF8 (doc, "file_name", fileName);
			BSON_APPEND_UTF8 (doc, "index_name", zincName);
			BSON_APPEND_UTF8 (doc, "pdbqt_file", content);
			BSON_APPEND_INT32 (doc, "num_atoms", heavyAtomCount);
						
			if (!mongoc_collection_insert (collection,
				MONGOC_INSERT_NONE, doc, NULL, &error)) {
				fprintf (stderr, "%s\n", error.message);
			}
			bson_destroy (doc);
						
			// display splited result
//			printf("atom id = %d\n", atomId);
//			printf("zinc name = %s\n", zincName);
//			printf("content = \n%s\n", content);
//			printf("num heavy atoms = %d\n", heavyAtomCount);
			            
			// reset value
			free(content);
			content = NULL;
			heavyAtomCount = 0;
			free(zincName);
		}
		else{
			char * tmp = concat(content, lineCopy);
			if (content) free(content);
			content = tmp;
			if (isIn(spliteLine, wordNum, "ATOM") & strcmp(spliteLine[2], "H") != 0){
			        heavyAtomCount ++;
			}
		}
	}
	fclose(fp);
	if (line) free(line);		   
	
	// free mongo db connection 
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);
    mongoc_cleanup ();
    exit(EXIT_SUCCESS);
}
