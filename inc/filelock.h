/*
 * Copyright 2007 (c) SAEIL Systems Inc. All Rights Reserved.
 *
 * MODULE NAME : filelock.h
 *
 * PRODUCT(S)  : Panel Monitoring Unit Development
 *
 * DESCRIPTION : pdb(performance-db) info definition header file
 *
 * CAUTION     :
 *
 * GLOBAL FUNCTIONS DEFINED IN THIS MODULE :
 *
 * REVISION HISTORY
 *   Date      Who    Rev                       Comments
 * ---------- ------  ----  -------------------------------------------
 * 2012.07.17 ChoiBC   01   copy from dcufep
 */
#if !defined(__filelock_H)
#define __filelock_H

/*
 * include files
 */
#include <fcntl.h>
//#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	file_lock_read( fd, offset, len ) \
			lock_reg( fd, F_SETLKW, F_RDLCK, offset, len )
#define	file_lock_write( fd, offset, len ) \
			lock_reg( fd, F_SETLKW, F_WRLCK, offset, len )
#define	file_unlock( fd, offset, len ) \
			lock_reg( fd, F_SETLK, F_UNLCK, offset, len )

/*
 *  Functions proto-type definition
 *      filelock.c
 */
int lock_reg( int fd, int cmd, int type, off_t offset, off_t len );

#ifdef __cplusplus
}
#endif

#endif  /* end of if !defined(__filelock_H) */
