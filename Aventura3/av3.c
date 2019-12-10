/*
*
* Authors: Aguilar Ferrer, Felix Lluis
*          Bennasar Polzin, Adrian
*          Bueno Lopez, Alvaro
*
* Date:    03/01/2020
*/

// Constants: 
#define THREADS 10
#define NODES 10

// Libraries:
#include <stdio.h>
#include "my_lib.h" 


/*
* Structure for the data: 
* -----------------------
*  val: an integer.
*/
struct my_data 
{
    int val;
};


int main(int argc, char **argv)
{
    if(!argv[1] || argv[2])
    {
        fprintf(stderr,"Error de sintaxis: ./av3 nombre archivo\n");
        return 1;
    }
    struct my_stack *stack;
    stack = my_stack_read(argv[1]);

    if(stack)
    {
        if(my_stack_len(stack) != NODES)
        {
            if(my_stack_len(stack) < NODES)
            {
                int error = 0;
                while(my_stack_len(stack) != NODES && !error)
                {
                    struct my_data *data;
                    data = malloc(sizeof(struct my_data));
                    if(data)
                    {
                        data->val = 0;
                        my_stack_push(stack, data);
                        free(data);
                    }
                    else 
                    {
                        //Error
                        error = 1;
                    }
                }
            }
            else
            {
                while(my_stack_len(stack) != NODES)
                {
                    my_stack_pop(stack);
                }
            }
        }
    }
    else
    {
        stack = my_stack_init(sizeof(struct my_data));
        int error = 0;

        while(my_stack_len(stack) != NODES && !error)
        {
            struct my_data *data;
            data = malloc(sizeof(struct my_data));
            if(data)
            {
                data->val = 0;
                my_stack_push(stack, data);
                free(data);
            }
            else 
            {
                //Error
                error = 1;
            }
        }
    }

    my_stack_write(stack,argv[1]);

    struct my_data *data;
    while ((data = my_stack_pop(stack))) {
        printf("Node of s1: (%d)\n", data->val);
    }
    
    my_stack_purge(stack);
}