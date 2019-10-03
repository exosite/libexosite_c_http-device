/*****************************************************************************
*
*  exosite_pal.c - Exosite hardware & environmenat adapation layer.
*  Copyright (C) 2012-2017 Exosite LLC
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Exosite LLC nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
//#include "exosite.h"
#include "exosite_pal.h"
#include "exosite_pal_private.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include <stdlib.h> //for exit(0);
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h> //For errno - the error number
#include <netdb.h>   //hostent
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


char hostname[MAX_HOSTNAME_SIZE];;
static int sockfd = 0, n = 0;
char recvBuff[1024];
pthread_t listening_thread_id;
static char writeBuff[1024];
static int writeLen;


/*
 * Private
 */
int hostname_to_ip(char * hostname , char* ip);


/*!
 * Buffer for rx data
 */
char  exoPal_rxBuffer[RX_BUFFER_SIZE];

/*!
 * Used to track how far into the rx buffer we are
 */
static uint16_t bufferCount=0;

/*!
 * Used to track if we currently have a socket open
 */
static uint8_t isSocketOpen = 0;

// local functions

/*!
 * Stores a bunch of stuff to emulate nvm and manipulate return results
 */
struct UnitTest_storage mem_nvm;


/*!
 * \brief Retrieves a pointer to the UnitTest_storage struct
 *
 * This allows external applications speciically unit tests, to manipulate
 * the way the the hal behaves.
 *
 *
 * \return A pointer to the UnitTest_storage
 * \sa
 * \note
 * \warning
 */
void * getUnitTestStorageStruct()
{
    return &mem_nvm;
}


/*!
 * \brief Determines the length of a null terminated string
 *
 * You can write your own, or link to one that is available.
 * This does not include the trailing '\0'. for example, strlen("") == 0 and
 * strlen("A") == 1
 *
 * \param[in] s String you want to determine the length of
 *
 * \return The length of the inputted string.
 * \sa
 * \note
 * \warning  There is no protection for overrun here... Be Careful!
 */
uint16_t exoPal_strlen(const char *s)
{
    return strlen(s);
}

/*!
 * \brief Gets the decimal ascii representation of an integer
 *
 *
 *
 * \param[in] value Number that you want a string representation of
 * \param[in] buf Buffer to place results
 * \param[in] bufSizelength of buffer

 * \return Length of string written to buf
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_itoa(int value, char* buf, uint8_t bufSize)
{
    int32_t chars = sprintf(buf, "%d", value);
    if (chars < 0)
    {
        return 0;
    }
    return chars;

}

int32_t exoPal_atoi(char* val)
{
	return atoi(val);
}

/*!
 * \brief A function to copy from one place in memory to another
 *
 * You can write your own or call another.
 *
 * \param[in] dst Memory location of where to write to
 * \param[in] src Memory location of where to copy from
 * \param[in] length Number of bytes to copy
 *
 * \return 0 if failed, else returns dst address.
 * \sa
 * \note
 * \warning
 */
void * exoPal_memcpy(void* dst, const void * src, uint16_t length)
{
    return memcpy(dst,src,length);
}

/*!
 * \brief Closes a tcp socket
 *
 *
 *
 * \param[in]
 * \param[out]
 * \return errorcode if failed, else a 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_tcpSocketClose()
{
    return close(sockfd);
}

/*!
 * \brief Opens a tcp socket
 *
 *
 *
 * \param[in]
 * \param[out]
 * \return errorcode if failed, else a 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_tcpSocketOpen()
{
    //Get IP Address
    char ip[100];
    exoPal_getHostname(hostname);
    int ret = hostname_to_ip(hostname, ip);
    printf("Got IP: %s\n", ip);

    struct sockaddr_in serv_addr; 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    //serv_addr.sin_port = htons(443); 
    serv_addr.sin_port = htons(80); 

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    printf("connecting...\n");
    return connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0;
}




/*!
 * \brief Sends data to the open tcp socket
 *
 *
 *
 * \param[in] buffer Data to write to socket
 * \param[in] len Length of data to write to socket
 *
 * \return errorcode if failed, else a 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_socketWrite( const char * buffer, uint16_t len)
{
    strcat(writeBuff, buffer);
    writeLen += len;
    return 0;
}


/*!
 * \brief
 *
 *
 *
 * \param[in] bufferSize Size of buffer
 * \param[out] buffer Buffer received data will be written to
 * \param[out] responseLength amount of data received from modem
 *
 * \return errorcode if failed, else a 0
 * \sa
 * \note len must be greater than sizeof(buffer)
 * \warning
 */
uint8_t exoPal_socketRead( char * buffer, uint16_t bufSize, uint16_t * responseLength)
{
    n = recv(sockfd, buffer, bufSize, MSG_WAITALL);
    buffer[n] = 0;
    if (n == 0)
        n = recv(sockfd, buffer, bufSize, MSG_WAITALL);

    *responseLength = n;

    if(fputs(buffer, stdout) == EOF)
    {
        printf("\n Error : Fputs error\n");
        return 1;
    }

    return 0;
}




/*!
 * \brief Delay function
 *
 *
 *
 * \param[in] delay Delay time in ms
 *
 * \return
 * \sa
 * \note
 * \warning
 */
void exoPal_MSDelay(uint16_t delay)
{
    //MSTimerDelay(delay);
    return;
}


/*!
 * \brief
 *
 * Any HW or SW initialization should be performed in here
 *
 * This function is meant to perform any one time initialization and/or setup.
 * This will be called everytime Exosite_init is called.  An example use for
 * this function would be to perform a DNS lookup of m2.exosite.com
 *
 */
void exoPal_init()
{
    return;

}


/*!
 * \brief Sets the cik
 *
 * Writes the 40 chars starting at cik* to nvm.
 *
 * \param[in] cik cik to write to nvm
 *
 * \return errorcode if successful else 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_setCik(const char * cik)
{
    FILE *fp;
    fp = fopen("cik.txt", "w");
    if (fp == NULL) return -1;

    fprintf(fp, "%s", cik);
    fclose(fp);

    return 0;
}


/*!
 * \brief Retrieves the stored CIK
 *
 *
 *
 * \param[out] read_buffer pointer of buffer to place results in
 *
 * \return 0 if successful, else errorCode
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_getCik(char * read_buffer)
{
    FILE *fp;
    fp = fopen("cik.txt", "r");
    if (fp == NULL) return -1;
    fscanf(fp, "%s", read_buffer);
    printf("GOT CIK from file: %s\n", read_buffer);
    fclose(fp);
    return 0;
}

uint8_t exoPal_getHostname(char* read_buffer)
{
    FILE *fp;
    fp = fopen("hostname.txt", "r");
    if (fp == NULL) {printf("couldn't find hostname.txt"); return -1;}
    fscanf(fp, "%s", read_buffer);
    printf("GOT hostname from file: %s\n", read_buffer);
    fclose(fp);
    return 0;
}

/*!
 * \brief Retrieves UUID from device
 *
 *	This function retrieves a unique ID from your device.  This is typically
 *	the MEID of a cell modem, MAC address of a network card, or serial number
 *	of the device.
 *
 * \param[in] UUID_buf Buffer to put the devices UUID into.
 * \param[out]
 * \return 1 if failed to retrieve UUID, else 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_getUuid(char * read_buffer)
{
    FILE *fp;
    fp = fopen("uuid.txt", "r");
    if (fp == NULL) {printf("couldn't find uuid.txt"); return -1;}
    fscanf(fp, "%s", read_buffer);
    printf("GOT uuid from file: %s\n", read_buffer);
    fclose(fp);
    return 0;
}

/*!
 * @brief  Used to do any operations before
 *
 *
 * @return Returns 0 if successful, else error code
 */
int32_t exoPal_sendingComplete()
{
    writeBuff[writeLen] = '\0';
    writeLen++;
    printf("WriteBuff:\n%s\n", writeBuff);
    printf("WriteLen: %d\n", writeLen);
    int written =  write(sockfd, writeBuff, writeLen);
    printf("Wrote: %d\n", written);
    writeLen = 0;
    writeBuff[0] = '\0';
    if (written > 0)
        return 0;
    else
        return written;
}

char* exoPal_strstr(const char *in, const char *str)
{
	char c;
	size_t len;

	c = *str++;
	if (!c)
		return (char *) in;	// Trivial empty string case

	len = strlen(str);
	do {
		char sc;

		do {
			sc = *in++;
			if (!sc)
				return (char *) 0;
		} while (sc != c);
	} while (strncmp(in, str, len) != 0);

	return (char *) (in - 1);
}

/*
    Get ip from domain name
 */
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he-> h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }

    return 1;
}



/* vim: set ai cin et sw=4 ts=4 : */
