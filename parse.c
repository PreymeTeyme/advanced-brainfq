#include "brainfq.h"

int parse(uint8_t ***const code, const char *const source, const int sourcesize)
{
    int funamount=0, cmdamount=0, *loop_stack=NULL, loop_depth=0;
    bool infunction=false;

    *code= NULL;

    for(int i=0; i<sourcesize; i++)                                                             //iterate through source file
    {
        if(source[i]==':')                                                                      //check for function indicator
        {
            infunction=!infunction;

            if(infunction)                                                                      //if new function is reached
            {
                funamount++;

                *code= realloc(*code, funamount * sizeof(uint8_t*));                            //generate new function array

                if(*code!=NULL)                                                                 //check for allocation error
                {
                    (*code)[funamount-1]= NULL;
                }
                else
                {
                    printf("[@Error] Memory allocation error, for new byte code function amount.\n");
                    return 0;
                }
            }

            if(!newbyte(&((*code)[funamount-1]), &cmdamount))                                   //add function indicator to function array
            {
                return 0;
            }
            (*code)[funamount-1][cmdamount-1]= 9;

            if(!infunction)                                                                     //add end of string symbol to end of function array
            {
                if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                {
                    return 0;
                }
                (*code)[funamount-1][cmdamount-1]= '\0';
            }
        }
        else
        {
            if(infunction)                                                                      //if currently in a function
            {
                switch(source[i])                                                               //enter main state machine
                {

                case '>':                                                                       //new pointer operator
                    if(!newbyte(code[funamount-1], &cmdamount))                                 //add command
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 1;
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))                           //add data byte
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= repaeting(source, &i, sourcesize, '>');  //count the amount commands
                    i--;
                    break;

                case '<':
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 2;
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= repaeting(source, &i, sourcesize, '<');
                    i--;
                    break;

                case '+':
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 3;
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= repaeting(source, &i, sourcesize, '+');
                    i--;
                    break;

                case '-':
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 4;
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= repaeting(source, &i, sourcesize, '-');
                    i--;
                    break;

                case '[':                                                                       //new loop
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))                           //add command byte
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 5;

                    loop_depth++;                                                               //add loop stack element
                    loop_stack= (int*) realloc(loop_stack, loop_depth * sizeof(int));
                    if(loop_stack==NULL)                                                        //check for allocation error
                    {
                        printf("[@Error] Memory allocation error, for new loop stack element during parsing.\n");
                        return 0;
                    }

                    loop_stack[loop_depth-1]= cmdamount-1;                                      //push begin of the loop to the loop stack

                    if(!newquadbyte(&((*code)[funamount-1]), &cmdamount))                       //add data integer
                    {
                        return 0;
                    }
                    break;

                case ']':                                                                       //end of loop
                    (*code)[funamount-1][loop_stack[loop_depth-1]+1]= ((uint8_t*)&cmdamount)[0];//save end position of loop to begin of loop command
                    (*code)[funamount-1][loop_stack[loop_depth-1]+2]= ((uint8_t*)&cmdamount)[1];
                    (*code)[funamount-1][loop_stack[loop_depth-1]+3]= ((uint8_t*)&cmdamount)[2];
                    (*code)[funamount-1][loop_stack[loop_depth-1]+4]= ((uint8_t*)&cmdamount)[3];

                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))                           //add command byte
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 6;

                    if(!newquadbyte(&((*code)[funamount-1]), &cmdamount))                       //add data integer
                    {
                        return 0;
                    }

                    (*code)[funamount-1][cmdamount-4]= ((uint8_t*)(loop_stack+loop_depth-1))[0];//save begin position of loop to end of loop command
                    (*code)[funamount-1][cmdamount-3]= ((uint8_t*)(loop_stack+loop_depth-1))[1];
                    (*code)[funamount-1][cmdamount-2]= ((uint8_t*)(loop_stack+loop_depth-1))[2];
                    (*code)[funamount-1][cmdamount-1]= ((uint8_t*)(loop_stack+loop_depth-1))[3];

                    loop_depth--;                                                               //pop stack
                    loop_stack= (int*) realloc(loop_stack, loop_depth * sizeof(int));
                    break;

                case '.':
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 7;
                    break;

                case ',':
                    if(!newbyte(&((*code)[funamount-1]), &cmdamount))
                    {
                        return 0;
                    }
                    (*code)[funamount-1][cmdamount-1]= 8;
                    break;

                default:                                                                        //unknown state machine value
                    printf("[@Error] Character out of context: '%c'.\n", source[i]);
                    break;
                }
            }
        }

    }

    return funamount;
}

