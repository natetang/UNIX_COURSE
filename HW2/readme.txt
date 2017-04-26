I choose five more functions to monitor
fread  fwrite fputs  fputc fgetc

I choose these 5 because they seem like easy to test

int main () {
  FILE * pFile;
  long lSize;
  char * buffer;
  char * bufferforw={123465465};	
  size_t result;

  pFile = fopen ( "myfile.bin" , "rb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  // allocate memory to contain the whole file:
  buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (buffer,1,lSize,pFile);
	fwrite(bufferforw,1lSize,pFile);
  
  if (result != lSize) {fputs ("Reading error",stderr);fputc(3,stderr); exit (3);}
	fgetc(stderr);
  /* the whole file is now loaded in the memory buffer. */

  // terminate
  fclose (pFile);
  free (buffer);
  return 0;
}


