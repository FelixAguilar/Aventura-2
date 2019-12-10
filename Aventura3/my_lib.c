/*
* Part 1
* ------
* Implementation of a set of functions analogous to the respective ones of
* <string.h>. they are strlen, strcmp, strcpy, strncpy and strcat.
*
* Part 2
* ------
* Implementation of a set of functions to create and modify using pop and push 
* one stack of elements, this stack can be saved or loaded from a file and it 
* is posible to erase it from memory.
*
* Authors: Aguilar Ferrer, Felix Lluis
*          Bennasar Polzin, Adrian
*          Bueno Lopez, Alvaro
*
* Date:    01/10/2019.
*/

#include <stdio.h>
#include "my_lib.h"

/*
* Part 1
* ------
*/

/*
* Function:  my_strlen
* --------------------
* Returns the length of the string of characters that finishes with "/0".
*
*  str: pointer of the string of characters to look at.
*
*  returns: the length of the string pointed by str.
*/
size_t my_strlen(const char * str){
    
    // It will contain the length of the str.
    size_t length;
    
    // Counts the number of characters until it finds "/0" then returns it.
    for(length = 0; *(str + length) != '\0'; length++){}
    return length;
}

/*
* Function:  my_strcmp
* --------------------
* Compare character by character if they are the same between the strings. If 
* they are not equal it returns the subtraction of str1 with str2. If they are 
* equal returns 0.
*
*  str1: pointer of the character string 1.
*  str2: pointer of the character string 2.
*
*  returns: 0 if they are equal, negative if str1 is lower than str2 and 
*           positive if str1 is higher than str2.
*/
int my_strcmp(const char *str1, const char *str2){

    // Obtain the size of both strings.
    size_t sizeStr1 = my_strlen(str1);
    size_t sizeStr2 = my_strlen(str2);

    // Compare character by character.
    for(int index = 0; index < (sizeStr1 && sizeStr2) ; index++){
        
        // If they are not equal returns the subtraction.
        if((*(str1 + index)) != (*(str2 + index))){
            return *(str1 + index) - *(str2 + index);
        }
    }
    
    // If they are equal returns 0.
    return 0;
}

/*
* Function:  my_strcpy
* --------------------
* Copies the string pointed by src into the memory pointed by dest.
*
*  dest: pointer of the memory where will be stored the string src.
*  str: pointer of the string that will be copied into dest.
*
*  returns: pointer of the memory position dest.
*/
char *my_strcpy(char *dest, const char *src){

    // Gets the length of the string src.
    size_t sizeSrc = my_strlen(src);

    // Copies src into dest character by character.
    for(int index=0; index < sizeSrc; index++){
        *(dest+index)= *(src+index);
    }

    // Adds the end of string character, '\0', into the end of dest.
    *(dest+sizeSrc) = '\0';
    return dest;
}

/*
* Function:  my_strncpy
* ---------------------
* Copies n characters from the char string src into the vector dest.
*
*  dest: pointer of the vector where it will be copied the src.
*  str: pointer of the character string which will be copied in dest.
*  n: number of characters that will be copied into dest.
*
*  returns: pointer of the character string dest.
*/
char *my_strncpy(char *dest, const char *src, size_t n){

    // Get the lenght of src.
    size_t sizeSrc = my_strlen(src);

    // Copies src into dest until reaching n or ending the string.
    for(int index=0; (index < n)&&(index < sizeSrc); index++){
        *(dest+index)= *(src+index);
    }

    // Add the final character '\0' into the end of the char string dest.
    *(dest+n+1) = '\0';
    return dest;
}

/*
* Function:  my_strcat
* --------------------
* Add the character string src into the character string dest at the end of it.
* This overwrites the character "/0" at the end of dest and moves it after
* adding src being the new end of the character string.
*
*  dest: pointer of the character string where it will be added the src.
*  str: pointer of the character string which will be copied in dest.
*
*  returns: the pointer of the character string dest.
*/
char *my_strcat(char *dest, const char *src){
    
    // Obtain the size of both character strings.
    size_t sizeDest = my_strlen(dest);
    size_t sizeSrc = my_strlen(src);
    
    // Copies the src into the end of dest.
    for (int index = 0; index < sizeSrc; index++) {
        *(dest + sizeDest + index) = *(src + index);
    }
    
    // Adds the ending character into the character string dest.
    *(dest + sizeDest + sizeSrc) = '\0';
    return dest;
}

/*
* Part 2
* ------
*/

/*
* Function:  my_stack_init
* ------------------------
* Reserve space for the pointer for the top node of the stack and the size of
* the data that will hold, and initialize those values with NULL as the pointer
* value to the top node of the stack and with the size of data passed to it as
* parameter.
*
*  size: size of the data that will be inside each element in the stack.
*
*  returns: the pointer of the stack created.
*/
struct my_stack *my_stack_init (int size){
    
    // Initialize stack and allocates memory to it.
    struct my_stack *stack;
    stack = (struct my_stack *) malloc (sizeof(struct my_stack));
    
    // If there is no memory available, then error.
    if (stack){

        // Adds the values for the header of the stack.
        stack->size = size;
        stack->first = NULL;
    }
    else{
        printf("Error, not enough memory free to use.");
    }
    return stack;
}

/*
* Function:  my_stack_push
* ------------------------
* Insert a new node into the stack at the begining of it so it will be the
* first node now.
*
*  stack: The pointer to the stack.
*  data: The pointer to the data for the node.
*
*  returns: 0 or -1 depending of the result.
*/
int my_stack_push (struct my_stack *stack, void *data){
    
    // It will check for the stack and size of data (init).
    int result = -1;
    if (stack) {
        if (stack->size){
            
            // Allocates memory for the node.
            struct my_stack_node *node;
            node = (struct my_stack_node *) malloc(sizeof(struct my_stack_node));
            
            // If there is no memory available, then error.
            if (node) {

                // Adds the node at the begining by moving the pointers.
                node->data = data;
                node->next = stack->first;
                stack->first = node;
                
                // If it is correct it will return 0.
                result = 0;
            }
            else{
                printf("Error, not enough memory free to use.");   
            }
        }
    }
    return result;
}

/*
* Function:  my_stack_pop (struct my_stack *stack)
* ------------------------------------------------
* Deletes the node pointed by the pointer first from the stack.
*
*  stack: The pointer to the stack.
*
*  returns: the pointer to the data of the deleted node.
*/
void *my_stack_pop (struct my_stack *stack){
    
    // If the stack is empty, there is no node to delete.
    if(stack->first){
        struct my_stack_node *node;
        
        // It creates a pointer to the node that will be deleted.
        node = stack->first;
        stack->first = node->next;
        
        // Obteins the pointer to the data of the deleted node.
        void *data = node->data;
        
        // Free allocated memory of the deleted node.
        free(node);
        return data;
    }
    else{
        return NULL;
    }
}

/*
* Function:  my_stack_len
* -----------------------
* Returns the number of nodes inside the stack (length of the stack).
*
*  stack: The pointer to the stack.
*
*  returns: the number of nodes inside the stack.
*/
int my_stack_len (struct my_stack *stack){
    
    // Inicialize the counter at 0 and checks if there is at least one node.
    int size = 0;
    if (stack->first){
        
        // Creates a node pointing to the first node.
        struct my_stack_node *pointer;
        pointer = stack->first;
        
        // Goes over the stack adding 1 to size until it reaches the NULL.
        while (pointer){
            size++;
            pointer = pointer->next;
        }
    }
    return size;
}

/*
* Function:  aux_my_stack_write
* -----------------------------
* It is a recursive function, it will check if it is the first node introduced
* into. when it finds that, it will starts writing from the last node into the
* top node when it writes saves the number of bytes writen into the file.
*
*  node: The pointer to the node to look at.
*  filename: The pointer to the file name.
*  size: The data size to write into the file.
*
*  returns: -1 if an error ocurred or the bytes written inside the file.
*/
int aux_my_stack_write (struct my_stack_node *node, int fd , int size){
    
    // Inicializes wrtten to 0 and looks if is not the last node.
    int bytes = 0;
    if (node->next){
        
        // Calls again this function with the next node as input.
        bytes = aux_my_stack_write(node->next, fd, size);
    }
    
    // Writes the node into the file and returns the bytes written until now.
    return write(fd, node->data, size) + bytes;
}

/*
* Function:  my_stack_write
* -------------------------
* It saves the stack values and their size into a file so it will be able to be
* recovered later. There is an auxiliary function that will be the one to look
* into the stack until it reaches the end of it, and then it starts writing
* the data from the nodes inside the file from the first introduced until the
* top node.
*
*  stack: The pointer to the stack.
*  filename: The pointer to the file name.
*
*  returns: -1 if an error ocurred or the number of nodes if it was succesful.
*/
int my_stack_write (struct my_stack *stack, char *filename){
    
    // Creates the file with the name introduced as parameter.
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    // Writes the data size inside the file.
    write(fd, &stack->size, sizeof(int));
    
    // Starts the auxiliary function to write the nodes.
    int bytes = aux_my_stack_write(stack->first, fd, stack->size);
    
    // Closes the file and returns the number of nodes written inside the file.
    close(fd);
    if (bytes == -1) {
        return bytes;
    }
    else{
        return bytes/stack->size;
    }
}

/*
* Function:  my_stack_read
* ------------------------
* It restores inside memory the stack and the data from the nodes stored 
* inside a file.
*
*  filename: The pointer to the file name.
*
*  returns: NULL if an error ocurred or the pointer of the stack if it was 
*           succesful.
*/
struct my_stack *my_stack_read (char *filename){

    // Creates a file id so it can be accessed.
    int fd;
    if ((fd = open(filename, O_RDONLY)) < 0){
        return NULL;
    }

    // Gets the size of the data from the file.
    int size;
    read(fd, &size, sizeof(int));

    // Inicializes the stack using init.
    struct my_stack *stack = my_stack_init(size);

    // Loop to restore the nodes from the file, end is the boolean to exit it.
    int end = 1;
    while(end){

        // Reserves memory space for the data so it can be restored the node.
        void *data;
        data = malloc(stack->size);

        // If it was posible to read data from the file it will create a node.
        if (read(fd, data, stack->size)) {
            my_stack_push(stack, data);
        }
        else{

            // If not it will exit from the loop.
            end = 0;
        }
    }

    // It closes the file.
    close(fd);
    return stack;
}


/*
* Function:  my_stack_purge
* -------------------------
* It frees all memory that has been allocated with malloc() at any moment.
*
*  stack: The pointer to the stack.
*
*  returns: number of bytes that has been liberated.
*/
int my_stack_purge (struct my_stack *stack){
    
    // In it will be stored the number of bytes liberated.
    int bytes = 0;

    // Loop until the last node has been purged.
    while (stack->first) {

        // Moves the pointers so it is save to delete the node.
        struct my_stack_node *node = stack->first;
        stack->first = node->next;

        // Add the size in bytes of data stored in the node and frees it.
        bytes = bytes + stack->size;
        free(node->data);

        // Add the size in bytes of the node and frees it.
        bytes = bytes + sizeof(struct my_stack_node);
        free(node);
    }
    
    // Frees the header for the stack and returns the number of bytes now free.
    bytes = bytes + sizeof(struct my_stack);
    free(stack);
    return bytes;
}