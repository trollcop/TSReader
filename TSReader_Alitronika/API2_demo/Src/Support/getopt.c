#include <stdio.h>
#include <string.h>
#include "getopt.h"

int	opterr;
int	optind;
int	optopt;
char *optarg;
int	optreset;

static void getopterror(int which)
{
  static char error1[]="Unknown option `-x'.\n";
  static char error2[]="Missing argument for `-x'.\n";
  if (opterr) {
    if (which) {
      error2[23]=optopt;
      fprintf(stderr,error2,28);
    } else {
      error1[17]=optopt;
      fprintf(stderr,error1,22);
    }
  }
}

int getopt_long(int argc, char * const argv[], const char *optstring,
		const struct CmdLineOption *longopts, int *longindex)
{
  static int lastidx,lastofs;
  char *tmp;
  if (optind==0) optind=1;	/* whoever started setting optind to 0 should be shot */
again:
  if (optind>argc || !argv[optind] || *argv[optind]!='-' || argv[optind][1]==0)
    return -1;
  if (argv[optind][1]=='-' && argv[optind][2]==0) {
    ++optind;
    return -1;
  }
  if (argv[optind][1]=='-') {	/* long option */
    char* arg=argv[optind]+2;
    char* max=strchr(arg,'=');
    const struct CmdLineOption* o;
    if (!max) max=arg+strlen(arg);
    for (o=longopts; o->name; ++o) {
      if (!strncmp(o->name,arg,(size_t)(max-arg))) {	/* match */
	if (longindex) *longindex=o-longopts;
	if (o->has_arg>0) {
	  if (*max=='=')
	    optarg=max+1;
	  else {
	    optarg=argv[optind+1];
	    if (!optarg && o->has_arg==1) {	/* no argument there */
	      if (*optstring==':') return ':';
	      fprintf(stderr,"argument required: `",20);
	      fprintf(stderr,arg,(size_t)(max-arg));
	      fprintf(stderr,"'.\n",3);
	      ++optind;
	      return '?';
	    }
	    ++optind;
	  }
	}
	++optind;
	if (o->flag)
	  *(o->flag)=o->val;
	else
	  return o->val;
	return 0;
      }
    }
    if (*optstring==':') return ':';
    fprintf(stderr,"invalid option `",16);
    fprintf(stderr,arg,(size_t)(max-arg));
    fprintf(stderr,"'.\n",3);
    ++optind;
    return '?';
  }
  if (lastidx!=optind) {
    lastidx=optind; lastofs=0;
  }
  optopt=argv[optind][lastofs+1];
  if ((tmp=strchr(optstring,optopt))) {
    if (*tmp==0) {	/* apparently, we looked for \0, i.e. end of argument */
      ++optind;
      goto again;
    }
    if (tmp[1]==':') {	/* argument expected */
      if (tmp[2]==':' || argv[optind][lastofs+2]) {	/* "-foo", return "oo" as optarg */
	if (!*(optarg=argv[optind]+lastofs+2)) optarg=0;
	goto found;
      }
      optarg=argv[optind+1];
      if (!optarg) {	/* missing argument */
	++optind;
	if (*optstring==':') return ':';
	getopterror(1);
	return ':';
      }
      ++optind;
    } else {
      ++lastofs;
      return optopt;
    }
found:
    ++optind;
    return optopt;
  } else {	/* not found */
    getopterror(0);
    ++optind;
    return '?';
  }
}
