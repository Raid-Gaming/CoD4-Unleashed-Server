//This code has no use at all.

#include <stdio.h>

typedef enum{qfalse, qtrue}qboolean;

qboolean CL_CDKeyValidate(char* key1234, char* key5){

	int testkey = 0;
	int j;
	int i;
	char key5t[5];

	for(j=0; j < 16; j++)
	{
		testkey ^= (signed char)key1234[j];
		for(i = 8; i > 0; i--){

			if(testkey & 1){
				testkey >>= 1;
				testkey ^= 0xa001;
			}else{
				testkey >>= 1;
			}
		}

	}

	sprintf(key5t, "%04x", testkey);
	if(!key5)
		return 1;

	printf("Testkey: %s\n", key5t);

	if(Q_stricmpn(key5t, key5, 4))
		return 1;

	return 0;
}

int Q_stricmpn (const char *s1, const char *s2, int n) {
	int		c1, c2;

        if ( s1 == NULL ) {
           if ( s2 == NULL )
             return 0;
           else
             return -1;
        }
        else if ( s2==NULL )
          return 1;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);
	
	return 0;		// strings are equal
}



int main(void){
	if(CL_CDKeyValidate("ICENINJAMENRULES","08FF")){
		printf("Key is bad\n");
	}else{
		printf("Key is good\n");
	}
	return 0;
}