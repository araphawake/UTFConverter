#include <stdio.h>
#include <stdlib.h>

#include "utf.h"

int main ( )
{

	FILE * file;
	FILE * fileOut;
	char fileName[100];
	char convertionOption = '0';
	int state = 0;
	
	// Input de opção de conversão entre UTF-8 e UTF-32.
	printf( "\n\nEnter '1' for convertion from UTF-8 to UTF-32 or '2' for convertion from UTF-32 to UTF-8: " );
	
	scanf( " %c", &convertionOption );
	
	// Input do nome do arquivo UNICODE a ser lido.
	printf( "\n\nDigite o nome do arquivo de entrada: " );
	
	scanf( " %s", fileName );
	
	// Abre arquivo para leitura.
	file = fopen ( fileName, "r" );
	
	if ( file == NULL )
	{
		printf( "ERROR: File %s cannot be opened.", fileName );
		exit(1);
	}
	
	// Abre arquivo para escrita dos bytes após conversão.
	fileOut = fopen ( "ConvertedFile.txt", "w" );
	
	if ( fileOut == NULL )
	{
		printf( "ERROR: File %s cannot be opened.", fileName );
		exit(1);
	}
	
	// Converte de UTF-8 para UTF-32.
	if ( convertionOption == '1' )
	{
	
		printf( "\n\nChoose 'L' for Little Endian or 'B' for Big Endian: " );
		scanf( " %c", &convertionOption );
			
		state = conv8_32 ( file, fileOut, convertionOption );

	}
	// Converte de UTF-32 para UTF-8.
	else if ( convertionOption == '2' )
	{

		state = conv32_8 ( file, fileOut );

	}
	// Opção inválida indicada pelo usuário.
	else
	{
	
		printf( "\n\nERROR: invalid option.\n\n" );
		fclose(file);
		fclose(fileOut);
		return 0;
	}

	fclose(file);
	fclose(fileOut);
	
	// Não ocorrreu erro durante a conversão.
	if( state == 0 )
		printf("\n\nConvertedFile created successfully.\n\n");
	
	// Ocorreu erro durante a conversão.
	else if( state == -1 )
		printf("\n\nConvertedFile created full of errors. Read report of errors above for more detail. \n\n");
	
	return 0;
}

