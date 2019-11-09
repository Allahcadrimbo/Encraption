//Author Grant Mitchell
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define TRANS 1
#define NOTRANS 0

#ifndef DEBUG
	#define DEBUG 0
#endif 

#define ENCODE 0
#define DECODE 1

#ifndef MODE
	#define MODE ENCODE
#endif


int print_buffer(char *buf, unsigned int bytes) {
	/* takes in a pointer to a buffer and prints out as many
	 * bytes as specified */

	for(int i = 0; i < bytes; ++i)
	{
		printf("%c", buf[i]);
	}

	return bytes;

}

int transpose_buffer(char *out, char *in, unsigned int dim) {
	/* do a columnar encipher/decipher
	 * from in to out
	 * using box of size dim*dim
	 * since it's a square, enciphering and deciphering is the same
	 */

	//2D array to store the data to be encrypted/decrypted
	char trans[dim][dim];
	int iter = 0; //Interate throught the stored data
	//Loading the data into the 2d array. Loading it by rows.
	//print_buffer(in, 100);
	for(int i = 0; i < dim; i++)
	{
		for(int j = 0; j < dim; j++)
		{
			trans[j][i] = in[iter];
			++iter;
		}
	}

	iter = 0; //Reset the iterator
	//Write the date from trans into the out array by reading through columns instead of rows
	for(int i = 0; i < dim; i++)
	{
		for(int j = 0; j < dim; j++)
		{
			out[iter] = trans[i][j];
			iter++;
		}
	}	

	return 0;
}

int dump_buffer(char *buffer, unsigned int bufsize, 
				unsigned int bytes, FILE *output) {

	/* prints a buffer one character at a time to a file using %c
	 * takes in:
	 *  buffer -- pointer to a buffer
	 *  bufsize -- size of 'buffer'
	 *  bytes -- number of bytes from buffer to print
	 *  output -- path to the file to open and output to
	 */
	
	/* open the output or quit on error */
	/*if ((output = fopen(output, "w+")) == NULL)
	{
			printf("Problem opening output file '%s'; errno: %d\n", output, errno);
			return 1;
	}*/

	/* print 'bytes' bytes from buffer to output file one char at a time */
	for(int i = 0; i < bytes; i++)
  	{
    	int out = buffer[i];
		fprintf(output, "%c", out);
  	}
	  //print_buffer(buffer, bytes);
	/* optional: wipe buffer using memset */
	memset(buffer, 0, bufsize);
	
	/* close output file */
	//close(output);

	return bytes;
}


int pad_buffer(char *buffer, unsigned int bufsize, unsigned int rbuf_index) {


	/* pad_buffer pads the empty space in a buffer
	 *  buffer -- pointer to buffer
	 *  bufsize -- size of 'buffer'
	 *  rbuf_index -- first "empty" spot in buffer, i.e.,
	 *                put the 'X' at rbuf_index and fill the
	 *                rest with 'Y' characters.
	 */

	int padded = 0;

	buffer[rbuf_index] = 'X';
	padded++;
	rbuf_index++;
	for(rbuf_index; rbuf_index < bufsize; rbuf_index++)
	{
		buffer[rbuf_index] = 'Y';
		++padded;
	}

	return padded;
}
	
int unpad_buffer(char *buffer, unsigned int bufsize) {

	/* unpads a buffer of a given size
	 *  buffer -- buffer containing padded data
	 *  bufsize -- size of 'buffer'
	 */

	int unpadded = 0;

	//Remove all of the Ys
	do{
		bufsize--;
	}
	while(buffer[bufsize] != 'X');

	return bufsize;
}
	
int main(int argc, char *argv[]) {

	int i = 0; /* iterator we'll reuse */

	if (argc < 4) {
		printf("Missing arguments!\n\n");
		printf("Usage: encoder dim infile outfile ['notrans']\n\n");
		printf("Note: outfile will be overwritten.\n");
		printf("Optional '1' as last parameter will disable transposition.\n");
		return 1;
	}

	/* give input and output nicer names */
	unsigned int dim = atoi(argv[1]); 	/* dimension of the box */
	char *input = argv[2]; 				/* input file path */
	char *output = argv[3];				/* output file path */

	/* use 'transmode' to determine if we are just padding or also
	 * doing transposition. very helpful for debugging! */

	unsigned int transmode = TRANS;		/* default is TRANS */
	if (argc > 4 && (atoi(argv[4]) == 1)) {
		printf("Warning: Transposition disabled\n");
		transmode = NOTRANS;
	}

	unsigned int rbuf_count = 0;
	unsigned int bufsize = dim * dim;
	char read_buf[bufsize]; /* buffer for reading and padding */
	char write_buf[bufsize]; /* buffer for transposition */

	/* open the input or quit on error. */
	FILE *INPUT;
   	if ((INPUT = fopen(input, "r")) == NULL) {
		printf("Problem opening input file '%s'; errno: %d\n", input, errno);
		return 1;
	}


	/* get length of input file */
	unsigned int filesize;		/* length of file in bytes */
	unsigned int bytesleft;		/* counter we reduce on reading */
	struct stat filestats;		/* struct for file stats */
	int err;
	if ((err = stat(input, &filestats)) < 0) {
		printf("error statting file! Error: %d\n", err);
	}

	filesize = filestats.st_size;
	bytesleft = filesize;

	if (DEBUG) printf("Size of 'input' is: %u bytes\n", filesize);

	/* truncate output file if it exists */

	FILE *OUTPUT;
   	if ((OUTPUT = fopen(output, "w+")) == NULL) {
		printf("Problem truncating output file '%s'; errno: %d\n", output, errno);
		return 1;
	}
	


	/* loop through the input file, reading into a buffer and 
	 * processing the buffer when 1) the buffer is full or
	 * 2) the file has ended (or in the case of decoding, when
	 * the last block is being processed.
	 */

	int rbuf_index = 0; /* index into the read buffer */
	int symbol; /* we will read each input byte into 'symbol' */
	int count = 0;
	bool addEnd = true;

	//Opens the input file stream
	do{
		for(int temp = 0; temp < bufsize; temp++)
		{
			if(((symbol = fgetc(INPUT)) != EOF))
			{
				read_buf[count] = symbol;
				count++;
				bytesleft--;
			}
		}

		if(MODE == ENCODE)
		{
			int encSize;
			if(count < bufsize)
			{
				addEnd = false; //Whole block doesn't need to be added.
				encSize = count + pad_buffer(read_buf, bufsize, count);
				transpose_buffer(write_buf,read_buf, dim);
			}
			else{
				transpose_buffer(write_buf, read_buf, dim);
			}
			dump_buffer(write_buf, encSize, bufsize, OUTPUT);
		}
		else{ //DECODING unpad only for last block.
			transpose_buffer(write_buf, read_buf, dim);
			
			if(bytesleft == 0)
			{	
				//print_buffer(write_buf, bufsize);
				int decSize = unpad_buffer(write_buf, bufsize);		
				dump_buffer(write_buf, bufsize, decSize, OUTPUT);
			}
			else{
				dump_buffer(write_buf, bufsize, bufsize, OUTPUT);
			}
		}
		count =  0;
	}
	while(bytesleft != 0);
 
	//Add the padding when an entire padding block is needed
	if(MODE == ENCODE)
	{
		if(addEnd)
		{
			read_buf[0] = 'X';

			for(int j = 1; j < bufsize; j++)
			{
				read_buf[j] = 'Y';
			}
			transpose_buffer(write_buf, read_buf, dim);
			dump_buffer(write_buf, bufsize, bufsize, OUTPUT);
		}
	}

	fclose(INPUT);
	fclose(OUTPUT);	
	return 0;
}

