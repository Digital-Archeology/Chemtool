/*
    libundo, an easy to use undo/redo management library
    Copyright 1999 Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>


#define BLOCK_FLAG_LARGE	0x01

#define MEMORY_RAW_SIZE(mem)	(*(size_t *)(mem))
#define MEMORY_USED(mem)		(MEMORY_RAW_SIZE(mem) & 1)
#define MEMORY_SIZE(mem)		(MEMORY_RAW_SIZE(mem) & ~1)

#define MEMORY_SET_SIZE_USED(mem, size, used) \
			(MEMORY_RAW_SIZE(mem) = ((size) & ~1) | ((used) ? 1 : 0))

#define MEMORY_NEXT(mem) (MEMORY_OFFSET((mem), MEMORY_SIZE(mem)))
#define MEMORY_BODY(mem) (MEMORY_OFFSET((mem), UNDO_MEMORY_OVERHEAD))

#define FOREACH_SIZED_BLOCK(mem, size, block_ix, block) \
			for((block_ix) = 0, (block) = &(mem)->size##_alloc_list[0]; \
				(block_ix) < (mem)->size##_alloc_list_count; \
				(block_ix)++, (block) = &(mem)->size##_alloc_list[(block_ix)])
#define FOREACH_SMALL_BLOCK(mem, block_ix, block) \
			FOREACH_SIZED_BLOCK(mem, small, block_ix, block)
#define FOREACH_LARGE_BLOCK(mem, block_ix, block) \
			FOREACH_SIZED_BLOCK(mem, large, block_ix, block)

#define MEMORY_BACKUP_OVERHEAD(mem) \
			((mem) = MEMORY_OFFSET((mem), -UNDO_MEMORY_OVERHEAD))

#define BLOCK_END(block) MEMORY_OFFSET((block).mem, (block).size)
#define IN_BLOCK(block, m) ((m) >= (block).mem && (m) < BLOCK_END(block))
#define BLOCK_PAGES(block) ((block).size / getpagesize())
#define FOREACH_IN_BLOCK(block, mem) \
			for((mem) = (block).mem; \
				(mem) != BLOCK_END(block); \
				(mem) = MEMORY_NEXT(mem))

/* Some systems define MAP_ANON instead of MAP_ANONYMOUS. */
/* Some (e.g. Solaris <8) have neither, but  map /dev/zero instead */
#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static int dev_zero_fd;
#endif /* MAP_ANON */
#endif /* MAP_ANONYMOUS */

#ifdef MAP_ANONYMOUS
#define MAP_NEW_ANON_AT_FLAGS(pos, size, flags) \
            (mmap((pos), (size), \
			      PROT_READ | PROT_WRITE | PROT_EXEC, \
				  MAP_PRIVATE | MAP_ANONYMOUS | (flags),-1, 0))
#else
#define MAP_NEW_ANON_AT_FLAGS(pos, size, flags) \
            (mmap((pos), (size), \
			      PROT_READ | PROT_WRITE | PROT_EXEC, \
				  MAP_PRIVATE | (flags),dev_zero_fd, 0))
#endif				  
#define MAP_NEW_ANON(size) MAP_NEW_ANON_AT_FLAGS(0, (size), 0)
#define MAP_NEW_ANON_AT(pos, size) \
            MAP_NEW_ANON_AT_FLAGS((pos), (size), MAP_FIXED)
#define PAGE_REMAINDER(size) (getpagesize() - (size))

#define IF_IN_BLOCK_VAR \
			unsigned _if_in_block_ix; \
			UNDO_BLOCK *_if_in_block
#define IF_IN_SMALL_BLOCK(memory, ptr) \
			FOREACH_SMALL_BLOCK((memory), _if_in_block_ix, _if_in_block) \
				if(IN_BLOCK(*_if_in_block, (ptr)))
#define IF_IN_LARGE_BLOCK(memory, ptr) \
			FOREACH_LARGE_BLOCK((memory), _if_in_block_ix, _if_in_block) \
				if((ptr) == _if_in_block->mem)

#define NEW(type) ((type *)calloc(1, sizeof(type)))

#define UNDO_SUCCESS            return UNDO_NOERROR;
#define UNDO_ERROR(err)         return (undo_set_error(err), (err));
#define UNDO_ERROR_NULL(err)    return (undo_set_error(err), (void*)NULL);

#ifdef LIBUNDO
void undo_set_error (int err);

typedef struct _UNDO_HISTORY UNDO_HISTORY;
typedef struct _UNDO_HISTORY_ITEM UNDO_HISTORY_ITEM;



struct _UNDO_HISTORY
{
  UNDO_HISTORY_ITEM *item;
  unsigned length;
  unsigned ix;
  size_t memory_limit;
};

struct _UNDO_HISTORY_ITEM
{
  void *mem;
  size_t size;
};

typedef struct _UNDO_MEMORY_STREAM UNDO_MEMORY_STREAM;
typedef struct _UNDO_MEMORY_STREAM_BLOCK_HEADER
  UNDO_MEMORY_STREAM_BLOCK_HEADER;

#define MEMORY_OFFSET(mem, offset) ((void *)((char *)(mem) + (offset)))

struct _UNDO_MEMORY_STREAM
{
  void (*destroy) (UNDO_MEMORY_STREAM * stream);
    size_t (*read) (UNDO_MEMORY_STREAM * stream, size_t offset,
		    void *mem, size_t count);
  void *implementation;
};

UNDO_HISTORY *undo_history_new (void);
int undo_history_destroy (UNDO_HISTORY * history);

int undo_history_record (UNDO_HISTORY * history, UNDO_MEMORY_STREAM * stream);

UNDO_MEMORY_STREAM *undo_history_undo (UNDO_HISTORY * history);
UNDO_MEMORY_STREAM *undo_history_redo (UNDO_HISTORY * history);

unsigned undo_history_undo_count (UNDO_HISTORY * history);
unsigned undo_history_redo_count (UNDO_HISTORY * history);

void undo_history_set_memory_limit (UNDO_HISTORY * history, size_t limit);
size_t undo_history_memory_usage (UNDO_HISTORY * history);

#include "undo.h"



size_t undo_memory_stream_write (size_t offset, void *mem, size_t size,
				 size_t * pos, void *data, size_t length);
size_t undo_memory_stream_length (UNDO_MEMORY_STREAM * stream);

/*  This might not be the same as sizeof(UNDO_MEMORY_STREAM_BLOCK_HEADER)
	if the compiler pads the structure in some way.  This value represents
	the size of the header when it is serialized in a stream.  */
#define STREAM_SERIALIZED_BLOCK_HEADER_SIZE (sizeof(void *) + \
                                    sizeof(size_t) + sizeof(unsigned))
struct _UNDO_MEMORY_STREAM_BLOCK_HEADER
{
  void *addr;
  size_t size;
  unsigned flags;
};

size_t undo_memory_stream_read_header (UNDO_MEMORY_STREAM * stream,
				       size_t offset,
				       UNDO_MEMORY_STREAM_BLOCK_HEADER *
				       header);

typedef struct _UNDO_BLOCK UNDO_BLOCK;
typedef struct _UNDO_MEMORY UNDO_MEMORY;

struct _UNDO_BLOCK
{
  void *mem;
  size_t size;
};

struct _UNDO_MEMORY
{
  UNDO_BLOCK *small_alloc_list;
  unsigned small_alloc_list_count;

  UNDO_BLOCK *large_alloc_list;
  unsigned large_alloc_list_count;
};

struct _UNDO
{
  char *name;

  UNDO_MEMORY *memory;
  UNDO_HISTORY *history;
};


UNDO_MEMORY *undo_memory_new (void);
int undo_memory_destroy (UNDO_MEMORY * memory);

size_t undo_memory_used (UNDO_MEMORY * memory);
unsigned undo_memory_pages_used (UNDO_MEMORY * memory);

void *undo_memory_alloc (UNDO_MEMORY * memory, size_t size);
int undo_memory_free (UNDO_MEMORY * memory, void *alloc);
size_t undo_memory_size (UNDO_MEMORY * memory, void *alloc);

UNDO_MEMORY_STREAM *undo_memory_stream (UNDO_MEMORY * memory);
int undo_memory_set (UNDO_MEMORY * memory, UNDO_MEMORY_STREAM * stream);

#define UNDO_MEMORY_OVERHEAD (sizeof(size_t))


static UNDO *undo_session = NULL;


int
undo_new (char *session_name)
{
  if (session_name == NULL)
    UNDO_ERROR (UNDO_BADPARAM);

  undo_session = NEW (UNDO);
  if (undo_session == NULL)
    UNDO_ERROR (UNDO_NOMEM);

#ifndef MAP_ANONYMOUS
dev_zero_fd = open("/dev/zero",O_RDONLY);
#endif

  undo_session->name = (char *) malloc (strlen (session_name) + 1);
  if (undo_session->name == NULL)
    {
      undo_destroy ();
      UNDO_ERROR (UNDO_NOMEM);
    }
  strcpy (undo_session->name, session_name);

  undo_session->memory = undo_memory_new ();
  if (undo_session->memory == NULL)
    {
      undo_destroy ();
      UNDO_ERROR (UNDO_NOMEM);
    }

  undo_session->history = undo_history_new ();
  if (undo_session->history == NULL)
    {
      undo_destroy ();
      UNDO_ERROR (UNDO_NOMEM);
    }

  UNDO_SUCCESS;
}

int
undo_destroy (void)
{
  if (undo_session == NULL)
    UNDO_ERROR (UNDO_NOSESSION);

  if (undo_session->name)
    free (undo_session->name);

  if (undo_session->memory)
    undo_memory_destroy (undo_session->memory);

  if (undo_session->history)
    undo_history_destroy (undo_session->history);

  undo_session = NULL;
#ifndef MAP_ANONYMOUS
	close(dev_zero_fd);
#endif
	
  UNDO_SUCCESS;
}

UNDO *
undo_get_session (void)
{
  return undo_session;
}

int
undo_set_session (UNDO * undo)
{
  undo_session = undo;

  UNDO_SUCCESS;
}

int
undo_set_memory_limit (size_t max_memory)
{
  if (undo_session == NULL)
    UNDO_ERROR (UNDO_NOSESSION);

  undo_history_set_memory_limit (undo_session->history, max_memory);

  UNDO_SUCCESS;
}

unsigned
undo_get_undo_count (void)
{
  if (undo_session == NULL)
    return 0;

  return undo_history_undo_count (undo_session->history);
}

unsigned
undo_get_redo_count (void)
{
  if (undo_session == NULL)
    return 0;

  return undo_history_redo_count (undo_session->history);
}

int
undo_undo (void)
{
  UNDO_MEMORY_STREAM *stream;
  int ret;

  if (undo_session == NULL)
    UNDO_ERROR (UNDO_NOSESSION);

  if (undo_history_undo_count (undo_session->history) == 0)
    UNDO_ERROR (UNDO_NODO);

  stream = undo_history_undo (undo_session->history);
  if (stream == NULL)
    UNDO_ERROR (UNDO_NOMEM);
  ret = undo_memory_set (undo_session->memory, stream);
  stream->destroy (stream);

  return ret;
}

int
undo_redo (void)
{
  UNDO_MEMORY_STREAM *stream;
  int ret;

  if (undo_session == NULL)
    UNDO_ERROR (UNDO_NOSESSION);

  if (undo_history_redo_count (undo_session->history) == 0)
    UNDO_ERROR (UNDO_NODO);

  stream = undo_history_redo (undo_session->history);
  if (stream == NULL)
    UNDO_ERROR (UNDO_NOMEM);
  ret = undo_memory_set (undo_session->memory, stream);
  stream->destroy (stream);

  return ret;
}

int
undo_snapshot (void)
{
  UNDO_MEMORY_STREAM *stream;
  int ret;

  if (undo_session == NULL)
    UNDO_ERROR (UNDO_NOSESSION);

  stream = undo_memory_stream (undo_session->memory);
  if (stream == NULL)
    UNDO_ERROR (UNDO_NOMEM);
  ret = undo_history_record (undo_session->history, stream);
  stream->destroy (stream);

  return ret;
}

void *
undo_malloc (size_t size)
{
  if (undo_session == NULL)
    UNDO_ERROR_NULL (UNDO_NOSESSION);

  return undo_memory_alloc (undo_session->memory, size);
}

void *
undo_realloc (void *mem, size_t size)
{
  size_t min_size;
  void *new_mem;

  if (undo_session == NULL)
    UNDO_ERROR_NULL (UNDO_NOSESSION);

  if (mem == NULL)
    return undo_memory_alloc (undo_session->memory, size);

  min_size = undo_memory_size (undo_session->memory, mem);
  if (size < min_size)
    min_size = size;

  if (size == min_size)
    return mem;

  new_mem = undo_memory_alloc (undo_session->memory, size);
  if (new_mem == NULL)
    UNDO_ERROR_NULL (UNDO_NOMEM);

  memcpy (new_mem, mem, min_size);
  undo_memory_free (undo_session->memory, mem);

  return new_mem;
}

void
undo_free (void *mem)
{
  if (undo_session == NULL)
    return;

  undo_memory_free (undo_session->memory, mem);
}

static void *undo_memory_alloc_small (UNDO_MEMORY * memory, size_t size);
static void *undo_memory_alloc_large (UNDO_MEMORY * memory, size_t size);
static void *undo_memory_alloc_small_block (UNDO_BLOCK * block, size_t size);
static void undo_memory_block_coalesce_free (UNDO_BLOCK * block);
static void *undo_memory_alloc_new_small_block (UNDO_MEMORY * mem,
						size_t size);
static void *undo_memory_new_block (UNDO_BLOCK ** block_list,
				    unsigned *length, size_t size);
static int undo_memory_new_block_record (UNDO_BLOCK ** block_list,
					 unsigned *len, void *mem,
					 size_t size);
static int undo_memory_delete_block (UNDO_BLOCK ** block_list,
				     unsigned *length, unsigned index);

static void undo_stream_destroy (UNDO_MEMORY_STREAM * stream);
static size_t undo_stream_read (UNDO_MEMORY_STREAM * stream, size_t offset,
				void *mem, size_t size);
static size_t undo_stream_block_write (size_t offset, void *mem, size_t size,
				       size_t * pos,
				       UNDO_BLOCK * block, unsigned flags);
static void undo_memory_clear (UNDO_MEMORY * memory);
static int undo_memory_add_blocks_from_stream (UNDO_MEMORY * memory,
					       UNDO_MEMORY_STREAM * stream);
static void undo_memory_block_header_record (UNDO_MEMORY * memory,
					     UNDO_MEMORY_STREAM_BLOCK_HEADER *
					     header);

UNDO_MEMORY *
undo_memory_new (void)
{
  UNDO_MEMORY *mem;

  mem = NEW (UNDO_MEMORY);
  if (mem == NULL)
    UNDO_ERROR_NULL (UNDO_NOMEM);

  return mem;
}

int
undo_memory_destroy (UNDO_MEMORY * memory)
{
  if (memory == NULL)
    UNDO_ERROR (UNDO_BADPARAM);

  undo_memory_clear (memory);
  free (memory);

  UNDO_SUCCESS;
}

size_t
undo_memory_used (UNDO_MEMORY * memory)
{
  size_t used;
  unsigned block_ix;
  UNDO_BLOCK *block;

  used = 0;
  FOREACH_SMALL_BLOCK (memory, block_ix, block)
  {
    void *mem;

    FOREACH_IN_BLOCK (*block, mem)
    {
      if (MEMORY_USED (mem))
	{
	  used += MEMORY_SIZE (mem);
	}
    }
  }

  FOREACH_LARGE_BLOCK (memory, block_ix, block)
  {
    used += block->size;
  }

  return used;
}

void *
undo_memory_alloc (UNDO_MEMORY * memory, size_t size)
{
  while (size & (sizeof (size_t) - 1))
    size++;

  if (size < getpagesize () - UNDO_MEMORY_OVERHEAD * 2)
    {
      return undo_memory_alloc_small (memory, size);
    }
  else
    {
      return undo_memory_alloc_large (memory, size);
    }
}

int
undo_memory_free (UNDO_MEMORY * memory, void *alloc)
{
  IF_IN_BLOCK_VAR;

  IF_IN_SMALL_BLOCK (memory, alloc)
  {
    MEMORY_BACKUP_OVERHEAD (alloc);
    MEMORY_SET_SIZE_USED (alloc, MEMORY_SIZE (alloc), 0);
    UNDO_SUCCESS;
  }

  IF_IN_LARGE_BLOCK (memory, alloc)
  {
    return undo_memory_delete_block (&memory->large_alloc_list,
				     &memory->large_alloc_list_count,
				     _if_in_block_ix);
  }

  UNDO_ERROR (UNDO_BADPARAM);
}

size_t
undo_memory_size (UNDO_MEMORY * memory, void *alloc)
{
  IF_IN_BLOCK_VAR;

  IF_IN_SMALL_BLOCK (memory, alloc)
  {
    MEMORY_BACKUP_OVERHEAD (alloc);
    return MEMORY_SIZE (alloc) - UNDO_MEMORY_OVERHEAD;
  }

  IF_IN_LARGE_BLOCK (memory, alloc)
  {
    return _if_in_block->size;
  }

  return 0;
}

unsigned
undo_memory_pages_used (UNDO_MEMORY * memory)
{
  unsigned block_ix;
  UNDO_BLOCK *block;
  unsigned pages;

  pages = 0;

  FOREACH_SMALL_BLOCK (memory, block_ix, block)
  {
    pages += BLOCK_PAGES (*block);
  }

  FOREACH_LARGE_BLOCK (memory, block_ix, block)
  {
    pages += BLOCK_PAGES (*block);
  }

  return pages;
}

UNDO_MEMORY_STREAM *
undo_memory_stream (UNDO_MEMORY * memory)
{
  UNDO_MEMORY_STREAM *stream;

  stream = NEW (UNDO_MEMORY_STREAM);
  if (stream == NULL)
    UNDO_ERROR_NULL (UNDO_NOMEM);

  stream->implementation = memory;
  stream->destroy = /*(void * )*/  undo_stream_destroy;
  stream->read = /*(void *)*/ undo_stream_read;

  return (UNDO_MEMORY_STREAM *) stream;
}

int
undo_memory_set (UNDO_MEMORY * memory, UNDO_MEMORY_STREAM * stream)
{
  undo_memory_clear (memory);
  return undo_memory_add_blocks_from_stream (memory, stream);
}

static void *
undo_memory_alloc_small (UNDO_MEMORY * memory, size_t size)
{
  unsigned block_ix;
  UNDO_BLOCK *block;

  FOREACH_SMALL_BLOCK (memory, block_ix, block)
  {
    void *mem;

    mem = undo_memory_alloc_small_block (block, size);
    if (mem != NULL)
      {
	return mem;
      }
  }

  return undo_memory_alloc_new_small_block (memory, size);
}

static void *
undo_memory_alloc_large (UNDO_MEMORY * memory, size_t size)
{
  size_t page_fraction;

  page_fraction = size & (getpagesize () - 1);
  if (page_fraction)
    {
      size += PAGE_REMAINDER (page_fraction);
    }

  return undo_memory_new_block (&memory->large_alloc_list,
				&memory->large_alloc_list_count, size);
}

static void *
undo_memory_alloc_small_block (UNDO_BLOCK * block, size_t size)
{
  void *mem;
  size_t new_size;

  undo_memory_block_coalesce_free (block);

  new_size = size + UNDO_MEMORY_OVERHEAD;
  FOREACH_IN_BLOCK (*block, mem)
  {
    if (!MEMORY_USED (mem) && MEMORY_SIZE (mem) >= new_size)
      {
	if (MEMORY_SIZE (mem) == new_size)
	  {
	    MEMORY_SET_SIZE_USED (mem, new_size, 1);
	  }
	else
	  {
	    size_t old_size;
	    void *next;

	    old_size = MEMORY_SIZE (mem);
	    MEMORY_SET_SIZE_USED (mem, new_size, 1);
	    next = MEMORY_NEXT (mem);
	    MEMORY_SET_SIZE_USED (next, old_size - new_size, 0);
	  }
	return MEMORY_BODY (mem);
      }
  }

  return NULL;
}

static void
undo_memory_block_coalesce_free (UNDO_BLOCK * block)
{
  void *last, *mem;

  last = NULL;
  FOREACH_IN_BLOCK (*block, mem)
  {
    if (last == NULL)
      {
	last = mem;
	continue;
      }

    if (!MEMORY_USED (last) && !MEMORY_USED (mem))
      {
	MEMORY_SET_SIZE_USED (last,
			      MEMORY_SIZE (last) + MEMORY_SIZE (mem), 0);
	continue;
      }

    last = mem;
  }
}

static void *
undo_memory_alloc_new_small_block (UNDO_MEMORY * mem, size_t size)
{
  void *page, *next;

  page = undo_memory_new_block (&mem->small_alloc_list,
				&mem->small_alloc_list_count, getpagesize ());
  if (page == NULL)
    {
      UNDO_ERROR_NULL (UNDO_NOMEM);
    }

  MEMORY_SET_SIZE_USED (page, size + UNDO_MEMORY_OVERHEAD, 1);
  next = MEMORY_NEXT (page);
  MEMORY_SET_SIZE_USED (next, PAGE_REMAINDER (size + UNDO_MEMORY_OVERHEAD),
			0);

  return MEMORY_BODY (page);
}

static void *
undo_memory_new_block (UNDO_BLOCK ** block_list, unsigned *len, size_t size)
{
  void *mem;

  mem = MAP_NEW_ANON (size);
  if (mem == NULL)
    UNDO_ERROR_NULL (UNDO_NOMEM);

  if (undo_memory_new_block_record (block_list, len, mem, size))
    {
      munmap (mem, size);
      return NULL;
    }

  return mem;
}

static int
undo_memory_new_block_record (UNDO_BLOCK ** block_list, unsigned *len,
			      void *mem, size_t size)
{
  UNDO_BLOCK *new_block_list;


  new_block_list = (UNDO_BLOCK *) realloc (*block_list,
					   (*len + 1) * sizeof (UNDO_BLOCK));
  if (new_block_list == NULL)
    {
      UNDO_ERROR (UNDO_NOMEM);
    }
  *block_list = new_block_list;
  (*block_list)[*len].mem = mem;
  (*block_list)[*len].size = size;
  (*len)++;

  UNDO_SUCCESS;
}

static int
undo_memory_delete_block (UNDO_BLOCK ** block_list, unsigned *length,
			  unsigned index)
{
  UNDO_BLOCK *block;
  UNDO_BLOCK *new_block_list;

  block = &(*block_list)[index];
  munmap (block->mem, block->size);

  if (*length == 1)
    {
      free (*block_list);
      new_block_list = NULL;
    }
  else
    {
      memcpy (&(*block_list)[index], &(*block_list)[index + 1],
	      sizeof (UNDO_BLOCK) * (*length - 1 - index));
      new_block_list = realloc (*block_list,
				sizeof (UNDO_BLOCK) * (*length - 1));
      if (new_block_list == NULL)
	{
	  (*length)--;
	  UNDO_SUCCESS;
	}
    }
  *block_list = new_block_list;
  (*length)--;

  UNDO_SUCCESS;
}


static void
undo_stream_destroy (UNDO_MEMORY_STREAM * stream)
{
  free (stream);
}

static size_t
undo_stream_read (UNDO_MEMORY_STREAM * stream, size_t offset,
		  void *mem, size_t size)
{
  unsigned block_ix;
  UNDO_BLOCK *block;
  UNDO_MEMORY *memory;
  size_t ret;
  size_t pos;
  unsigned flags;

  memory = (UNDO_MEMORY *) stream->implementation;

  ret = 0;
  pos = 0;
  flags = 0;
  FOREACH_SMALL_BLOCK (memory, block_ix, block)
  {
    ret += undo_stream_block_write (offset, mem, size, &pos, block, flags);
  }

  flags = BLOCK_FLAG_LARGE;
  FOREACH_LARGE_BLOCK (memory, block_ix, block)
  {
    ret += undo_stream_block_write (offset, mem, size, &pos, block, flags);
  }

  return ret;
}

static size_t
undo_stream_block_write (size_t offset, void *mem, size_t size,
			 size_t * pos, UNDO_BLOCK * block, unsigned flags)
{
  size_t ret;

  ret = 0;
  ret += undo_memory_stream_write (offset, mem, size,
				   pos, &block->mem, sizeof (void *));
  ret += undo_memory_stream_write (offset, mem, size,
				   pos, &block->size, sizeof (size_t));
  ret += undo_memory_stream_write (offset, mem, size,
				   pos, &flags, sizeof (unsigned));
  ret += undo_memory_stream_write (offset, mem, size,
				   pos, block->mem, block->size);

  return ret;
}

static void
undo_memory_clear (UNDO_MEMORY * memory)
{
  unsigned block_ix;
  UNDO_BLOCK *block;

  FOREACH_SMALL_BLOCK (memory, block_ix, block)
  {
    munmap (block->mem, block->size);
  }
  FOREACH_LARGE_BLOCK (memory, block_ix, block)
  {
    munmap (block->mem, block->size);
  }

  memset (memory, 0, sizeof (UNDO_MEMORY));
}

static int
undo_memory_add_blocks_from_stream (UNDO_MEMORY * memory,
				    UNDO_MEMORY_STREAM * stream)
{
  size_t pos, num_read;
  UNDO_MEMORY_STREAM_BLOCK_HEADER header;

  pos = 0;
  do
    {
      num_read = undo_memory_stream_read_header (stream, pos, &header);
      if (num_read != STREAM_SERIALIZED_BLOCK_HEADER_SIZE)
	{
	  UNDO_SUCCESS;
	}
      pos += num_read;

      if (MAP_NEW_ANON_AT (header.addr, header.size) != header.addr)
	{
	  UNDO_ERROR (UNDO_NOMEM);
	}
      undo_memory_block_header_record (memory, &header);

      num_read = stream->read (stream, pos, header.addr, header.size);
      pos += num_read;
    }
  while (1);
}

static void
undo_memory_block_header_record (UNDO_MEMORY * memory,
				 UNDO_MEMORY_STREAM_BLOCK_HEADER * header)
{
  UNDO_BLOCK **undo_block_list;
  unsigned *undo_block_list_count;

  if (header->flags & BLOCK_FLAG_LARGE)
    {
      undo_block_list = &memory->large_alloc_list;
      undo_block_list_count = &memory->large_alloc_list_count;
    }
  else
    {
      undo_block_list = &memory->small_alloc_list;
      undo_block_list_count = &memory->small_alloc_list_count;
    }

  undo_memory_new_block_record (undo_block_list, undo_block_list_count,
				header->addr, header->size);
}

size_t
undo_memory_stream_write (size_t offset, void *mem, size_t size,
			  size_t * pos, void *data, size_t length)
{
  size_t begin_mem, end_mem;
  size_t begin_data;

  if (offset < *pos)
    {
      begin_mem = *pos - offset;
      begin_data = 0;
    }
  else
    {
      begin_mem = 0;
      begin_data = offset - *pos;
    }

  if (offset + size < *pos + length)
    {
      end_mem = size;
    }
  else
    {
      end_mem = size - (offset + size - *pos - length);
    }

  *pos += length;

  if ((signed) end_mem < 0)
    return 0;
  if (end_mem <= begin_mem)
    return 0;

  memcpy (MEMORY_OFFSET (mem, begin_mem),
	  MEMORY_OFFSET (data, begin_data), end_mem - begin_mem);

  return end_mem - begin_mem;
}

size_t
undo_memory_stream_length (UNDO_MEMORY_STREAM * stream)
{
  size_t len, diff;
  char buff[4096];

  len = 0;
  do
    {
      diff = stream->read (stream, len, buff, 4096);
      len += diff;
    }
  while (diff != 0);

  return len;
}


size_t
undo_memory_stream_read_header (UNDO_MEMORY_STREAM * stream,
				size_t offset,
				UNDO_MEMORY_STREAM_BLOCK_HEADER * header)
{
  size_t num_read;

  num_read = 0;
  num_read += stream->read (stream, offset + num_read,
			    &header->addr, sizeof (void *));
  num_read += stream->read (stream, offset + num_read,
			    &header->size, sizeof (size_t));
  num_read += stream->read (stream, offset + num_read,
			    &header->flags, sizeof (unsigned));

  return num_read;
}




static void undo_history_stream_destroy (UNDO_MEMORY_STREAM * stream);
static size_t undo_history_stream_read (UNDO_MEMORY_STREAM * stream,
					size_t offset,
					void *mem, size_t size);
static int undo_history_add_item (UNDO_HISTORY * history,
				  void *mem, size_t size);
static UNDO_MEMORY_STREAM *undo_history_stream (UNDO_HISTORY * history);
static void undo_history_shrink (UNDO_HISTORY * history, size_t size);
static void undo_history_strip (UNDO_HISTORY * history, int count);

UNDO_HISTORY *
undo_history_new (void)
{
  UNDO_HISTORY *history;

  history = NEW (UNDO_HISTORY);
  if (history == NULL)
    UNDO_ERROR_NULL (UNDO_NOMEM);

  return history;
}

int
undo_history_destroy (UNDO_HISTORY * history)
{
  if (history == NULL)
    UNDO_ERROR (UNDO_BADPARAM);

  free (history);

  UNDO_SUCCESS;
}

int
undo_history_record (UNDO_HISTORY * history, UNDO_MEMORY_STREAM * stream)
{
  void *mem;
  size_t size;
  int ret;

  if (history->memory_limit == 0)
    UNDO_ERROR (UNDO_NOLIMIT);

  size = undo_memory_stream_length (stream);
  if (size > history->memory_limit)
    {
      undo_history_shrink (history, 0);
      UNDO_SUCCESS;
    }
  undo_history_shrink (history, history->memory_limit - size);

  mem = (void *) malloc (size);
  if (mem == NULL)
    UNDO_ERROR (UNDO_NOMEM);
  stream->read (stream, 0, mem, size);

  ret = undo_history_add_item (history, mem, size);
  if (ret)
    {
      free (mem);
    }

  return ret;
}

UNDO_MEMORY_STREAM *
undo_history_undo (UNDO_HISTORY * history)
{
  UNDO_MEMORY_STREAM *stream;

  if (history->ix == 0)
    UNDO_ERROR_NULL (UNDO_NODO);

  stream = undo_history_stream (history);

  if (stream)
    history->ix--;

  return stream;
}

UNDO_MEMORY_STREAM *
undo_history_redo (UNDO_HISTORY * history)
{
  UNDO_MEMORY_STREAM *stream;

  if (history->ix >= history->length - 1)
    UNDO_ERROR_NULL (UNDO_NODO);

  stream = undo_history_stream (history);

  if (stream)
    history->ix++;

  return stream;
}

unsigned
undo_history_undo_count (UNDO_HISTORY * history)
{
  if (history->length)
    return history->ix;

  return 0;
}

unsigned
undo_history_redo_count (UNDO_HISTORY * history)
{
  if (history->length)
    return history->length - 1 - history->ix;

  return 0;
}

void
undo_history_set_memory_limit (UNDO_HISTORY * history, size_t limit)
{
  history->memory_limit = limit;
}

size_t
undo_history_memory_usage (UNDO_HISTORY * history)
{
  int ix;
  size_t total;

  total = 0;
  for (ix = 0; ix < (int)history->length; ix++)
    {
      total += history->item[ix].size;
    }

  return total;
}

static void
undo_history_stream_destroy (UNDO_MEMORY_STREAM * stream)
{
  free (stream);
}

static size_t
undo_history_stream_read (UNDO_MEMORY_STREAM * stream,
			  size_t offset, void *mem, size_t size)
{
  UNDO_HISTORY *history;
  size_t pos;
  void *history_mem;
  size_t history_size;

  history = (UNDO_HISTORY *) stream->implementation;

  pos = 0;
  history_mem = history->item[history->ix].mem;
  history_size = history->item[history->ix].size;
  return undo_memory_stream_write (offset, mem, size,
				   &pos, history_mem, history_size);
}

static int
undo_history_add_item (UNDO_HISTORY * history, void *mem, size_t size)
{
  UNDO_HISTORY_ITEM *new_item;

  new_item = realloc (history->item,
		      sizeof (UNDO_HISTORY_ITEM) * (history->length + 1));
  if (new_item == NULL)
    {
      UNDO_ERROR (UNDO_NOMEM);
    }
  history->item = new_item;

  history->length++;

  history->ix = history->length - 1;
  history->item[history->ix].mem = mem;
  history->item[history->ix].size = size;

  UNDO_SUCCESS;
}

static UNDO_MEMORY_STREAM *
undo_history_stream (UNDO_HISTORY * history)
{
  UNDO_MEMORY_STREAM *stream;

  stream = NEW (UNDO_MEMORY_STREAM);
  if (stream == NULL)
    UNDO_ERROR_NULL (UNDO_NOMEM);

  stream->implementation = history;
  stream->destroy = undo_history_stream_destroy;
  stream->read = undo_history_stream_read;

  return stream;
}

static void
undo_history_shrink (UNDO_HISTORY * history, size_t size)
{
  int first_saved;
  int ix;
  size_t total;

  total = 0;
  first_saved = 0;
  for (ix = 0; ix < (int)history->length; ix++)
    {
      total += history->item[ix].size;
      while (total > size)
	{
	  total -= history->item[first_saved].size;
	  first_saved++;
	}
    }

  undo_history_strip (history, first_saved);
}

static void
undo_history_strip (UNDO_HISTORY * history, int count)
{
  int ix;

  if (count == 0)
    return;

  for (ix = 0; ix < count; ix++)
    {
      free (history->item[ix].mem);
    }

  memmove (history->item, history->item + count,
	   sizeof (UNDO_HISTORY_ITEM) * (history->length - count));
  history->length -= count;
  history->ix = history->length - 1;
}


static int undo_last_error = UNDO_NOERROR;

void
undo_set_error (int err)
{
  undo_last_error = err;
}

int
undo_get_errcode (void)
{
  int ret;

  ret = undo_last_error;
  undo_last_error = UNDO_NOERROR;
  return ret;
}

char *
undo_strerror (int errcode)
{
  char *err_string[] = {
    "No error",
    "Bad parameter passed to undo function",
    "Out of memory",
    "No active undo session",
    "Nothing to undo/redo",
    "No undoable memory limit set"
  };

  if (errcode < 0)
    return NULL;

  if (errcode >= (int) (sizeof (err_string) / sizeof (char *)))
    return NULL;

  return err_string[errcode];
}
#endif
