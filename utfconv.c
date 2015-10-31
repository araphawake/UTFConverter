/* Rafael Azevedo Moscoso Silva Cruz 1221020 3WB */
/* Felipe Zarattini Miranda 1220557 3WB */

#include <stdio.h>
#include <stdlib.h>

#include "utf.h"

// Variáveis para identificação de Big Endian e de Little Endian.
static unsigned int bigEndian 		= 0xFFFE0000;
static unsigned int littleEndian 	= 0x0000FEFF;

// Tipo de estado da conversão entre UTF-8 e UTF-32.
typedef enum
{

	ERROR_READ_FILE,
		// Erro de abertura do arquivo para leitura.
		
	ERROR_WRITE_FILE,
		// Erro de abertura do arquivo para escrita.
		
	ERROR_INVALID_BYTE,
		// Erro de leitura de byte. Byte lido não é válido.
		
	ERROR_INVALID_FIRSTBYTE,
		// Erro de leitura do primeiro byte em UTF-8. Byte lido não é válido.
		
	ERROR_INVALID_ENDIAN,
		// Erro de Endian, ordem de armazenamento. Endian lido não é válido.
		
	ERROR_INCOMPATIBLE_ENDIAN,
		// Erro de Endian incompatível, a ordem de armazenamento dos bytes
		// nãpo corresponde à ordem indicada pelo BOM.
		 
	ERROR_INVALID_BOM,
		// Erro de BOM lido. BOM é inválido.
		
	ERROR_QUANTITY_BYTES,
		// Erro de quantidade de bytes lidos. A quantidade de bytes esperada 
		// para leitura é menor que a quantidade de bytes disponíveis para leitura no arquivo.
		
	ERROR_MISSING_BOM,
		// Erro de BOM inexistente no arquivo UTF-32, arquivo de leitura.
		
	SUCCESS
		// Nenhum erro foi encontrado.
		
} STATE_tpReturn;


// Controle e classificação de erros que ocorrem durante a conversão entre UTF-32 e UTF-8.
int getError( STATE_tpReturn state )
{

	switch( state )
	{
	
		case ERROR_READ_FILE:
		
			fprintf( stderr, "\nErro de abertura do arquivo para leitura. " );
			break;
		
		case ERROR_WRITE_FILE:
		
			fprintf( stderr, "\nErro de abertura do arquivo para escrita. " );
			break;
			
		case ERROR_INVALID_BYTE:
		
			fprintf( stderr, "\nErro de byte inválido lido do arquivo de entrada. " );
			break;
		
		case ERROR_INVALID_FIRSTBYTE:
		
			fprintf( stderr, "\nErro de primeiro byte de sequência de bytes inválido em arquivo UTF-8." );
			fprintf( stderr, " Deveria ser 0xxxxxxx ou 110xxxxx ou 1110xxxx ou 11110xxx. " );
			break;
	
		case ERROR_INVALID_ENDIAN:
			
			fprintf( stderr, "\nErro de Endian inválido. " );
			break;
		
		case ERROR_INVALID_BOM:
		
			fprintf( stderr, "\nErro de BOM inválido. Deveria ser 0xFFFE0000 ou 0x0000FEFF." );
			break;
			
		case ERROR_QUANTITY_BYTES:
		
			fprintf( stderr, "\nErro de quantidade de bytes menor que aquela que deveria ser lida. " );
			break;
			
		case ERROR_MISSING_BOM:
		
			fprintf( stderr, "\nErro de arquivo UTF-32. Falta BOM no início do arquivo. " );
			break;
		
		case ERROR_INCOMPATIBLE_ENDIAN:
			fprintf( stderr, "\n\nErro de armazenamento de bytes incompatível com o BOM do arquivo." );
			break;
			
		case SUCCESS:
		
			return 0;
			break;
			
	};
	
	return -1;

}

// Imprime o caracter BOM no arquivo em UTF-32
STATE_tpReturn imprimeBOM( FILE * arq_saida, char ordem )
{

	if( !arq_saida )
		return ERROR_WRITE_FILE;
	
	// Caso a ordem seja de Big Endian.
	if ( ordem == 'B' )
	{
		fwrite( &bigEndian, sizeof(unsigned int), 1, arq_saida );
	}
	
	// Caso a ordem seja de Little Endian
	else if ( ordem == 'L' )
	{
		fwrite( &littleEndian, sizeof(unsigned int), 1, arq_saida );
	}
	
	// Caso a ordem não seja Big Endian ou Little Endian, retorno de erro -1.
	else
	{
		return ERROR_INVALID_ENDIAN;
	}
	
	return SUCCESS;
	
}


// Avalia o primeiro byte de uma sequência de bytes de um arquivo em UTF-8.
// Retorna a quantidade de bytes que devem ser lidos em seguida.
// Retorna a quantidade de bytes que formam a sequência de bytes de um caracter
// de um arquivo UTF-8.
STATE_tpReturn getNumberBytesUTF8Symbol( unsigned char firstByte, int * numberBytes )
{
	
	// Primeiro byte 11110000
	// Máscara & 11111000
	if ( (firstByte & 0xF8) == 0xF0 )
	{
		*numberBytes = 4;
		return SUCCESS;
	}
	
	// Primeiro byte 11100000
	// Máscara & 11110000
	if ( (firstByte & 0xF0) == 0xE0 )
	{
		*numberBytes = 3;
		return SUCCESS;
	}
	
	// Primeiro byte 11000000
	// Máscara & 11100000
	if ( (firstByte & 0xE0 ) == 0xC0 )
	{
		*numberBytes = 2;
		return SUCCESS;
	}
	
	// Primeiro byte 00000000
	// Máscara & 10000000
	if ( (firstByte & 0x80) == 0x0 )
	{
		*numberBytes = 1;
		return SUCCESS;
	}
	
	// ERROR: primeiro byte não tem os bits iniciais válidos.
	// Primeiro byte deveria ser: 0xxxxxxx ou 110xxxxx ou 1110xxxx ou 11110xxx.
	*numberBytes = -1;
	
	return ERROR_INVALID_FIRSTBYTE;
	
}


// Imprime uma sequência de 4 bytes (UTF-32) após conversão de um caracter 
// de UTF-8 para UTF-32.
/*
 *	Parametros: bytes 	  - Vetor de 4 bytes tal que a primeira posição do vetor 
 *							tenha o byte mais significativo e a quarta posição 
 *							do vetor tenha o byte menos significativo.
 *				ordem 	  - Ordem de armazenamento ('B' ou 'L').
 *				arq_saida - arquivo de escrita dos bytes.
 *
 */
int imprimeSimboloUTF32( unsigned char * bytes, char ordem, FILE * arq_saida )
{

	int i = 0;

	if( !arq_saida )
		return ERROR_WRITE_FILE;
	
	// Se Little Endian, byte mais significativo com endereço mais alto.
	// O vetor de bytes deve ser impresso em ordem invertida.
	if ( ordem == 'L' )
	{
		
		for( i = 3; i >= 0; i-- )
			fwrite( &bytes[i], 1, 1, arq_saida );
			
	}
	
	// Se Big Endian, byte mais significativo com endereço mais baixo.
	// O vetor de bytes deve ser impresso em ordem crescente de posição.
	else if( ordem == 'B' )
	{
	
		for( i = 0; i < 4; i++ )
			fwrite( &bytes[i], 1, 1, arq_saida );
	}
	
	// ERRO: Ordem inválida.
	else
	{
		return ERROR_INVALID_ENDIAN;
	}

	return SUCCESS;
	
}


// Lê as sequências de bytes em UTF-8 e converte os caracteres lidos do arquivo 
// em UTF-8 para caracteres correspondentes em arquivo de saída UTF-32.

STATE_tpReturn convertFile8_32( unsigned char * bytes, FILE * arq_entrada, FILE * arq_saida, char ordem )
{

	int numeroBytes = 0, i = 0; 
	int j = 0, check = 0;
	STATE_tpReturn state = SUCCESS;
	unsigned char firstByte;		// Primeiro byte da sequência em UTF-8.
	unsigned char byte;				// Variável auxiliar.
	
	if( !arq_entrada )
		return ERROR_READ_FILE;
	
	// Enquanto um byte de inicio de sequência em UTF-8 for lido, continuar lendo
	// sequências do aruqivo de entrada em UTF-8.
	while ( fread( &firstByte, sizeof(unsigned char), 1, arq_entrada ) )
	{
		
		// Retorna o númeor de bytes da sequência.
		state = getNumberBytesUTF8Symbol( firstByte, &numeroBytes );			

		// Caso a sequência tenha um único byte, não é necessário conversão.
		if( numeroBytes == 1 )
		{
			bytes[0] = firstByte;
			bytes[3] = bytes[2] = bytes[1] = 0x0;
		}
		
		// Caso a sequência tenha mais de um byte, eliminar os bits de controle
		// do primeiro byte da sequência em UTF-8.
		else if ( numeroBytes > 1 )
		{
			// Shift para eliminar os primeiros bits de inicío de sequência.
			firstByte = ( firstByte << ( numeroBytes + 1 ) );
			firstByte = ( firstByte >> ( numeroBytes + 1 ) );
			
			bytes[0] = firstByte;
			bytes[3] = bytes[2] = bytes[1] = 0x0;
		}
		
		// ERROR: primeiro byte não tem os bits iniciais válidos.
		else if(numeroBytes == -1)
		{
			return state;
		}
		
		// Lê os bytes, do arquivo de entrada em UTF-8, da sequência corrente. 
		for ( i = 1; i < numeroBytes; i++ )
		{
	
			if( ! fread( &byte, sizeof(unsigned char), 1, arq_entrada )  )
			{
				return ERROR_QUANTITY_BYTES;
			}
			
			if( (byte & 0xC0) != 0x80  )
			{
				return ERROR_INVALID_BYTE;
			}
			
			// Máscara & 00111111.
			// Elimina os bits de continuação 10.
			byte = ( byte & 0x3F );
			
			bytes[i] = byte;		
			
		}

		// Converte eliminando os bits 00 iniciais de todos os bytes da sequência
		// já alterados na etapa anterior. Percorre os bytes de trás para frente.
		for ( i = numeroBytes-1; i > 0; i-- )
		{
		
			// Salva os bits do byte anterior ao corrente apenas com os bits que
			// devem ser colocados nos bits nulos iniciais do byte corrente.
			// Para tal utiliza-se também a operação | entre o byte corrente e
			// o byte recuperado.
			byte = ( bytes[i-1] << ( 6 - ( 2 * ( ( numeroBytes - 1 ) - i ) ) ) ); 
			bytes[i] = ( bytes[i] | byte );
			
			// Elimina os bits utilizados do byte anterior ao byte corrente
			// para evitar duplicata de bits (bits repetidos).
			bytes[i-1] = (bytes[i-1] >> ( 2 + 2 * ( ( numeroBytes - 1 ) - i ) ) );
				
		}

		// Faz shift do byte convertido para pôr todos os bytes nulos em excesso
		// no fim do vetor no início do vetor. Os valores do vetor devem ser 
		// movidos tantas vezes para a esquerda (maior posição) quanto for o 
		// número de bytes da sequência.
			
		for( i = 3; i >= numeroBytes; i-- )
		{

			// Movimento dos bytes no vetor.
			if ( bytes[3] == 0x0 )
			{
				bytes[3] = bytes[2];
				bytes[2] = bytes[1];
				bytes[1] = bytes[0];
				bytes[0] = 0x0;
			} 

		}
		
		// Imprime sequência de 4 bytes em UTF-32 no arquivo de saída.
		state = imprimeSimboloUTF32( bytes, ordem, arq_saida );
		
	}
	
	return state;
	
}

// Converte arquivos em UTF-8 para arquivos em UTF-32.
int conv8_32(FILE *arq_entrada, FILE *arq_saida, char ordem)
{

	unsigned char bytes[4];
	STATE_tpReturn state = SUCCESS;
	
	// Imprime o BOM no arquivo.
	state = imprimeBOM( arq_saida, ordem );
	
	// Trata erros retornados.
	if ( getError( state ) == -1 )
	{
		return -1;
	}
	
	// Lê e converte arquivo de UTF-8 para UTF-32. Retorna possíveis erros.
	state = convertFile8_32( bytes, arq_entrada, arq_saida, ordem );
	
	// Trata erros retornados.
	if ( getError( state ) == -1 ) 
	{
		return -1;
	}
	
	return 0;

}


// Lê o byte BOM de um arquivo UTF-32 e retorna a ordem de armazenamento
// seja Big endian ou Little Endian.
STATE_tpReturn readBOM( int firstByte, char * bom )
{

	if( firstByte == bigEndian )
	{
		*bom = 'B';
	}
	else if( firstByte == littleEndian )
	{
		*bom = 'L';
	}
	else
	{
		*bom = 'E';
		return ERROR_INVALID_BOM;
	}
	
	return SUCCESS;
	
}


// Converte sequência de bytes em UTF-32 para sequência em UTF-8.
STATE_tpReturn convertBytes32_8( unsigned char * bytes, FILE * arq_saida, char ordem )
{

	unsigned char byte = 0x0;
	unsigned char lostBits[4];
	int backshift = 0, j = 0, i = 0, controlByte = 0;
	
	if( !arq_saida )
		return ERROR_WRITE_FILE;
	
	// Salva os bits a serem perdidos em substituição dos bits pelos bits de continuação 10.
	lostBits[3] = (bytes[3] ) >> 6;
	lostBits[2] = (bytes[2] ) >> 4;
	lostBits[1] = (bytes[1] ) >> 2;
	
	// Percorre o vetor até encontrar um byte não nulo.
	for( i = 0;  i < 4; i++ )
	{
		// Caso encontre byte não nulo, guarda a posição anterior à corrente.
		// Sai do for com i = 5 + 1 = 6.
		// j possui a posição do primeiro caracter nulo antes do primeiro caracter
		// não nulo.
		if( bytes[i] != 0x0 )
		{			
			j = i - 1;
			i = 5;
		}

	}

	// Caso nenhum caracter não nulo tenha sido encontrado no vetor de bytes.
	// Imprime portanto 4 bytes nulos.
	if ( i != 6 )
	{
		fwrite( &byte, 1, 1, arq_saida );
		return SUCCESS;
		
	}
	
	// Lida com os bytes do vetor desde a última posição até a posição j
	for( i = 3;  i >= j ; i--)
	{
	
		// Avalia se o byte anterior ao byte da posição j pode ser
		// o primeiro byte da sequência de bytes em UTF-8.
        if ( (i - 1) == j )
        {
			
			// Verifica se há espaço para os bits que indicam quantidade de bytes
			// no primeiro byte da sequência de bytes do caracter em UTF-8.
			// Verifica a faixa em que o byte se encontra.
			// Realiza as operações de shift, OR e AND para encaixar o byte no formato correspondente da UTF-8 de acordo com sua faixa
			// e passa os bits anteriormente salvos na lostBits para o byte anterior.			    
	 	// Caso após a conversão o caracter tenha apenas 1 byte.
	    if ( (  (bytes[i] >= 0x00) && (bytes[i] <= 0x7F) ) && j == 2 )
        	{
		        bytes[i] = bytes[i] & ( ( 0xFF << 1 ) >> 1 );
		    	controlByte = 1;
		    	i = j - 2;
		   	}
           		
         // Caso após a conversão o caracter tenha apenas 2 byte.  		
		 else if( ( (bytes[i] > 0x0) && (bytes[i] <= 0x07) ) && j == 1 )
			{
				bytes[i] = ( bytes[i] << ( 2 * (3 - i) ) );
				bytes[i] = ( bytes[i] | lostBits[i+1] );
				
	        		bytes[i] = bytes[i] & ( ( 0xFF << 3 ) >> 3 );
		    		bytes[i] = bytes[i] | ( ( 0xFF >> 6 ) << 6 );
				controlByte = 1;
				i = j - 2;
			
			}
			
			// Caso após a conversão o caracter tenha apenas 3 byte.
			else if( ( (bytes[i] <= 0xFF) && (bytes[i] >= 0x08) ) && j == 0 )
			{
				bytes[i] = ( bytes[i] << ( 2 * (3 - i) ) );
				bytes[i] = ( bytes[i] | lostBits[i+1] );
				
	        	bytes[i] = bytes[i] & ( ( 0xFF << 4 ) >> 4 );
		    	bytes[i] = bytes[i] | ( ( 0xFF >> 5 ) << 5 );
				controlByte = 1;
				i = j - 2;

			}
			// Caso o byte nulo seguinte seja o primeiro byte da sequência de bytes
			// do caracter convertido para UTF-8.
			else
            {
            	// Guarda o último byte não nulo do vetor em ordem crescente de
            	// de indice.
            	backshift = bytes[i];
               
               	// Elimina/zera os dois bits à esquerda.
               	bytes[i] = ( bytes[i] << ( 2 * (3 - i) ) );
			
				// Caso não seja o byte não nulo de maior indice,
				// evita que a posição 4 do vetor seja requisitada. Essa posição
				// não existe. 
				if( i != 3 )
					bytes[i] = ( bytes[i] | lostBits[i+1] ); 
               
               	// Os dois primeiros bits do byte são transformados para 10.
               	// 10 é o caracter de continuação.
			    bytes[i] = (bytes[i] & 0x3F);
			    bytes[i] = (bytes[i] | 0x80);
           
            }


     	}
     	
        else
        {
        
			// Elimina/zera os dois bits à esquerda.
			bytes[i] = ( bytes[i] << ( 2 * (3 - i) ) );
			
			// Caso não seja o byte não nulo de maior indice,
			// evita que a posição 4 do vetor seja requisitada. Essa posição
			// não existe. 
		    if( i != 3 )
			    bytes[i] = ( bytes[i] | lostBits[i+1] );

			// Os dois primeiros bits do byte são transformados para 10.
            // 10 é o caracter de continuação.
            // Caso o byte seja o candidato à primeiro byte da sequência de bytes em UTF-8,
            // os dois primeiros bytes não são transformados para 10. Devem continuar sendo
            // nulos.
			if ( i != j )
			{
		    	bytes[i] = (bytes[i] & 0x3F);
		    	bytes[i] = (bytes[i] | 0x80);
		    }
		    
		}
		
	}

	// Verifica se a sequência de bytes em UTF-8 já tem um byte de início de sequência.
	// Caso não tenha, o byte na posição j é transformado no byte de início de sequência
	// de bytes de um caracter em UTF-8.
	if ( controlByte == 0 )
	{
	        
	    bytes[j] = bytes[j] & ( ( 0xFF << ( 4 - j + 1 ) ) >> ( 4 - j + 1 ) );
		bytes[j] = bytes[j] | ( ( 0xFF >> ( 8 - ( 4 - j ) ) ) << ( 8 - ( 4 - j ) ) );
	        
	}
	
	// Imprime apenas os bytes não nulos, que formam a sequência de bytes de um caracter
	// em UTF-8 após conversão completa.
	for( i = 0; i < 4 ; i++ )
	{
		if ( bytes[i] != 0x0 )
			fwrite( &bytes[i], 1, 1, arq_saida );	
	}

}


// Lê sequências de 4 bytes de um arquivo UTF-32 e converte o arquivo para UTF-8.
STATE_tpReturn readConvertBytes32_8( unsigned char * bytes, FILE * arq_entrada, FILE * arq_saida )
{

	char ordem;
	int firstByte = 0, i = 0;
	STATE_tpReturn state = SUCCESS;
	
	// Lê o primeiro byte, o BOM.
	if ( ! fread( &firstByte, sizeof(int), 1, arq_entrada ) )
	{
	
		return ERROR_MISSING_BOM;
	
	}
	

	// Obtém a classificação do BOM como Little Endian ('L') ou Big Endian ('B').
	state = readBOM( firstByte, &ordem );
	
	if( state != SUCCESS )
		return state;
	
	// Lê de 4 em 4 bytes do arquivo de entrada em UTF-32.
	while( fread( &bytes[i], 1, 1, arq_entrada ) )
	{
		// Caso a ordem seja Big Endian, os bytes devem ser lidos e armazenados
		// da menor posição do vetor para a maior posição do vetor para já ser
		// compatível com a ordem de armazenamento em UTF-8.
		if( ordem == 'B' )
		{
		
			for( i = 1; i < 4; i++ )
			{
		
				if( ! fread( &bytes[i], 1, 1, arq_entrada ) )
				{
					return ERROR_QUANTITY_BYTES;
				}
				
				
			}
		
		}
		
		// Caso a ordem seja Little Endian, os bytes lidos devem ser armazenados
		// da maior posição do vetor para a menor posição do vetor. Assim, a ordem
		// dos bytes já é compatível com a ordem de armazenamento em UTF-8.
		else if( ordem == 'L' )
		{
		
			bytes[3] = bytes[0];
		
			for( i = 2; i >= 0; i-- )
			{
				
				if( ! fread( &bytes[i], 1, 1, arq_entrada ) )
				{
					return ERROR_QUANTITY_BYTES;
				}
			
			}
			
		
		}
		// ERRO: Ordem de armazenamento inválida.
		//else
		//{
		//	return ERROR_INVALID_ENDIAN;
		//}
		
		//if( bytes[3] == 0x0 && ( bytes[1] != 0x0 || bytes[2] != 0x0 )  )
		//{
		
		//	return ERROR_INVALID_BYTE;
			
		//}
	
		//if( bytes[0] != 0x0 && ( bytes[1] != 0x0 || bytes[2] != 0x0 ) && ordem == 'L' )
		//{
		
		//	return ERROR_INCOMPATIBLE_ENDIAN;
			
		//}
			
		
		// Converte os bytes de UTF-32 para UTF-8. E imprime em arquivo de saída
		// o resultado.
		convertBytes32_8( bytes, arq_saida, ordem );
		
		// Zera o contador do vetor para leitura de nova sequência de 4 bytes
		// do arquivo de entrada em UTF-32.
		i = 0;
		
	}
	
	return SUCCESS;
	
}


// Converte arquivos em UTF-32 para arquivos em UTF-8.
int conv32_8(FILE *arq_entrada, FILE *arq_saida)
{

	unsigned char bytes[4];
	STATE_tpReturn state = SUCCESS;
	
	// Zera os bytes para que não haja lixo durante a execução.
	bytes[0] = bytes[1] = bytes[2] = bytes[3] = 0; 
	
	// Lê e converte bytes de UTF-32 para UTF-8.
	state = readConvertBytes32_8( &bytes[0], arq_entrada, arq_saida );
	
	// Retorna erro ou sucesso da função, retorno 0 para sucesso e retorno -1 para erro.
	return ( getError( state ) );

}














































