/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Multi-threaded concurrent line echo server that uses the sock and socks
 * classes.
 *
 ****************************************************************************/

#define _LIBSOCK_USE_SOCKS
#include <libsock/libsock.h>

#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

typedef struct
{
  cw_thd_t send_thd;
  cw_thd_t recv_thd;
  cw_mtx_t lock;
  cw_bool_t should_quit;
  cw_log_t * log;
  cw_sock_t client_sock;
  cw_sock_t remote_sock;
} connection_t;

cw_bool_t should_quit = FALSE;

/* Function prototypes. */
void
sig_int(int a_signum);

void
sig_hup(int a_signum);

cw_bool_t
daemonize(char * a_work_dir);

char *
get_log_str(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str);

void *
handle_client_send(void * a_arg);

void *
handle_client_recv(void * a_arg);

void
sig_int(int a_signum)
{
  extern cw_bool_t should_quit;
  
  _cw_assert(a_signum == SIGINT);

  log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
	      "Caught SIGINT\n");

  should_quit = TRUE;
}

void
sig_hup(int a_signum)
{
  extern cw_bool_t should_quit;
  
  _cw_assert(a_signum == SIGHUP);

  log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
	      "Caught SIGHUP\n");

  should_quit = TRUE;
}

cw_bool_t
daemonize(char * a_work_dir)
{
  cw_bool_t retval = FALSE;

  {
    int pid;
  
    pid = fork();
    if (pid == -1)
    {
      retval = TRUE;
      goto RETURN;
    }
    else if (pid != 0) /* This is the parent. */
    {
      exit(0);
    }
  }
  
  {
    int error;

    error = setsid();
    if (error == -1) /* Already process group leader. */
    {
      retval = TRUE;
      goto RETURN;
    }
  }

  {
    void (*func_ptr)();
    
    func_ptr = signal(SIGHUP, SIG_IGN);
    if (func_ptr == SIG_ERR)
    {
      retval = TRUE;
      goto RETURN;
    }
  }
  
  {
    int pid;
  
    pid = fork();
    if (pid == -1)
    {
      retval = TRUE;
      goto RETURN;
    }
    else if (pid != 0) /* This is the current parent (was child before). */
    {
      exit(0);
    }
  }
  
  {
    int error;

    _cw_assert(a_work_dir != NULL);

    error = chdir(a_work_dir);

    if (error != 0)
    {
      retval = TRUE;
      goto RETURN;
    }
  }

  /* Very conservative umask.  No one else can do anything with our files. */
  umask(0077);

  /* Close all open file descriptors.  We'll assume that 0, 1, and 2 are the
   * only ones open.  */
  {
    int error;

    error = close(0);
    if ((error != 0) && (errno != EBADF))
    {
      retval = TRUE;
      goto RETURN;
    }

    error = close(1);
    if ((error != 0) && (errno != EBADF))
    {
      retval = TRUE;
      goto RETURN;
    }

    error = close(2);
    if ((error != 0) && (errno != EBADF))
    {
      retval = TRUE;
      goto RETURN;
    }
  }  

 RETURN:
  if (retval == FALSE)
  {
    log_lprintf(cw_g_log, "Daemon pid == %d\n", (int) getpid());
  }
  else
  {
    log_leprintf(cw_g_log, NULL, 0, __FUNCTION__, "Error daemonizing.\n");
  }
  
  return retval;
}

char *
get_log_str(cw_buf_t * a_buf, cw_bool_t is_send, char * a_str)
{
  char * retval;
  char c_trans[4], line_a[81], line_b[81],
    line_sep[81]
    = "         |                 |                 |                 |\n";
/*    cw_buf_t t_buf; */
/*    cw_bufel_t * bufel; */
  cw_uint32_t str_len, buf_size, /*  bufel_offset,  */i, j;
  cw_uint8_t c;

/*    buf_new(&t_buf, NULL); */

  buf_size = buf_get_size(a_buf);

  /* Re-alloc enough space to hold the log string. */
  str_len = (81 /* First dashed line. */
	     + 35 /* Header. */
	     + 1 /* Blank line. */
	     + 1 /* Blank line. */
	     + 81 /* Last dashed line. */
	     + 1) /* Terminating \0. */
    + (((buf_size / 16) + 1) * 227); /* 16 bytes data prints as 227 bytes. */
  
  if (a_str == NULL)
  {
    /* Allocate for the first time. */
    retval = _cw_malloc(str_len);
  }
  else
  {
    /* Re-use a_str. */
    retval = _cw_realloc(a_str, str_len);
  }
  /* Clear the string. */
  retval[0] = '\0';

  /* First dashed line. */
  strcat(retval,
	 "----------------------------------------"
	 "----------------------------------------\n");

  sprintf(line_a, "%s:0x%x (%d) byte%s\n",
	  (TRUE == is_send) ? "send" : "recv",
	  buf_size,
	  buf_size,
	  (buf_size != 1) ? "s" : "");
  strcat(retval, line_a);

  /* Blank line. */
  strcat(retval, "\n");

/*    bufel = buf_rm_head_bufel(a_buf); */
/*    if (bufel != NULL) */
/*    { */
/*      bufel_offset = bufel_get_beg_offset(bufel); */
/*    } */
  for (i = 0; i < buf_size; i += j)
  {
    /* Clear the line buffers and get them set up for printing character
     * translations. */
    line_a[0] = '\0';
    line_b[0] = '\0';
    sprintf(line_a, "%08x", i);
    strcat(line_b, "        ");
    
    /* Each iteration generates log text for 16 bytes of data. */
    for (j = 0; (j < 16) && ((i + j) < buf_size); j++)
    {
      if ((j % 4) == 0)
      {
	/* Print the word separators. */
	strcat(line_a, " |");
	strcat(line_b, " |");
      }
	
      /* Get a bufel with data in it if necessary. */
/*        while (bufel_offset >= bufel_get_end_offset(bufel)) */
/*        { */
/*  	buf_append_bufel(&t_buf, bufel); */
/*  	bufel = buf_rm_head_bufel(a_buf); */
/*  	_cw_check_ptr(bufel); */
/*  	bufel_offset = bufel_get_beg_offset(bufel); */
/*        } */

      /* Print the next character. */
/*        c = bufel_get_uint8(bufel, bufel_offset); */
      c = buf_get_uint8(a_buf, i);
      
      switch (c)
      {
	case 0x00:
	  strcpy(c_trans, "nul");
	  break;
	case 0x01:
	  strcpy(c_trans, "soh");
	  break;
	case 0x02:
	  strcpy(c_trans, "stx");
	  break;
	case 0x03:
	  strcpy(c_trans, "etx");
	  break;
	case 0x04:
	  strcpy(c_trans, "eot");
	  break;
	case 0x05:
	  strcpy(c_trans, "enq");
	  break;
	case 0x06:
	  strcpy(c_trans, "ack");
	  break;
	case 0x07:
	  strcpy(c_trans, "bel");
	  break;
	case 0x08:
	  strcpy(c_trans, "bs");
	  break;
	case 0x09:
	  strcpy(c_trans, "ht");
	  break;
	case 0x0a:
	  strcpy(c_trans, "lf");
	  break;
	case 0x0b:
	  strcpy(c_trans, "vt");
	  break;
	case 0x0c:
	  strcpy(c_trans, "ff");
	  break;
	case 0x0d:
	  strcpy(c_trans, "cr");
	  break;
	case 0x0e:
	  strcpy(c_trans, "so");
	  break;
	case 0x0f:
	  strcpy(c_trans, "si");
	  break;
	  
	case 0x10:
	  strcpy(c_trans, "dle");
	  break;
	case 0x11:
	  strcpy(c_trans, "dc1");
	  break;
	case 0x12:
	  strcpy(c_trans, "dc2");
	  break;
	case 0x13:
	  strcpy(c_trans, "dc3");
	  break;
	case 0x14:
	  strcpy(c_trans, "dc4");
	  break;
	case 0x15:
	  strcpy(c_trans, "ack");
	  break;
	case 0x16:
	  strcpy(c_trans, "syn");
	  break;
	case 0x17:
	  strcpy(c_trans, "etb");
	  break;
	case 0x18:
	  strcpy(c_trans, "can");
	  break;
	case 0x19:
	  strcpy(c_trans, "em");
	  break;
	case 0x1a:
	  strcpy(c_trans, "sub");
	  break;
	case 0x1b:
	  strcpy(c_trans, "ec");
	  break;
	case 0x1c:
	  strcpy(c_trans, "fs");
	  break;
	case 0x1d:
	  strcpy(c_trans, "gs");
	  break;
	case 0x1e:
	  strcpy(c_trans, "rs");
	  break;
	case 0x1f:
	  strcpy(c_trans, "us");
	  break;

	case 0x20:
	  strcpy(c_trans, "sp");
	  break;
	case 0x21:
	  strcpy(c_trans, "!");
	  break;
	case 0x22:
	  strcpy(c_trans, "\"");
	  break;
	case 0x23:
	  strcpy(c_trans, "#");
	  break;
	case 0x24:
	  strcpy(c_trans, "$");
	  break;
	case 0x25:
	  strcpy(c_trans, "%");
	  break;
	case 0x26:
	  strcpy(c_trans, "&");
	  break;
	case 0x27:
	  strcpy(c_trans, "'");
	  break;
	case 0x28:
	  strcpy(c_trans, "(");
	  break;
	case 0x29:
	  strcpy(c_trans, ")");
	  break;
	case 0x2a:
	  strcpy(c_trans, "*");
	  break;
	case 0x2b:
	  strcpy(c_trans, "+");
	  break;
	case 0x2c:
	  strcpy(c_trans, ",");
	  break;
	case 0x2d:
	  strcpy(c_trans, "-");
	  break;
	case 0x2e:
	  strcpy(c_trans, ".");
	  break;
	case 0x2f:
	  strcpy(c_trans, "/");
	  break;

	case 0x30:
	  strcpy(c_trans, "0");
	  break;
	case 0x31:
	  strcpy(c_trans, "1");
	  break;
	case 0x32:
	  strcpy(c_trans, "2");
	  break;
	case 0x33:
	  strcpy(c_trans, "3");
	  break;
	case 0x34:
	  strcpy(c_trans, "4");
	  break;
	case 0x35:
	  strcpy(c_trans, "5");
	  break;
	case 0x36:
	  strcpy(c_trans, "6");
	  break;
	case 0x37:
	  strcpy(c_trans, "7");
	  break;
	case 0x38:
	  strcpy(c_trans, "8");
	  break;
	case 0x39:
	  strcpy(c_trans, "9");
	  break;
	case 0x3a:
	  strcpy(c_trans, ":");
	  break;
	case 0x3b:
	  strcpy(c_trans, ";");
	  break;
	case 0x3c:
	  strcpy(c_trans, "<");
	  break;
	case 0x3d:
	  strcpy(c_trans, "=");
	  break;
	case 0x3e:
	  strcpy(c_trans, ">");
	  break;
	case 0x3f:
	  strcpy(c_trans, "?");
	  break;

  	case 0x40:
	  strcpy(c_trans, "@");
	  break;
	case 0x41:
	  strcpy(c_trans, "A");
	  break;
	case 0x42:
	  strcpy(c_trans, "B");
	  break;
	case 0x43:
	  strcpy(c_trans, "C");
	  break;
	case 0x44:
	  strcpy(c_trans, "D");
	  break;
	case 0x45:
	  strcpy(c_trans, "E");
	  break;
	case 0x46:
	  strcpy(c_trans, "F");
	  break;
	case 0x47:
	  strcpy(c_trans, "G");
	  break;
	case 0x48:
	  strcpy(c_trans, "H");
	  break;
	case 0x49:
	  strcpy(c_trans, "I");
	  break;
	case 0x4a:
	  strcpy(c_trans, "J");
	  break;
	case 0x4b:
	  strcpy(c_trans, "K");
	  break;
	case 0x4c:
	  strcpy(c_trans, "L");
	  break;
	case 0x4d:
	  strcpy(c_trans, "M");
	  break;
	case 0x4e:
	  strcpy(c_trans, "N");
	  break;
	case 0x4f:
	  strcpy(c_trans, "O");
	  break;

  	case 0x50:
	  strcpy(c_trans, "P");
	  break;
	case 0x51:
	  strcpy(c_trans, "Q");
	  break;
	case 0x52:
	  strcpy(c_trans, "R");
	  break;
	case 0x53:
	  strcpy(c_trans, "S");
	  break;
	case 0x54:
	  strcpy(c_trans, "T");
	  break;
	case 0x55:
	  strcpy(c_trans, "U");
	  break;
	case 0x56:
	  strcpy(c_trans, "V");
	  break;
	case 0x57:
	  strcpy(c_trans, "W");
	  break;
	case 0x58:
	  strcpy(c_trans, "X");
	  break;
	case 0x59:
	  strcpy(c_trans, "Y");
	  break;
	case 0x5a:
	  strcpy(c_trans, "Z");
	  break;
	case 0x5b:
	  strcpy(c_trans, "[");
	  break;
	case 0x5c:
	  strcpy(c_trans, "\\");
	  break;
	case 0x5d:
	  strcpy(c_trans, "]");
	  break;
	case 0x5e:
	  strcpy(c_trans, "^");
	  break;
	case 0x5f:
	  strcpy(c_trans, "_");
	  break;

  	case 0x60:
	  strcpy(c_trans, "`");
	  break;
	case 0x61:
	  strcpy(c_trans, "a");
	  break;
	case 0x62:
	  strcpy(c_trans, "b");
	  break;
	case 0x63:
	  strcpy(c_trans, "c");
	  break;
	case 0x64:
	  strcpy(c_trans, "d");
	  break;
	case 0x65:
	  strcpy(c_trans, "e");
	  break;
	case 0x66:
	  strcpy(c_trans, "f");
	  break;
	case 0x67:
	  strcpy(c_trans, "g");
	  break;
	case 0x68:
	  strcpy(c_trans, "h");
	  break;
	case 0x69:
	  strcpy(c_trans, "i");
	  break;
	case 0x6a:
	  strcpy(c_trans, "j");
	  break;
	case 0x6b:
	  strcpy(c_trans, "k");
	  break;
	case 0x6c:
	  strcpy(c_trans, "l");
	  break;
	case 0x6d:
	  strcpy(c_trans, "m");
	  break;
	case 0x6e:
	  strcpy(c_trans, "n");
	  break;
	case 0x6f:
	  strcpy(c_trans, "o");
	  break;

  	case 0x70:
	  strcpy(c_trans, "p");
	  break;
	case 0x71:
	  strcpy(c_trans, "q");
	  break;
	case 0x72:
	  strcpy(c_trans, "r");
	  break;
	case 0x73:
	  strcpy(c_trans, "s");
	  break;
	case 0x74:
	  strcpy(c_trans, "t");
	  break;
	case 0x75:
	  strcpy(c_trans, "u");
	  break;
	case 0x76:
	  strcpy(c_trans, "v");
	  break;
	case 0x77:
	  strcpy(c_trans, "w");
	  break;
	case 0x78:
	  strcpy(c_trans, "x");
	  break;
	case 0x79:
	  strcpy(c_trans, "y");
	  break;
	case 0x7a:
	  strcpy(c_trans, "z");
	  break;
	case 0x7b:
	  strcpy(c_trans, "{");
	  break;
	case 0x7c:
	  strcpy(c_trans, "|");
	  break;
	case 0x7d:
	  strcpy(c_trans, "}");
	  break;
	case 0x7e:
	  strcpy(c_trans, "~");
	  break;
	case 0x7f:
	  strcpy(c_trans, "del");
	  break;

	default:
	  strcpy(c_trans, "---");
	  break;
      }
      
      sprintf(line_a + strlen(line_a), "  %02x", c);
      sprintf(line_b + strlen(line_b), " %3s", c_trans);

/*        bufel_offset++; */
    }
    /* Actually copy the strings to the final output string. */
    strcat(retval, line_a);
    strcat(retval, "\n");
    
    strcat(retval, line_b);
    strcat(retval, "\n");

    if ((i + j) < buf_size)
    {
      strcat(retval, line_sep);
    }
    else
    {
      strcat(retval, "\n");
    }
  }

/*    if (bufel != NULL) */
/*    { */
/*      buf_append_bufel(&t_buf, bufel); */
/*    } */

  /* Last dashed line. */
  strcat(retval,
	 "----------------------------------------"
	 "----------------------------------------\n");

  /* Move all the data back to a_buf. */
/*    buf_append_buf(a_buf, &t_buf); */
/*    buf_delete(&t_buf); */

  return retval;
}

void *
handle_client_send(void * a_arg)
{
  cw_buf_t buf;
  connection_t * conn = (connection_t *) a_arg;
  char * str = NULL;
  char * hostname = NULL, * port_str = NULL;
  cw_bool_t parse_error = TRUE;

  log_printf(cw_g_log, "New connection\n");

  buf_new(&buf, FALSE);

  /* Finish initializing conn. */
  mtx_new(&conn->lock);
  conn->should_quit = FALSE;
  conn->log = cw_g_log; /* XXX */
/*   conn->log = log_new(); */
/*   if (log_set_logfile(conn->log, "/space/home/jasone/logfile_XXX", TRUE)) */
/*   { */
/*     log_printf(cw_g_log, "Error creating log file"); */
/*     goto RETURN; */
/*   } */

  /* Parse the proxy options from the socket stream.  The syntax is:
   * hostname:port[\r]\n */
  {
    int port;
#if (0)
    cw_uint32_t offset;
    cw_bufel_t * bufel;
    enum
    {
      SERVER,
      PORT,
      DONE
    } state;

    hostname = (char *) _cw_malloc(1);
    hostname[0] = '\0';

    port_str = (char *) _cw_malloc(1);
    port_str[0] = '\0';

    for (bufel = NULL, state = SERVER;
	 state != DONE;
	 )
    {
      if ((bufel == NULL) /* Short circuits, so don't worry about the second
                             part executing with a NULL pointer. */
	  || (0 == bufel_get_valid_data_size(bufel)))
      {
	if (bufel != NULL)
	{
	  /* bufel doesn't have any valid data, so throw it away. */
	  sockb_put_spare_bufel(bufel);
	  bufel = NULL;
	}

	if (buf_get_size(&buf) == 0)
	{
	  /* Need more data. */
	  if (-1 == sock_read_block(&conn->client_sock, &buf))
	  {
	    log_printf(conn->log, "Socket closed while parsing commands\n");
	    parse_error = TRUE;
	    goto RETURN;
	  }
	  else
	  {
/*  	log_printf(cw_g_log, "%s", get_log_str(&buf, TRUE, str)); */
	    bufel = buf_rm_head_bufel(&buf);
	  }
	}
      }
      _cw_check_ptr(bufel);

      switch (state)
      {
	case SERVER:
	{
	  for (offset = bufel_get_beg_offset(bufel);
	       0 < bufel_get_valid_data_size(bufel);
	       offset++)
	  {
	    if (bufel_get_uint8(bufel, offset) != ':')
	    {
	      cw_uint32_t str_len = strlen(hostname);
	    
	      hostname = (char *) _cw_realloc(hostname, str_len + 2);
	      hostname[str_len] = bufel_get_uint8(bufel, offset);
	      hostname[str_len + 1] = '\0';
	      bufel_set_beg_offset(bufel, offset + 1);
	    }
	    else
	    {
	      bufel_set_beg_offset(bufel, offset + 1);
	      state = PORT;
	      break;
	    }
	  }
	  break;
	}
	case PORT:
	{
	  for (offset = bufel_get_beg_offset(bufel);
	       0 < bufel_get_valid_data_size(bufel);
	       offset++)
	  {
	    if (bufel_get_uint8(bufel, offset) != '\n')
	    {
	      cw_uint32_t str_len = strlen(port_str);
	    
	      port_str = (char *) _cw_realloc(port_str, str_len + 2);
	      port_str[str_len] = bufel_get_uint8(bufel, offset);
	      port_str[str_len + 1] = '\0';
	      bufel_set_beg_offset(bufel, offset + 1);
	    }
	    else
	    {
	      bufel_set_beg_offset(bufel, offset + 1);
	      port = atoi(port_str);
	      if (0 < bufel_get_valid_data_size(bufel))
	      {
		buf_prepend_bufel(&buf, bufel);
	      }
	      state = DONE;
	      break;
	    }
	  }
	  break;
	}
	default:
	{
	  _cw_error("Unexpected state");
	}
      }
    }
#endif
    /* XXX */
    hostname = "localhost";
/*      port = 23; */
    port = 6000;
    
    log_printf(conn->log, "Connecting to \"%s\" on port %d\n", hostname, port);
      
    /* Open a connection as specified by the proxy options. */

    /* Connect to the remote end, using hostname and port. */
    sock_new(&conn->remote_sock, 8192);
    if (TRUE == sock_connect(&conn->remote_sock, hostname, port))
    {
      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		  "Error in sock_connect(&conn->remote_sock, \"%s\", %d)\n",
		  hostname, port);
      goto RETURN;
    }
  }
  
  /* Okay, the connection is totally established now.  Create another thread to
   * handle the data being sent from the remote end back to the client.  That
   * allows both this thread and the new one to block, waiting for data, rather
   * than polling. */
  thd_new(&conn->recv_thd, handle_client_recv, (void *) conn);

  /* Continually read data from the socket, create a log string, print to the
   * log, then send the data on. */
  while (FALSE == conn->should_quit)
  {
    if (-1 == sock_read_block(&conn->client_sock, &buf))
    {
      mtx_lock(&conn->lock);
      conn->should_quit = TRUE;
      mtx_unlock(&conn->lock);
      break;
    }
    else if (conn->should_quit)
    {
      break;
    }
    else
    {
/*        str = get_log_str(&buf, TRUE, str); */
/*        log_printf(conn->log, "%s", str); */
      if (-1 == sock_write(&conn->remote_sock, &buf))
      {
	mtx_lock(&conn->lock);
	conn->should_quit = TRUE;
	mtx_unlock(&conn->lock);
	break;
      }
    }
  }
  sock_disconnect(&conn->remote_sock);

  if (NULL != str)
  {
    _cw_free(str);
  }
    
/*    JOIN: */
  /* Join on the recv thread. */
  thd_join(&conn->recv_thd);
  thd_delete(&conn->recv_thd);

  RETURN:
/*    _cw_free(hostname); */
/*    _cw_free(port_str); */
  
  /* Delete this thread. */
  thd_delete(&conn->send_thd);

  /* Finish cleaning up conn. */
  mtx_delete(&conn->lock);
/*   log_delete(conn->log); */
  sock_delete(&conn->client_sock);
  /* XXX Don't do this if the socket wasn't created. */
  sock_delete(&conn->remote_sock);
  
  _cw_free(conn);
  
  buf_delete(&buf);

  log_printf(cw_g_log, "Connection closed\n");
  
  return NULL;
}

void *
handle_client_recv(void * a_arg)
{
  cw_buf_t buf;
  connection_t * conn = (connection_t *) a_arg;
  char * str = NULL;

  buf_new(&buf, FALSE);
  
  /* Continually read data from the socket, create a log string, print to the
   * log, then send the data on. */
  while (FALSE == conn->should_quit)
  {
    if ((-1 == sock_read_block(&conn->remote_sock, &buf))
	|| (conn->should_quit))
    {
      mtx_lock(&conn->lock);
      conn->should_quit = TRUE;
      mtx_unlock(&conn->lock);
      break;
    }
    else
    {
/*        str = get_log_str(&buf, FALSE, str); */
/*        log_printf(conn->log, "%s", str); */
      if (-1 == sock_write(&conn->client_sock, &buf))
      {
	mtx_lock(&conn->lock);
	conn->should_quit = TRUE;
	mtx_unlock(&conn->lock);
	break;
      }
    }
  }
  sock_disconnect(&conn->client_sock);

  if (NULL != str)
  {
    _cw_free(str);
  }
  buf_delete(&buf);
  return NULL;
}

int
main(int argc, char ** argv)
{
  cw_socks_t * socks;
  connection_t * conn;
  int port;

  if (argc != 2)
  {
    port = 0;
  }
  else
  {
    port = strtol(argv[1], (char **) NULL, 10);
  }

  /* XXX */
  port = 6010;
  
  libstash_init();

  /* XXX Set the per-thread signal masks such that only one thread will catch
   * the signal. */
  if (SIG_ERR == signal(SIGINT, sig_int))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error setting signal handler for SIGINT\n");
  }
  if (SIG_ERR == signal(SIGHUP, sig_int))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"Error setting signal handler for SIGHUP\n");
  }
  
/*    log_set_logfile(cw_g_log, "/tmp/b_proxy.log", FALSE); */

/*    if (TRUE == daemonize("/tmp")) */
/*    { */
/*      abort(); */
/*    } */
  log_printf(cw_g_log, "pid: %d\n", getpid());

/*    sockb_init(512, 512); */
  sockb_init(4096, 1024);
  
  dbg_register(cw_g_dbg, "mem_error");
  dbg_register(cw_g_dbg, "mem_verbose");
/*    dbg_register(cw_g_dbg, "sockb_verbose"); */
  dbg_register(cw_g_dbg, "sockb_error");
/*    dbg_register(cw_g_dbg, "socks_verbose"); */
  dbg_register(cw_g_dbg, "socks_error");
/*    dbg_register(cw_g_dbg, "sock_verbose"); */
  dbg_register(cw_g_dbg, "sock_error");
  
  socks = socks_new();
  if (TRUE == socks_listen(socks, &port))
  {
    _cw_error("socks_listen() error");
  }
  log_lprintf(cw_g_log, "%s: Listening on port %d\n", argv[0], port);

  while (should_quit == FALSE)
  {
    conn = _cw_malloc(sizeof(connection_t));
    bzero(conn, sizeof(conn));
    sock_new(&conn->client_sock, 8192);
    
    if (NULL == socks_accept_block(socks, &conn->client_sock))
    {
      log_lprintf(cw_g_log, "socks_accept_block() error");
      sock_delete(&conn->client_sock);
      _cw_free(conn);
    }
    else
    {
      thd_new(&conn->send_thd, handle_client_send, (void *) conn);
    }
  }

  socks_delete(socks);
  
  sockb_shutdown();
  libstash_shutdown();
  return 0;
}
