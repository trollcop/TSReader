/* FFdecsa -- fast decsa algorithm
 *
 * Copyright (C) 2003-2004  fatih89r
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <string.h>
#include <stdio.h>
// #include <sys/time.h>
#include "timeval.h"

#include "FFdecsa.h"


#ifndef NULL
#define NULL 0
#endif

#include "FFdecsa_test_testcases.h"

int SilentRun,ExitPause,RunOneByOne;
int PacketsForTest;


int compare(unsigned char *p1, unsigned char *p2, int n, int silently){
  int i;
  int ok=1;
  for(i=0;i<n;i++){
    if(i==3) continue; // tolerate this
    if(p1[i]!=p2[i]){
    //  if (!SilentRun) fprintf(stdout,"at pos 0x%02x, got 0x%02x instead of 0x%02x\n",i,p1[i],p2[i]);
      ok=0;
    }
  }
  
  if ((!silently) && (!SilentRun)){
    if(ok){
       fprintf(stdout,"CORRECT!\n");
    }
    else{
       fprintf(stdout,"FAILED!\n");
    }
  }
  return ok;
}



#define MAX_TS_PKTS_FOR_TEST 200*1000



unsigned char megabuf[188*MAX_TS_PKTS_FOR_TEST];
unsigned char onebuf[188];

unsigned char *cluster[10];

int main(int argc, char **argv){
  int i;
  struct timeval tvs,tve;
  char c;
  csa_keys_set TheKeys;

  
  SilentRun = 0;
  ExitPause = 1;
  RunOneByOne = 0;
  
  for (i=1;i<argc;i++){
   if ( (strcmp(argv[i], "-s")==0) || (strcmp(argv[i],"/s")==0) ) SilentRun = 1;
   if ( (strcmp(argv[i], "-1")==0) || (strcmp(argv[i],"/1")==0) ) RunOneByOne = 1;    	
   if ( (strcmp(argv[i], "-q")==0) || (strcmp(argv[i],"/q")==0) ) ExitPause = 0;
  }  	
    
  PacketsForTest = MAX_TS_PKTS_FOR_TEST;
  if (RunOneByOne) PacketsForTest = MAX_TS_PKTS_FOR_TEST/10;
  
  fprintf(stdout,"KeySet Size= %d\n",get_keyset_size());
  
  // fprintf(stdout,"FFdecsa 1.0: testing correctness and speed\n");
  // fprintf(stdout,"\nSilent=%d ,OneByOne=%d ,ExitPause=%d\n",SilentRun,RunOneByOne,ExitPause);
   
    
/* begin correctness testing */

  set_control_words(test_invalid_key,test_1_key,&TheKeys);
  memcpy(onebuf,test_1_encrypted,188);
  cluster[0]=onebuf;cluster[1]=onebuf+188;cluster[2]=NULL;
  decrypt_packets(cluster,&TheKeys);
  compare(onebuf,test_1_expected,188,0);

  set_control_words(test_2_key,test_invalid_key,&TheKeys);
  memcpy(onebuf,test_2_encrypted,188);
  cluster[0]=onebuf;cluster[1]=onebuf+188;cluster[2]=NULL;
  decrypt_packets(cluster,&TheKeys);
  compare(onebuf,test_2_expected,188,0);

  set_control_words(test_3_key,test_invalid_key,&TheKeys);
  memcpy(onebuf,test_3_encrypted,188);
  cluster[0]=onebuf;cluster[1]=onebuf+188;cluster[2]=NULL;
  decrypt_packets(cluster,&TheKeys);
  compare(onebuf,test_3_expected,188,0);

  set_control_words(test_p_10_0_key,test_invalid_key,&TheKeys);
  memcpy(onebuf,test_p_10_0_encrypted,188);
  cluster[0]=onebuf;cluster[1]=onebuf+188;cluster[2]=NULL;
  decrypt_packets(cluster,&TheKeys);
  compare(onebuf,test_p_10_0_expected,188,0);

  set_control_words(test_p_1_6_key,test_invalid_key,&TheKeys);
  memcpy(onebuf,test_p_1_6_encrypted,188);
  cluster[0]=onebuf;cluster[1]=onebuf+188;cluster[2]=NULL;
  decrypt_packets(cluster,&TheKeys);
  compare(onebuf,test_p_1_6_expected,188,0);

/* begin speed testing */


  
#if 0
// test on short packets
#define s_encrypted test_p_1_6_encrypted
#define s_key_e     test_p_1_6_key
#define s_key_o     test_invalid_key
#define s_expected  test_p_1_6_expected

#else
//test on full packets
#define s_encrypted test_2_encrypted
#define s_key_e     test_2_key
#define s_key_o     test_invalid_key
#define s_expected  test_2_expected

#endif

  for(i=0;i<PacketsForTest;i++){
    memcpy(&megabuf[188*i],s_encrypted,188);
  }
  
  
// test that packets are not shuffled around
// so, let's put an undecryptable packet somewhere in the middle (we will use a wrong key)
#define noONE_POISONED_PACKET
#ifdef ONE_POISONED_PACKET
  memcpy(&megabuf[188*(PacketsForTest*2/3)],test_3_encrypted,188);
#endif



  if (!SilentRun){
   if (RunOneByOne)
    fprintf(stdout,"Starting test ONE-BY-ONE , Packets =%d\n",PacketsForTest);
   else
    fprintf(stdout,"Starting test on CLUSTER , Packets =%d\n",PacketsForTest);
  };
  

  // start decryption
  set_control_words(s_key_e,s_key_o,&TheKeys);
  gettimeofday(&tvs,NULL);
  
 SetThreadPriority (GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL );
     

  if (RunOneByOne){
   // force one by one
   for(i=0;i<PacketsForTest;i++){
    cluster[0]=megabuf+188*i;
    cluster[1]=megabuf+188*i+188;
    cluster[2]=NULL;
    decrypt_packets(cluster,&TheKeys);
   }
  }  
  else
  {
   int done=0;
   while(done<PacketsForTest){
    cluster[0]=megabuf+188*done;
    cluster[1]=megabuf+188*PacketsForTest;
    cluster[2]=NULL;
    done+=decrypt_packets(cluster,&TheKeys);
   }
  };
  
 SetThreadPriority (GetCurrentThread(),THREAD_PRIORITY_NORMAL );

  gettimeofday(&tve,NULL);
  //end decryption

  fprintf(stdout,"Speed=%.2f Mbit/s , %.2f pkts/s \n",
         (184*PacketsForTest*8)/((tve.tv_sec-tvs.tv_sec)+1e-6*(tve.tv_usec-tvs.tv_usec))/1000000,
         PacketsForTest/((tve.tv_sec-tvs.tv_sec)+1e-6*(tve.tv_usec-tvs.tv_usec))
         );

  // this packet couldn't be decrypted correctly
#ifdef ONE_POISONED_PACKET
  compare(megabuf+188*(PacketsForTest*2/3),test_3_expected,188,0); /* will fail because we used a wrong key */
#endif
  // these should be ok
  compare(megabuf,s_expected,188,0);
  compare(megabuf+188*511,s_expected,188,0);
  compare(megabuf+188*512,s_expected,188,0);
  compare(megabuf+188*319,s_expected,188,0);
  compare(megabuf+188*(PacketsForTest-1),s_expected,188,0);

  for(i=0;i<PacketsForTest;i++){
    if(!compare(megabuf+188*i,s_expected,188,1)){
      fprintf(stdout,"FAILED COMPARISON OF PACKET %10i\n",i);
    };
  }
 
 if (ExitPause) scanf ("%c",&c); 
 return 0;
}
