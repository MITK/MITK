/*************************************************************************
 *
 * $Id$
 *
 * Copyright (c) 1998 by Bjorn Reese <breese@mail1.stofanet.dk>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 ************************************************************************
 *
 * 1999/08/19 - breese
 *  - Cleaned up the interface
 *
 * 1999/08/06 - breese
 *  - Added U_STACK_TRACE for HP/UX
 *
 * 1999/01/24 - breese
 *  - Added GCC_DumpStack
 *
 * 1998/12/21 - breese
 *  - Fixed include files and arguments for waitpid()
 *  - Made an AIX workaround for waitpid() exiting with EINTR
 *  - Added a missing 'quit' command for OSF dbx
 *
 ************************************************************************/

#if defined(unix) || defined(__unix) || defined(__xlC__)
# define PLATFORM_UNIX
#elif defined(WIN32) || defined(_WIN32)
# define PLATFORM_WIN32
#endif

#if defined(_AIX) || defined(__xlC__)
# define PLATFORM_AIX
#elif defined(__FreeBSD__)
# define PLATFORM_FREEBSD
#elif defined(hpux) || defined(__hpux) || defined(_HPUX_SOURCE)
# define PLATFORM_HPUX
#elif defined(sgi) || defined(mips) || defined(_SGI_SOURCE)
# define PLATFORM_IRIX
#elif defined(__osf__)
# define PLATFORM_OSF
#elif defined(M_I386) || defined(_SCO_DS) || defined(_SCO_C_DIALECT)
# define PLATFORM_SCO
#elif defined(sun) || defined(__sun__) || defined(__SUNPRO_C)
# if defined(__SVR4) || defined(__svr4__)
#  define PLATFORM_SOLARIS
# endif
#endif

/* ANSI C includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#if defined(PLATFORM_UNIX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# if defined(PLATFORM_IRIX) && defined(USE_BUILTIN)
/* Compile with -DUSE_BUILTIN and -lexc */
#  include <libexc.h>
# elif defined(PLATFORM_HPUX) && defined(USE_BUILTIN)
/* Compile with -DUSE_BUILTIN and -lcl */
extern void U_STACK_TRACE(void);
# endif
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
# define STDOUT_FILENO 1
# define STDERR_FILENO 2
#endif // STDIN_FILENO

#include "berryStackTrace.h"

#ifndef FALSE
# define FALSE (0 == 1)
# define TRUE (! FALSE)
#endif

#define SYS_ERROR -1

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#define MAX_BUFFER_SIZE 512
#if defined(__GNUC__)
/* Change the code if ADDRESSLIST_SIZE is increased */
# define ADDRESSLIST_SIZE 20
#endif

/*************************************************************************
 * Globals
 *
 * We cannot pass custom arguments to signal handlers so we store
 * them as global variables (but limit their scope to this file.)
 */
static const char *global_progname;
static int global_output = STDOUT_FILENO;


#if defined(PLATFORM_UNIX)
/*************************************************************************
 * my_pclose [private]
 */
static void my_pclose(int fd, int pid)
{
  close(fd);
  /* Make sure the the child process has terminated */
  (void)kill(pid, SIGTERM);
}

/*************************************************************************
 * my_popen [private]
 */
static int my_popen(const char *command, pid_t *pid)
{
  int rc;
  int pipefd[2];

  rc = pipe(pipefd);
  if (SYS_ERROR != rc)
    {
      *pid = fork();
      switch (*pid)
  {
  case SYS_ERROR:
    rc = SYS_ERROR;
    close(pipefd[0]);
    close(pipefd[1]);
    break;

  case 0: /* Child */
    close(pipefd[0]);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    /*
     * The System() call assumes that /bin/sh is
     * always available, and so will we.
     */
    execl("/bin/sh", "/bin/sh", "-c", command, NULL);
    _exit(EXIT_FAILURE);
    break;

  default: /* Parent */
    close(pipefd[1]);
    rc = pipefd[0];
    break;
  } /* switch */
    }
  return rc;
}

/*************************************************************************
 * my_getline [private]
 */
static int my_getline(int fd, char *buffer, int max)
{
  char c;
  int i = 0;

  do {
    if (read(fd, &c, 1) < 1)
      return 0;
    if (i < max)
      buffer[i++] = c;
  } while (c != '\n');
  buffer[i] = (char)0;
  return i;
}

/*************************************************************************
 * GCC_DumpStack [private]
 *
 *
 * This code is still experimental.
 *
 * Stackbased arrays are used to prevent allocating from the heap.
 * Warning: sprintf/sscanf are not ASync safe. They were used for
 *  convenience.
 *
 * 'nm' is used because it is most widespread
 *  GNU:     nm [-B]
 *  Solaris: nm -x -p
 *  IRIX:    nm -x -B  (but __builtin_return_address() always returns NULL)
 *  AIX:     nm -x -B
 *  OSF/1:   nm -B
 *  SCO/OpenServer: nm -x -p
 *  HP/UX    nm -x -p
 */

#if defined(__GNUC__) && defined(USE_BUILTIN)

typedef struct {
  unsigned long realAddress;
  unsigned long closestAddress;
  char name[MAX_BUFFER_SIZE + 1];
  char type;
} address_T;


static void GCC_DumpStack(void)
{
  int i;
  void *p = &p; /* dummy start value */
  address_T syms[ADDRESSLIST_SIZE + 1];
  char buffer[MAX_BUFFER_SIZE];
  int fd;
  pid_t pid;
  unsigned long addr;
  unsigned long highestAddress;
  unsigned long lowestAddress;
  char type;
  char *pname;
  char name[MAX_BUFFER_SIZE];
  int number;

  for (i = 0; p; i++)
    {
      /*
       * This is based on code by Steve Coleman <steve.colemanjhuapl.edu>
       *
       * __builtin_return_address() only accepts a constant as argument.
       */
      switch (i)
  {
  case 0:
          if (__builtin_frame_address(0))
    p = __builtin_return_address(0);
    else p = NULL;
    break;
  case 1:
    if (__builtin_frame_address(1))
    p = __builtin_return_address(1);
    else p = NULL;
    break;
  case 2:
    if (__builtin_frame_address(2))
    p = __builtin_return_address(2);
    else p = NULL;
    break;
  case 3:
    if (__builtin_frame_address(3))
    p = __builtin_return_address(3);
    else p = NULL;
    break;
  case 4:
    if (__builtin_frame_address(4))
    p = __builtin_return_address(4);
    else p = NULL;
    break;
  case 5:
    if (__builtin_frame_address(5))
    p = __builtin_return_address(5);
    else p = NULL;
    break;
  case 6:
    if (__builtin_frame_address(6))
    p = __builtin_return_address(6);
    else p = NULL;
    break;
  case 7:
    if (__builtin_frame_address(7))
    p = __builtin_return_address(7);
    else p = NULL;
    break;
  case 8:
    if (__builtin_frame_address(8))
    p = __builtin_return_address(8);
    else p = NULL;
    break;
  case 9:
    if (__builtin_frame_address(9))
    p = __builtin_return_address(9);
    else p = NULL;
    break;
  case 10:
    if (__builtin_frame_address(10))
    p = __builtin_return_address(10);
    else p = NULL;
    break;
  case 11:
    if (__builtin_frame_address(11))
    p = __builtin_return_address(11);
    else p = NULL;
    break;
  case 12:
    if (__builtin_frame_address(12))
    p = __builtin_return_address(12);
    else p = NULL;
    break;
  case 13:
    if (__builtin_frame_address(13))
    p = __builtin_return_address(13);
    else p = NULL;
    break;
  case 14:
    if (__builtin_frame_address(14))
    p = __builtin_return_address(14);
    else p = NULL;
    break;
  case 15:
    if (__builtin_frame_address(15))
    p = __builtin_return_address(15);
    else p = NULL;
    break;
  case 16:
    if (__builtin_frame_address(16))
    p = __builtin_return_address(16);
    else p = NULL;
    break;
  case 17:
    if (__builtin_frame_address(17))
    p = __builtin_return_address(17);
    else p = NULL;
    break;
  case 18:
    if (__builtin_frame_address(18))
    p = __builtin_return_address(18);
    else p = NULL;
    break;
  case 19:
    if (__builtin_frame_address(19))
    p = __builtin_return_address(19);
    else p = NULL;
    break;
  default:
    /* Change ADDRESSLIST_SIZE if more are added */
    p = NULL;
    break;
  }
      if ((p) && (i < ADDRESSLIST_SIZE))
  {
    syms[i].realAddress = (unsigned long)p;
    syms[i].closestAddress = 0;
    syms[i].name[0] = (char)0;
    syms[i].type = ' ';
  }
      else
  {
    syms[i].realAddress = 0;
    break; /* for */
  }
    } /* for */


  /* First find out if we are using GNU or vendor nm */
  number = 0;
  strcpy(buffer, "nm -V 2>/dev/null | grep GNU | wc -l");
  fd = my_popen(buffer, &pid);
  if (SYS_ERROR != fd)
    {
      if (my_getline(fd, buffer, sizeof(buffer)))
  {
    sscanf(buffer, "%d", &number);
  }
      my_pclose(fd, pid);
    }
  if (number == 0) /* vendor nm */
    {
# if defined(PLATFORM_SOLARIS) || defined(PLATFORM_SCO) || defined(PLATFORM_HPUX)
      strcpy(buffer, "nm -x -p ");
# elif defined(PLATFORM_AIX) || defined(PLATFORM_IRIX) || defined(PLATFORM_OSF)
      strcpy(buffer, "nm -x -B ");
# else
      strcpy(buffer, "nm -B ");
# endif
    }
  else /* GNU nm */
    strcpy(buffer, "nm -B ");
  strcat(buffer, global_progname);

  lowestAddress = ULONG_MAX;
  highestAddress = 0;
  fd = my_popen(buffer, &pid);
  if (SYS_ERROR != fd)
    {
      while (my_getline(fd, buffer, sizeof(buffer)))
  {
    if (buffer[0] == '\n')
      continue;
    if (3 == sscanf(buffer, "%lx %c %s", &addr, &type, name))
      {
        if ((type == 't') || type == 'T')
    {
      if (addr == 0)
        continue; /* while */
      if (addr < lowestAddress)
        lowestAddress = addr;
      if (addr > highestAddress)
        highestAddress = addr;
      for (i = 0; syms[i].realAddress != 0; i++)
        {
          if ((addr <= syms[i].realAddress) &&
        (addr > syms[i].closestAddress))
      {
        syms[i].closestAddress = addr;
        strncpy(syms[i].name, name, MAX_BUFFER_SIZE);
        syms[i].name[MAX_BUFFER_SIZE] = (char)0;
        syms[i].type = type;
      }
        }
    }
      }
  }
      my_pclose(fd, pid);

      for (i = 0; syms[i].realAddress != 0; i++)
  {
    if ((syms[i].name[0] == (char)0) ||
        (syms[i].realAddress <= lowestAddress) ||
        (syms[i].realAddress >= highestAddress))
      {
        sprintf(buffer, "[%d] 0x%08lx ???\n", i, syms[i].realAddress);
      }
    else
      {
        sprintf(buffer, "[%d] 0x%08lx <%s + 0x%lx> %c\n",
          i,
          syms[i].realAddress,
          syms[i].name,
          syms[i].realAddress - syms[i].closestAddress,
          syms[i].type);
      }
    write(global_output, buffer, strlen(buffer));
  }
    }
}
#endif

/*************************************************************************
 * DumpStack [private]
 */
static int DumpStack(char *format, ...)
{
  int gotSomething = FALSE;
  int fd;
  pid_t pid;
  int status = EXIT_FAILURE;
  int rc;
  va_list args;
  char *buffer;
  char cmd[MAX_BUFFER_SIZE];
  char buf[MAX_BUFFER_SIZE];

  /*
   * Please note that vsprintf() is not ASync safe (ie. cannot safely
   * be used from a signal handler.) If this proves to be a problem
   * then the cmd string can be built by more basic functions such as
   * strcpy, strcat, and a homemade integer-to-ascii function.
   */
  va_start(args, format);
  vsprintf(cmd, format, args);
  va_end(args);

  fd = my_popen(cmd, &pid);
  if (SYS_ERROR != fd)
    {
      /*
       * Wait for the child to exit. This must be done
       * to make the debugger attach successfully.
       * The output from the debugger is buffered on
       * the pipe.
       *
       * AIX needs the looping hack
       */
      do
  {
    rc = waitpid(pid, &status, 0);
  }
      while ((SYS_ERROR == rc) && (EINTR == errno));

      if ((WIFEXITED(status)) && (WEXITSTATUS(status) == EXIT_SUCCESS))
  {
    while (my_getline(fd, buf, sizeof(buf)))
      {
        buffer = buf;
        if (! gotSomething)
    {
      write(global_output, "Output from ",
      strlen("Output from "));
      strtok(cmd, " ");
      write(global_output, cmd, strlen(cmd));
      write(global_output, "\n", strlen("\n"));
      gotSomething = TRUE;
    }
        if ('\n' == buf[strlen(buf)-1])
    {
      buf[strlen(buf)-1] = (char)0;
    }
        write(global_output, buffer, strlen(buffer));
        write(global_output, "\n", strlen("\n"));
      }
  }
      my_pclose(fd, pid);
    }
  return gotSomething;
}
#endif /* PLATFORM_UNIX */

/*************************************************************************
 * StackTrace
 */
void StackTrace(void)
{
#if defined(PLATFORM_UNIX)
  /*
   * In general dbx seems to do a better job than gdb.
   *
   * Different dbx implementations require different flags/commands.
   */

# if defined(PLATFORM_AIX)

  if (DumpStack("dbx -a %d 2>/dev/null <<EOF\n"
    "where\n"
    "detach\n"
    "EOF\n",
    (int)getpid()))
    return;

  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "where\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

# elif defined(PLATFORM_FREEBSD)

  /*
   * FreeBSD insists on sending a SIGSTOP to the process we
   * attach to, so we let the debugger send a SIGCONT to that
   * process after we have detached.
   */
  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "where\n"
    "detach\n"
    "shell kill -CONT %d\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid(), (int)getpid()))
    return;

# elif defined(PLATFORM_HPUX)

  /*
   * HP decided to call their debugger xdb.
   *
   * This does not seem to work properly yet. The debugger says
   * "Note: Stack traces may not be possible until you are
   *  stopped in user code." on HP-UX 09.01
   *
   * -L = line-oriented interface.
   * "T [depth]" gives a stacktrace with local variables.
   * The final "y" is confirmation to the quit command.
   */

  if (DumpStack("xdb -P %d -L %s 2>&1 <<EOF\n"
    "T 50\n"
    "q\ny\n"
    "EOF\n",
    (int)getpid(), global_progname))
    return;

  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "where\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

#  if defined(PLATFORM_HPUX) && defined(USE_BUILTIN)
  U_STACK_TRACE();
  return;
#  endif

# elif defined(PLATFORM_IRIX)

  /*
   * "set $page=0" drops hold mode
   * "dump ." displays the contents of the variables
   */
  if (DumpStack("dbx -p %d 2>/dev/null <<EOF\n"
    "set \\$page=0\n"
    "where\n"
#  if !defined(__GNUC__)
    /* gcc does not generate this information */
    "dump .\n"
#  endif
    "detach\n"
    "EOF\n",
    (int)getpid()))
    return;

#  if defined(USE_BUILTIN)
  if (trace_back_stack_and_print())
    return;
#  endif

  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "echo --- Stacktrace\\n\n"
    "where\n"
    "echo --- Symbols\\n\n"
    "frame 5\n"      /* Skip signal handler frames */
    "set \\$x = 50\n"
    "while (\\$x)\n" /* Print local variables for each frame */
    "info locals\n"
    "up\n"
    "set \\$x--\n"
    "end\n"
    "echo ---\\n\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

# elif defined(PLATFORM_OSF)

  if (DumpStack("dbx -pid %d %s 2>/dev/null <<EOF\n"
    "where\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    (int)getpid(), global_progname))
    return;

  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "where\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

# elif defined(PLATFORM_SCO)

  /*
   * SCO OpenServer dbx is like a catch-22. The 'detach' command
   * depends on whether ptrace(S) support detaching or not. If it
   * is supported then 'detach' must be used, otherwise the process
   * will be killed upon dbx exit. If it isn't supported then 'detach'
   * will cause the process to be killed. We do not want it to be
   * killed.
   *
   * Out of two evils, the omission of 'detach' was chosen because
   * it worked on our system.
   */
  if (DumpStack("dbx %s %d 2>/dev/null <<EOF\n"
    "where\n"
    "quit\nEOF\n",
    global_progname, (int)getpid()))
    return;

  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "where\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

# elif defined(PLATFORM_SOLARIS)

  if (DumpStack("dbx %s %d 2>/dev/null <<EOF\n"
    "where\n"
    "detach\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

  if (DumpStack("gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt\n"
    "echo --- Stacktrace\\n\n"
    "where\n"
    "echo --- Symbols\\n\n"
    "frame 5\n"      /* Skip signal handler frames */
    "set \\$x = 50\n"
    "while (\\$x)\n" /* Print local variables for each frame */
    "info locals\n"
    "up\n"
    "set \\$x--\n"
    "end\n"
    "echo ---\\n\n"
    "detach\n"
    "quit\n"
    "EOF\n",
    global_progname, (int)getpid()))
    return;

  if (DumpStack("/usr/proc/bin/pstack %d",
    (int)getpid()))
    return;

  /*
   * Other Unices (AIX, HPUX, SCO) also have adb, but
   * they seem unable to attach to a running process.
   */
  if (DumpStack("adb %s 2>&1 <<EOF\n"
    "0t%d:A\n" /* Attach to pid */
    "\\$c\n"   /* print stacktrace */
    ":R\n"     /* Detach */
    "\\$q\n"   /* Quit */
    "EOF\n",
    global_progname, (int)getpid()))
    return;

# else /* All other Unix platforms */

  /*
   * TODO: SCO/UnixWare 7 must be something like (not tested)
   *  debug -i c <pid> <<EOF\nstack -f 4\nquit\nEOF\n
   */

#  if !defined(__GNUC__)
  char line1[] = "dbx %s %d 2>/dev/null <<EOF\n"
                 "where\n"
                 "detach\n"
                 "EOF\n",
    if ( DumpStack( line1, global_progname, (int)getpid()) )
    return;
#  endif

  char line2[] = "gdb -q %s %d 2>/dev/null <<EOF\n"
                "set prompt\n"
                "echo --- Stacktrace\\n\n"
                "where\n"
                "detach\n"
                "quit\n"
                "EOF\n";
   if ( DumpStack( line2, global_progname, (int)getpid()))
    return;

# endif

# if defined(__GNUC__) && defined(USE_BUILTIN)

  GCC_DumpStack();

# endif

  write(global_output,
  "No debugger found\n", strlen("No debugger found\n"));

#elif defined(PLATFORM_WIN32)
  /* Use StackWalk() */
#endif
}

/*************************************************************************
 * StackTraceInit
 */
void StackTraceInit(const char *in_name, int in_handle)
{
  global_progname = in_name;
  global_output = (in_handle == -1) ? STDOUT_FILENO : in_handle;
}
