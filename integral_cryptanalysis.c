/* ========================================
	Integral cryptanalysis on 4-round AES:
	
	1) 	Choose 256 plaintext having equal values
	   	in 15 bytes and different values in one bye. 

	2)  Get the 256 ciphertexts; encrypted with a
		secret key using 4-rounds AES.
   ========================================*/

#include "sbox.h"
#include "aes.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function prototypes
void integral(unsigned char * ciphertext_set);
int roundKeyFound(unsigned char * candidates, int n);

// Expanded key (using https://www.cryptool.org/en/cto-highlights/aes)
unsigned char roundkeys[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, //r0 (cipherkey)
	0xd6, 0xaa, 0x74, 0xfd, 0xd2, 0xaf, 0x72, 0xfa, 0xda, 0xa6, 0x78, 0xf1, 0xd6, 0xab, 0x76, 0xfe, //r1
	0xb6, 0x92, 0xcf, 0x0b, 0x64, 0x3d, 0xbd, 0xf1, 0xbe, 0x9b, 0xc5, 0x00, 0x68, 0x30, 0xb3, 0xfe, //r2
	0xb6, 0xff, 0x74, 0x4e, 0xd2, 0xc2, 0xc9, 0xbf, 0x6c, 0x59, 0x0c, 0xbf, 0x04, 0x69, 0xbf, 0x41,	//r3
	0x47, 0xf7, 0xf7, 0xbc, 0x95, 0x35, 0x3e, 0x03, 0xf9, 0x6c, 0x32, 0xbc, 0xfd, 0x05, 0x8d, 0xfd, //r4
};

// Input
unsigned char set1_plaintext[4096] = {[0 ... 4095] = 0x00 };
unsigned char set1_ciphertext[4096] = { 0x00 };

unsigned char set2_plaintext[4096] = {[0 ... 4095] = 0x01 };
unsigned char set2_ciphertext[4096] = { 0 };

unsigned char set3_plaintext[4096] = {[0 ... 4095] = 0x02 };
unsigned char set3_ciphertext[4096] = { 0 };

unsigned char set4_plaintext[4096] = {[0 ... 4095] = 0x03 };
unsigned char set4_ciphertext[4096] = { 0 };

// Candidates array
unsigned char candidates[256] = {[0 ... 255] = 0x01 }; // Assume all are candidates and rule out as we integrate over multiple sets

int main(){
	
	int i;
	
	// while(!roundKeyFound(candidates, 16)){
	
	// Initialize chosen plaintexts with the first byte different
	for(i=0; i<256; i++){
		set1_plaintext[i*16] = i;
		set2_plaintext[i*16] = i;
	}
	// Encrypt plaintext sets and save to corresponding ciphertext sets
	for(i=0; i<256; i++){
		AES_enc(&set1_plaintext[i*16], roundkeys, &set1_ciphertext[i*16], S, 4);
		AES_enc(&set2_plaintext[i*16], roundkeys, &set2_ciphertext[i*16], S, 4);
	}

	// Run attack for chosen sets of ciphertexts
	integral(set1_ciphertext);
	integral(set2_ciphertext);
	// }
	
	// for(i=0; i<256; i++){
		// printf("candidate '0x%x' => %d\n", i, candidates[i]);
	// }
	
	return 0;
}


// Function definitons
void integral(unsigned char * ciphertext_set){
	
	int i, rk, ct;
	
	unsigned char tmp[4096];
	unsigned char roundkey_guess[16] = { 0x00 };
	
	unsigned char sum;
	unsigned char tmp_candidates[256] = { 0x0 };
		
	// Integrate for each round key (rk)
	for(rk=0; rk<256; rk++){
		
		// Copy ciphertext set to local scope for each rk value
		memcpy(tmp, ciphertext_set, 4096);
		
		// Initialize sum and round key guess for this loop-round
		sum = 0x00;	
		roundkey_guess[0] = (unsigned char) rk;
			
		// Compute with key guess on all ciphertexts values for one byte at a time
		for(ct=0; ct<256; ct++){
			
			// Compute backwards through last special round of AES
			addRoundKey(roundkey_guess, &tmp[ct*16]);				// addRoundKey is self-inverse		
			invShiftRows(&tmp[ct*16]);								// row i shift i bytes to the right
			subBytes(&tmp[ct*16], SI);								// substitute using inverse S-box
			
			sum ^=tmp[ct*16];										// xor-summatation of sub-results
		}
		
		// If guessed rk computed on all values of the ciphertext set sums to 0 then rk is a candidate
		if(sum == 0){
			tmp_candidates[rk] = 0x01;
		} else {
			tmp_candidates[rk] = 0x00;
		}
	}
	
	// Once done testing for each rk on the given ciphertext set, rule out candidates not in tmp_candidates
	for(i=0; i<256; i++){
		candidates[i] *= tmp_candidates[i];
	}
}

int roundKeyFound(unsigned char * candidates, int n){
	int i, sum;
	
	sum = 0;
	
	// candidates (cand) contain 0s and 1s; 1 => value is cand and 0 => cand is not
	for(i=0; i<256; i++){
		sum += candidates[i];
	}
	
	// If n candidates are found we are done
	if(sum == n)
		return 1;
	else
		return 0;
}










