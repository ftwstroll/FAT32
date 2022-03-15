/*
    Name: Samuel Diaz
    ID:   1000833026

*/
    // The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_COMMAND_SIZE 255
#define WHITESPACE " \t\n"
#define TRUE 1
#define MAX_NUM_ARGUMENTS 5

    char    BS_OEMName[8];
    uint16_t BPB_BytesPerSec;
    uint8_t  BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t  BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    char    BS_VolLab[11];
    uint32_t BPB_FATSz32;
    uint32_t BPB_RootClus;
    uint32_t RootClusterAddress;

    uint32_t RootDirSectors = 0;
    uint32_t FirstDataSector = 0;
    uint32_t FirstSectorofCluster = 0;

    FILE *fp;

struct __attribute__((__packed__)) DirectoryEntry
{
    char    DIR_Name[11];
    uint8_t DIR_attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;

};

struct DirectoryEntry dir[16];


int LBAToOffset(int32_t sector)
{
    return (( sector - 2) * BPB_BytesPerSec) +(BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

int16_t NextLB( uint32_t sector)
{
    uint32_t FATAdress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + ( sector * 4);
    int16_t val;
    fseek(fp, FATAdress, SEEK_SET);
    fread( &val, 2, 1, fp);
    return val;
}

int main()
{

   char name[12];
  uint32_t current[50];
  int location = 0;
  int i;
  int found;

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while(TRUE)
  {
    // Print out the msh prompt
    printf ("mfc> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
          token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
          if( strlen( token[token_count] ) == 0 )
          {
            token[token_count] = NULL;
          }
            token_count++;
    }

    if (strcasecmp(token[0],"exit") == 0)
        break;
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //get lost in the code without these markers     OPEN
    if (strcasecmp(token[0],"open")== 0)
    {

        if (fp != NULL)
        {
            printf("Error: File system image already open");
        }
        else
        {

            fp = fopen(token[1],"r");

            if(!fp)
            {
                printf("Error: File system image not found");
            }
            else
            {
               // fseek(fp,3,SEEK_SET);
                //fread(&BS_OEMName,8,1,fp);
                fseek(fp,11,SEEK_SET);
                fread(&BPB_BytesPerSec,2,1,fp);
                fseek(fp,13,SEEK_SET);
                fread(&BPB_SecPerClus,1,1,fp);
                fseek(fp,14,SEEK_SET);
                fread(&BPB_RsvdSecCnt,2,1,fp);
                fseek(fp,16,SEEK_SET);
                fread(&BPB_NumFATs,1,1,fp);
                fseek(fp,17,SEEK_SET);
                fread(&BPB_RootEntCnt,2,1,fp);
                fseek(fp,36,SEEK_SET);
                fread(&BPB_FATSz32,4,1,fp);
                fseek(fp,44,SEEK_SET);
                fread(&BPB_RootClus,4,1,fp);
                fseek(fp,71,SEEK_SET);
                fread(&BS_VolLab,11,1,fp);


                RootClusterAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) +(BPB_RsvdSecCnt * BPB_BytesPerSec);
                current[0] = RootClusterAddress;
                location = 0;

              // printf("\nRootClusterAddress: %x %d",RootClusterAddress,BPB_FATSz32);

                fseek(fp,RootClusterAddress,SEEK_SET);
                for (i = 0; i < 16; i++)
                {
                    fread(&dir[i],sizeof(struct DirectoryEntry),1,fp);

                }


            }
        }

    }
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //Close
    if (strcasecmp(token[0],"close")== 0)
    {
        if (fp != NULL)
        {
            fclose(fp);
            fp = NULL;
        }
        else
            printf("Error: File system image must be opened first.");
    }

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //INFO
    if (strcasecmp(token[0],"info")== 0)
    {
        if(fp != NULL)
        {

            printf("BPB_BytesPerSec: %d %x",BPB_BytesPerSec, BPB_BytesPerSec);

            printf("\nBPB_SecPerClus: %d %x",BPB_SecPerClus , BPB_SecPerClus);

            printf("\nBPB_RsvdSecCnt: %d %x",BPB_RsvdSecCnt, BPB_RsvdSecCnt);

            printf("\nBPB_NumFats: %d %x",BPB_NumFATs, BPB_NumFATs);

            printf("\nBPB_FATSz32: %d %x",BPB_FATSz32, BPB_FATSz32);
        }
        else
            printf("Error: File system image must be opened first.");
    }

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //stat
    if (strcasecmp(token[0],"stat")== 0)
    {
        found = -1;
        int dot = -1;
        if (token[1] != NULL)
        {


            for (i = 0; i < strlen(token[1]); i++)
                if (token[1][i] == '.')
                    dot = i;

            if ( dot == -1)
            {



                for ( i = 0; i < 16; i++)
                    {
                        if ( dir[i].DIR_attr == 16)
                        {


                            memcpy(name,dir[i].DIR_Name,11);
                            name[11] = '\0';
                            int j;
                            for(j = 0; j < 12; j++)
                                if(name[j] == ' ')
                                    name[j] = '\0';


                            if(strcasecmp(name,token[1]) == 0)
                                found = i;

                        }
                    }

                if (found == -1)
                    printf("Directory not found.");
                else
                {
                    printf("Dir: %s\nAttribute: %d\nSize: 0\nStarting Cluster Marker: %d",name,dir[found].DIR_attr,dir[found].DIR_FirstClusterLow);
                }
            }
            else
            {

                for ( i = 0; i < 16; i++)
                {
                    if ( dir[i].DIR_attr == 32 || dir[i].DIR_attr == 1)
                    {
                        memcpy(name,dir[i].DIR_Name,11);
                        name[11] = '\0';

                        if(strcasecmp(name+8,token[1]+(strlen(token[1])-3)) == 0 && strncasecmp(name,token[1],strlen(token[1])-4) == 0)
                        {   found = i;
                            printf("File: %s\nAttribute: %d\nSize: %d\nStarting Cluster Marker: %d",name,dir[i].DIR_attr,dir[i].DIR_FileSize,dir[i].DIR_FirstClusterLow);
                        }



                        }
                    }


                if (found == -1)
                    printf("File not found.");


            }


        }
    }

    //@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //get
    if (strcasecmp(token[0],"get")== 0)
    {
        if(token[1] != NULL  )
        {


            int dot = -1;

            for (i = 0; i < strlen(token[1]); i++)
                if (token[1][i] == '.')
                    dot = i;

            if ( dot == -1)
            {
                printf("improper file name: %s",token[1]);

            }
            else
            {
                found = 0;
                for ( i = 0; i < 16; i++)
                {
                    if ( dir[i].DIR_attr == 32 || dir[i].DIR_attr == 1)
                    {
                        memcpy(name,dir[i].DIR_Name,11);
                        name[11] = '\0';

                        if(strcasecmp(name+8,token[1]+(strlen(token[1])-3)) == 0 && strncasecmp(name,token[1],strlen(token[1])-4) == 0)
                        {   found = 1;

                            //printf("%d ",dir[i].DIR_FirstClusterLow);
                            fseek(fp,LBAToOffset(dir[i].DIR_FirstClusterLow),SEEK_SET);


                            //fseek(pf,dir[i].DIR_FirstClusterLow,SEEK_SET);
                            uint8_t data;
                            FILE *fp2 = fopen(token[1],"w");

                            for ( i = 0; i < BPB_BytesPerSec; i++)
                            {
                                fread(&data,1,1,fp);
                                putc(data,fp2);
                            }

                           //int next = NextLB(dir[i].DIR_FirstClusterLow);

                           // printf("%d",next);
                          //  while ( next != -1)
                            {
                                /* fseek(fp,LBAToOffset(next),SEEK_SET);
                                for ( i = 0; i < BPB_BytesPerSec; i++)
                                {
                                    fread(&data,1,1,fp);
                                    putc(data,fp2);
                                }
                               next = NextLB(ffset(next));*/
                            }
                            fclose(fp2);
                        }



                        }

                    }


                if (!found)
                    printf("File not found.");


            }







        }
    }

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //CD
    if (strcasecmp(token[0],"cd")== 0)
    {






        if (strcmp(token[1],"..") == 0)
        {
            if(location == 0)
            {
                printf("You are in the root directory, so can't go back up.");
            }
            else
            {
                location -= 1;
                fseek(fp,current[location],SEEK_SET);
                for (i = 0; i < 16; i++)
                {
                    fread(&dir[i],sizeof(struct DirectoryEntry),1,fp);
                }
            }
        }

        else if (strncmp(token[1],"../",3) == 0 && location > 0)
        {
            fseek(fp,current[location-1],SEEK_SET);
            for (i = 0; i < 16; i++)
            {
                fread(&dir[i],sizeof(struct DirectoryEntry),1,fp);
                if (strcmp(dir[i].DIR_Name,token[1]+3) == 0)
                {
                    current[location] = LBAToOffset(dir[i].DIR_FirstClusterLow);

                }
            }


            fseek(fp,current[location],SEEK_SET);

            for (i = 0; i < 16; i++)
            {
                fread(&dir[i],sizeof(struct DirectoryEntry),1,fp);
            }

        }

        else
        {
            found = 0;
            for (i = 0; i < 16; i++)
            {
                if ( dir[i].DIR_attr == 16)
                {

                    memcpy(name,dir[i].DIR_Name,11);
                    name[11] = '\0';

                    if (strncasecmp(name,token[1],strlen(token[1])) == 0)
                    {
                        current[location+1] = LBAToOffset(dir[i].DIR_FirstClusterLow);
                        location += 1;
                        found = 1;
                    }
                }
            }

            if (found)
            {

                fseek(fp,current[location],SEEK_SET);
                for (i = 0; i < 16; i++)
                {
                     fread(&dir[i],sizeof(struct DirectoryEntry),1,fp);
                }
            }
        }




    }


    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    //ls
    if (strcasecmp(token[0],"ls")== 0)
    {

        if (location == 0)
        {

            for(i = 0; i < 16; i++)
            {
                if (dir[i].DIR_attr == 16 || dir[i].DIR_attr == 32 || dir[i].DIR_attr == 1)
                {
                    char name[12];
                    memcpy(name,dir[i].DIR_Name,11);
                    name[11] = '\0';
                    printf("%s\n",name);

                }
            }
        }
        else
        {

            for(i = 0; i < 16; i++)
            {
                if (dir[i].DIR_Name[0] != 0xe5)
                if (dir[i].DIR_attr == 0x10 || dir[i].DIR_attr == 0x20 || dir[i].DIR_attr == 0x01)
                {
                    char name[12];
                    memcpy(name,dir[i].DIR_Name,11);
                    name[11] = '\0';
                    printf("%s\n",name);

                }
            }
        }
    }


    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    // READ
    if (strcmp(token[0],"read")== 0)
    {
        if(token[1] != NULL && token[2] != NULL && token[3] != NULL)
        {

            printf("%s\n",token[1]);
            int dot = -1;

            for (i = 0; i < strlen(token[1]); i++)
                if (token[1][i] == '.')
                    dot = i;

            if ( dot == -1)
            {
                printf("improper file name: %s",token[1]);

            }
            else
            {
                found = 0;
                for ( i = 0; i < 16; i++)
                {
                    if ( dir[i].DIR_attr == 32 || dir[i].DIR_attr == 1)
                    {
                        memcpy(name,dir[i].DIR_Name,11);
                        name[11] = '\0';

                        if(strcasecmp(name+8,token[1]+(strlen(token[1])-3)) == 0 && strncasecmp(name,token[1],strlen(token[1])-4) == 0)
                        {   found = 1;

                            //printf("%d ",dir[i].DIR_FirstClusterLow);
                            fseek(fp,LBAToOffset(dir[i].DIR_FirstClusterLow),SEEK_SET);
                            //fseek(pf,dir[i].DIR_FirstClusterLow,SEEK_SET);
                            uint8_t data;

                            for (i = 0; i < atoi(token[2]); i++)
                                fread(&data,1,1,fp);

                            for ( i = 0; i < atoi(token[3]); i++)
                            {
                                fread(&data,1,1,fp);
                                printf("%x ",data);
                            }
                           /* int16_t next = NextLB(dir[i].DIR_FirstClusterLow);

                            printf("(%d)",next);
                            while ( next != -1)
                            {
                                printf("%d ",next);
                                next = NextLB(dir[i].DIR_FirstClusterLow);
                            }*/

                        }



                        }
                    }


                if (!found)
                    printf("File not found.");


            }







        }


    }


    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(easier to find markers)
    // volume check
    if (strcasecmp(token[0],"volume")== 0)
    {
        if(fp != NULL)
            if(strcmp(BS_VolLab,"NO NAME    ") == 0)
                printf("Error: volume name not found.");
            else
            {

                char name[12];
                memcpy(name,BS_VolLab,11);
                name[11] = '\0';
                printf("Volume Name: \"%s\"",name);
            }
        else
           printf("Error: File system image must be opened first.") ;
    }
























        free( working_root );
        printf("\n");
    }




    return 0;

}
