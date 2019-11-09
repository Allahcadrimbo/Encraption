//Author Grant Mitchell
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG 0
#define ENCODE 1 /* the macro ENCODE expands to the number 1 */
#define DECODE 0 /* DECODE expands to 0 */
#ifndef MODE 	 /* if the macro MODE wasn't defined during compilation... */
  #define MODE ENCODE /* define it to be ENCODE */
#endif


int main(int argc, char *argv[]) {

  char *keypath = argv[1];    /* file path is the first argument */
  char *input = argv[2];      /* input file name */
  char *output = argv[3];     /* output file name */
  char key[128];             /* we will only copy up to 128 bytes into the array */
  unsigned int length = 0;    /* length of the data */
  int symbol;                /* we'll read characters into this variable */
  char cipher;                /* we will read encrypted and decrypted chars into this var*/
  FILE *INPUT;                /* Input file pointer */
  FILE *OUTPUT;               /* Output file pointer */
  FILE *KEYPATH;              /* key file pointer */
  int i = 0;                  /* iterator we'll reuse */


	/* perform a simple vigenere cipher
	 * -------------------------------
	 * accepts three arguments:
	 * 1. An input file of a key
	 * 2. An input file with content to either encrypt or decrypt
	 * 3. An output file to write to
	 */



  //Opens the input file stream
  if ((INPUT = fopen(input, "r")) == NULL)
  {
    printf("Problem opening key file '%s'; errno: %d\n", input, errno);
    exit(1);
  }

  //Opens the output file stream
  if ((OUTPUT = fopen(output, "w+")) == NULL)
  {
		printf("Problem opening output file '%s'; errno: %d\n", output, errno);
		return 1;
  }
  
  //Opens the keypath file stream
  if ((KEYPATH = fopen(keypath, "r")) == NULL)
  {
		printf("Problem opening output file '%s'; errno: %d\n", keypath, errno);
		return 1;
  }

  //Read the first 128 bytes of the keyfile into the key array
  while ((length < 128) && (symbol = fgetc(KEYPATH)) != EOF)
  {
    key[length] = (char) symbol;
    length++;
  }
  fclose(KEYPATH);

  if (MODE == ENCODE)
  {
    //Read in the input file and encrypt symbol
    while ((symbol = fgetc(INPUT)) != EOF)
    {
      i = i % length; //Keeps on looping over the key
      cipher = (((char)symbol + key[i]) % 256);
      fprintf(OUTPUT, "%c", cipher); //Print cipher to output
      i++;
    }
  }
  else //Decoding
  {
    //Read in the input file and decrypt symbol 
    while ((symbol = fgetc(INPUT)) != EOF)
    {
      i = i % length;
      cipher = (((char)symbol - key[i] + 256) % 256);
      fprintf(OUTPUT, "%c", cipher);
      i++;
    }
  }
  fclose(INPUT);
  fclose(OUTPUT);
  return 0;
}
 
