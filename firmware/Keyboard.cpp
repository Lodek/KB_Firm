#include "Keyboard.h"

//data structure with an array of key objects
//it contains all information regarding the keyboard
key matrix[NUM_KEYS]; 

//int that stores which layer is being used
int layervar = 0;

//buffer to output USB data
uint8_t out_buffer[8] = {0};

//physical pin number for each row and collunm
int row_pins[NUM_ROW] = {1, 21, 2, 3};
int col_pins[NUM_COLL] = {4, 5, 6, 7, 8, 9, 10, 16, 20, 15, 19, 14, 18};

//array that makes the matching of index number of layer with the value utilized by layer var
int layers[NUMLAYERS] = {LAYERVALUE};

//user defined mappings for each key
long mapping[NUMLAYERS][NUM_KEYS] =
{
	{0x002b, 0x0014, 0x001a, 0x0008, 0x0015, 0x0017, 0x001c, 0x0018, 0x000c, 0x0012, 0x0013, 0x002a, 0x002a, 0x0029, 0x0004, 0x0016, 0x0007, 0x0009, 0x000a, 0x000b, 0x000d, 0x000e, 0x000f, 0x0033, 0x0034, 0x0028, 0x0200, 0x001d, 0x001b, 0x0006, 0x0019, 0x0005, 0x0011, 0x0010, 0x0036, 0x0037, 0x0038, 0x2000, 0x0028, 0x0100, 0x0800, 0x0400, 0x0000, 0x010000, 0x002c, 0x002c, 0x020000, 0x0000, 0x0000, 0x0000, 0x1000, 0x004c},
	{0x0037, 0x0024, 0x0025, 0x0026, 0x2024, 0x2025, 0x002d, 0x0031, 0x0038, 0x2026, 0x2027, 0x002a, 0x002a, 0x0027, 0x0021, 0x0022, 0x0023, 0x2021, 0x2022, 0x2023, 0x2036, 0x2037, 0x202f, 0x2030, 0x2031, 0x0028, 0x0200, 0x001e, 0x001f, 0x0020, 0x201e, 0x201f, 0x2020, 0x002e, 0x0035, 0x002f, 0x0030, 0x2000, 0x0028, 0x0100, 0x0800, 0x0400, 0x0000, 0x010000, 0x002a, 0x002a, 0x020000, 0x0000, 0x0000, 0x0000, 0x1000, 0x002c},
	{0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x002a, 0x002a, 0x0039, 0x007f, 0x0081, 0x0080, 0x0074, 0x0078, 0x0050, 0x0051, 0x0052, 0x004f, 0x0048, 0x0045, 0x0028, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x004a, 0x004e, 0x004b, 0x004d, 0x0046, 0x2000, 0x0028, 0x0100, 0x0800, 0x0400, 0x0000, 0x010000, 0x0028, 0x0028, 0x020000, 0x0000, 0x0000, 0x0000, 0x1000, 0x002c},
};

// Initialize functions
void init_mappings()
{
	for (int i = 0; i < NUM_KEYS; i++)
    {
		 matrix[i].important = 0;
         matrix[i].on_buffer = 0;
         matrix[i].state = 0;
         for (int j = 0; j < NUMLAYERS; j++)
         {
            matrix[i].data[j].behavior = normal;
			matrix[i].data[j].keycode = mapping[j][i];
         }
    }
    return;
}


void _begin()
{
	/* begins commns serial communication
	no longer needed for pro micro, still used for uno and mega */
    Serial.end();
    Serial.begin(9600);
}


void startup_routine()
{
	/* initialize microcontroller
	sets rows as outputs and colls as inputs
	begins serial communication */
    for (int i = 0; i < NUM_ROW; i++) //sets row pins as OUT
    {
        pinMode(row_pins[i], OUTPUT);
        digitalWrite(row_pins[i], LOW);
    }
    for (int i = 0; i < NUM_COLL; i++) //sets colls pins as IN
        pinMode(col_pins[i], INPUT);
    _begin();
    init_mappings();
    return;
}



// Loop functions


void matrix_scan()
{
	/* uses col_pins & row_pins
	loops through each row; sets it to high, calls digital read for each coll_pin member and assigns it to the matching key object
	writes result of digital read to the matching key on the matrix array
	resets current row to low
	repeats for next row */
    for (int i = 0; i < NUM_ROW; i++)
    {
        digitalWrite(row_pins[i], HIGH);
		for (int j = 0; j < NUM_COLL; j++)  matrix[i*NUM_COLL+j].state = digitalRead(col_pins[j]);
		digitalWrite(row_pins[i], LOW);
    }
    return;
}


void matrix_iterator()
{
	//calls the key handler for each element of the matrix array
    for (int i = 0; i < NUM_COLL * NUM_ROW; i++)
    {
		key_handler(&matrix[i]);
    }
    return;
}


void key_handler(key *current_key)
{
	/* key handler receives a key struct object
	it first checks whether the key needs to be handled
	then it checks the key behavior and sends calls the appropriate handler function for that key type */
    (current_key->state == 0 && current_key->important == 0) return; // key is neither pressed nor important

    if (current_key->data[layervar_translator(layervar)].behavior == normal) behavior_normal(current_key); // normal key
    return ;
}


void write_buffer()
{
	//Sends the USB report to the host, the data element of the report is out_buffer which has length 8
    HID_SendReport(2, out_buffer, 8);
    return;
}


//Behavior Functions

void behavior_normal (key *current_key)
{
    long keycode;
  
    if (current_key->state == 1 && current_key->important == 1) return; //key was and still is pressed, nothig to be done

    if (current_key->state == 0 && current_key->important == 1) //is called when key has been released but its info is still on buffer and/or layervar
    {
        keycode = current_key->on_buffer;
        remove_from_buffer(keycode);
        current_key->on_buffer = -1;
        current_key->important = 0;
    }
  
    else if (current_key->state == 1 && current_key->important == 0) //key pressed and not on buffer
    {
        keycode = current_key->data[layervar_translator(layervar)].keycode;
        current_key->on_buffer = add_to_buffer(keycode); //calls add to buffer and writes the return to on_buffer variable

        if (current_key->on_buffer != -1)
        {
		    //add to buffer returns -1 if buffer is full. if buffer is full important remains 0 and key is proccessed next cycle
	        //sets the 2nd byte of on buffer with HID Modifiers used and the 3rd byte with layer value. Info needed for effective removal from buffer
            current_key->on_buffer = ((keycode & 0xFFFF00) | current_key->on_buffer);
            current_key->important = 1; //important=1 is important....
        }
    }
    return;
}


//Utility functions

int add_to_buffer(long key)
{
	/* Receives a key and does 3 operations. Returns the index of out_buffer where the HID Keycode was written to. If buffer is full, returns -1. 
	a - isolates and adds HID Keycode part of key to out_buffer (first byte of key, bitshifts and ANDs to isolate it).
	b - isolates and appends (ORs) HID modifier to out_buffer[0] (second byte of key).
	c - isolates and appends (ORs) layer byte of key to layervar (third byte of key) */
	int result = -1; 
	//In case key's 1st byte is 0, return 1. 1 is the reserved byte of the HID descriptor
	//Needed since otherwise 2 keys might point to the same index of out_buffer. Then releasing a key would release both and lead to an awkward situation
	if(key&0x0000FF==0) result=1;
	else
	{
	    for (int k = 2; k < 8; k++) //loops through out buffer looking for the first empty spot
	    {
	        if (out_buffer[k] == 0)
	        {
				out_buffer[k] = (key&0x0000FF); //a
				result = k;
				k = 9;
			}
	    }
	}
    out_buffer[0] = out_buffer[0] | ((key >> 8) & 0x0000FF); //b 
    layervar = (key >> 16) | layervar; //c 
    return result;
}


int remove_from_buffer(long key)
{
	/*removes the received key from the buffer and removes its value from layer var
	3 removal steps
	a - Removes key from buffer. Isolates first byte of key which contains the position of that key on out_buffer
	b - Removes modfier from buffer. Isolates second byte of key. That byte contais HID Mod code. Uses the data and XOR's it with the modifier byte of out_buffer
	c - Removes layer value from layervar. Isolates third byte of key which has layer value. Similarly, XOR's it with layervar to subtract its value */
    out_buffer[(key & 0x0000FF)] = 0; //a
	out_buffer[0] = out_buffer[0] ^ ((key >> 8) & 0x0000FF);//b
    layervar = (key >> 16)^layervar;//c
    return 0;
}


void flush()
{
	//loops through out_buffer zeroing all elements of the array
    for (int i = 0; i < 8; i++) out_buffer[i] = 0;
    return;
}


int layervar_translator(int layer)
{
	//function that returns the index for the corresponding VALUE of a layer (1,2,4,8...)
    for (int i = 0; i < NUMLAYERS ; i++) if (layers[i] == layer) return i;
}
