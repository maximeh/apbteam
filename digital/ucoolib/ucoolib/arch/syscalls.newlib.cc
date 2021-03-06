/* ucoolib - Microcontroller object oriented library. {{{
 *
 * Copyright (C) 2012 Nicolas Schodet
 *
 * APBTeam:
 *        Web: http://apbteam.org/
 *      Email: team AT apbteam DOT org
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * }}} */
#include "ucoolib/common.hh"
#include "syscalls.newlib.hh"

#include <reent.h>
#include <sys/stat.h>
#include <errno.h>

ucoo::Stream *ucoo::syscalls_streams[3];

/** This is needed by C++ ABI, this simple definition will do.  See:
 * http://lists.debian.org/debian-gcc/2003/07/msg00057.html */
void *__dso_handle = (void*) &__dso_handle;

/** This function is called when a pure virtual function is called.  This is
 * needed by linker because when a abstract class constructor or destructor is
 * called, object is not complete.  Replace the one provided by the toolchain
 * to avoid including the world. */
extern "C" void
__cxa_pure_virtual (void)
{
    ucoo::halt ();
}

/** Increase program data space. */
extern "C" void *
_sbrk_r (struct _reent *ptr, int incr)
{
    extern char end; /* Defined in linker script. */
    static char *heap_end;
    char *prev_heap_end;
    if (heap_end == 0)
        heap_end = &end;
    prev_heap_end = heap_end;
    heap_end += incr;
    return (void *) prev_heap_end;
}

/** Exit program, endless loop to stop program, to be improved. */
extern "C" void
_exit (int n)
{
    ucoo::halt ();
}

/** Close a file, unimplemented. */
extern "C" int
_close_r (struct _reent *ptr, int fd)
{
    return -1;
}

/** Status of open file, consider all files as character devices. */
extern "C" int
_fstat_r (struct _reent *ptr, int fd, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/** Get PID, minimal implementation. */
extern "C" int
_getpid_r (struct _reent *ptr)
{
    return 1;
}

/** Whether file is a terminal, consider this is always true. */
extern "C" int
_isatty_r (struct _reent *ptr, int fd)
{
    return 1;
}

/** Send a signal, no process, no signal. */
extern "C" int
_kill_r (struct _reent *ptr, int pid, int sig)
{
    ptr->_errno = EINVAL;
    return -1;
}

/** Set position in a file, no-op. */
extern "C" off_t
_lseek_r (struct _reent *ptr, int fd, off_t pos, int whence)
{
    return 0;
}

/** Open a file, unimplemented. */
extern "C" int
_open_r (struct _reent *ptr, const char *file, int flags, int mode)
{
    return -1;
}

/** Read from file. */
extern "C" int
_read_r (struct _reent *ptr, int fd, void *buf, size_t cnt)
{
    if (fd == 0)
    {
        if (ucoo::syscalls_streams[0])
        {
            ucoo::Stream &s = *ucoo::syscalls_streams[0];
            int r = s.read (reinterpret_cast<char *> (buf), cnt);
            switch (r)
            {
            case -2:
                return 0;
            case -1:
                ptr->_errno = EIO;
                return -1;
            case 0:
                ptr->_errno = EAGAIN;
                return -1;
            default:
                return r;
            }
        }
        else
            return 0;
    }
    else
    {
        ptr->_errno = EBADF;
        return -1;
    }
}

/** Write to file. */
extern "C" int
_write_r (struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
    if ((fd == 1 || fd == 2) && ucoo::syscalls_streams[fd])
    {
        ucoo::Stream &s = *ucoo::syscalls_streams[fd];
        int r = s.write (reinterpret_cast<const char *> (buf), cnt);
        switch (r)
        {
        case -1:
            ptr->_errno = EIO;
            return -1;
        case 0:
            ptr->_errno = EAGAIN;
            return -1;
        default:
            return r;
        }
    }
    else
    {
        ptr->_errno = EBADF;
        return -1;
    }
}

