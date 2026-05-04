/*
 * Portions Copyright (c) 1987, 1993, 1994
 * The Regents of the University of California.  All rights reserved.
 *
 * Portions Copyright (c) 2003, PostgreSQL Global Development Group
 *
 * $Header: /espirit/Alitronika/ATDV_API2/ATDemoApp/Src/Support/getopt.h,v 1.2 2007/05/25 15:32:34 Es10 Exp $
 */
#ifndef GETOPT_LONG_H
#define GETOPT_LONG_H

#ifdef __cplusplus
extern "C" {
#endif

/* These are picked up from the system's getopt() facility. */
extern int	opterr;
extern int	optind;
extern int	optopt;
extern char *optarg;

/* Some systems have this, otherwise you need to define it somewhere. */
extern int	optreset;

struct CmdLineOption
{
	const char *name;
	int			has_arg;
	int		   *flag;
	int			val;
};

#define no_argument 0
#define required_argument 1

extern int getopt_long(int argc, char * const argv[], const char *optstring,
				const struct CmdLineOption *longopts, int *longindex);

#ifdef __cplusplus
}
#endif


#endif   /* GETOPT_LONG_H */
