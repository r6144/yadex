/*
 *	wads2.cc
 *	Wads functions that are not needed during editing.
 *	AYM 1998-08-09
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Rapha�l Quinet and Brendon Wyber.

The rest of Yadex is Copyright � 1997-2003 Andr� Majorel and others.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/


#include "yadex.h"
#include "game.h"	/* yg_picture_format */
#include "serialnum.h"
#include "wadfile.h"
#include "wadlist.h"
#include "wads.h"
#include "wads2.h"


static char *locate_pwad (const char *filename);
static int level_name_order (const void *p1, const void *p2);


/*
 *	OpenMainWad - open the iwad
 *
 *	Open the main wad file, read in its directory and create
 *	the master directory.
 *
 *	Return 0 on success, non-zero on failure.
 */
int OpenMainWad (const char *filename)
{
MDirPtr lastp, newp;
long n;
Wad_file *wf;

/* open the wad file */
printf ("Loading iwad: %s...\n", filename);
wf = BasicWadOpen (filename, yg_picture_format);
if (wf == 0)
   return 1;
if (strncmp (wf->type, "IWAD", 4))
   warn ("%.128s: is a pwad, not an iwad. Will use it anyway.\n", filename);

/* create the master directory */
lastp = NULL;
for (n = 0; n < wf->dirsize; n++)
   {
   newp = (MDirPtr) GetMemory (sizeof (struct MasterDirectory));
   newp->next = NULL;
   newp->wadfile = wf;
   memcpy (&(newp->dir), &(wf->directory[n]), sizeof (struct Directory));
   if (MasterDir)
      lastp->next = newp;
   else
      MasterDir = newp;
   lastp = newp;
   }
master_dir_serial.bump ();

/* check if registered version */
if (FindMasterDir (MasterDir, "E2M1") == NULL
 && FindMasterDir (MasterDir, "MAP01") == NULL
 && FindMasterDir (MasterDir, "MAP33") == NULL
 && strcmp (Game, "doom02")
 && strcmp (Game, "doom04")
 && strcmp (Game, "doom05")
 && strcmp (Game, "doompr"))
   {
   printf ("   *-----------------------------------------------------*\n");
   printf ("   | Warning: this is the shareware version of the game. |\n");
   printf ("   |     You won't be allowed to save your changes.      |\n");
   printf ("   |       PLEASE REGISTER YOUR COPY OF THE GAME.        |\n");
   printf ("   *-----------------------------------------------------*\n");
   Registered = false; // If you remove this, bad things will happen to you...
   }
else
   Registered = true;
return 0;
}


/*
 *	OpenPatchWad - add a pwad
 *
 *	Open a patch wad file, read in its directory and alter
 *	the master directory.
 *
 *	Return 0 on success, non-zero on failure.
 */
int OpenPatchWad (const char *filename)
{
Wad_file * wad;
MDirPtr mdir = 0;
long n;
char entryname[WAD_NAME + 1];
const char *entry_type = 0;
char *real_name;
int nitems = 0;		// Number of items in group of flats/patches/sprites

// Look for the file and ignore it if it doesn't exist
real_name = locate_pwad (filename);
if (real_name == NULL)
   {
   warn ("%.128s: not found.\n", filename);
   return 1;
   }

/* open the wad file */
printf ("Loading pwad: %s...\n", real_name);
// By default, assume pwads use the normal picture format.
wad = BasicWadOpen (real_name, YGPF_NORMAL);
FreeMemory (real_name);
if (! wad)
   return 1;
if (strncmp (wad->type, "PWAD", 4))
   warn ("%.128s: is an iwad, not a pwad. Will use it anyway.\n", filename);

/* alter the master directory */

/* AYM: now, while the directory is scanned, a state variable is
   updated. its values are :
   0    no special state
   1-11 reading level lumps */
/* AYM 1998-11-15: FIXME: to be on the safe side, should consider
   FF_END to end a group of flats if the following entry is neither
   F_END nor F?_START. */

int state = 0;
int replaces = 0;
int state_prev;
int replaces_prev;
int names = 0;		// Number of names already printed on current line
const char *entry_type_prev;
typedef char level_list_item_t[WAD_NAME];
level_list_item_t *level_list = 0;
size_t nlevels = 0;
for (n = 0; n < wad->dirsize; n++)
   {
   strncpy (entryname, wad->directory[n].name, WAD_NAME);
   entryname[WAD_NAME] = '\0';
   state_prev = state;
   replaces_prev = replaces;
   entry_type_prev = entry_type;
   if (state == 0)
      {
      if (! strcmp (entryname, "F_START")
       || ! strcmp (entryname, "P_START")
       || ! strcmp (entryname, "S_START"))
	 {
	 entry_type = "label";
	 replaces   = 0;
	 }
      // DeuTex puts flats between FF_START and FF_END/F_END.
      // All lumps between those markers are assumed
      // to be flats.
      else if (! strncmp (entryname, "FF_START", WAD_NAME))
	 {
	 state      = 'f';
	 entry_type = "group of flats";
	 replaces   = 0;
         nitems     = 0;
	 }
      // DeuTex puts patches between PP_START and PP_END.
      // All lumps between those markers are assumed
      // to be patches.
      else if (! strncmp (entryname, "PP_START", WAD_NAME))
	 {
	 state      = 'p';
	 entry_type = "group of patches";
	 replaces   = 0;
         nitems     = 0;
	 }
      // DeuTex puts patches between SS_START and SS_END/S_END.
      // All lumps between those markers are assumed
      // to be sprites.
      else if (! strncmp (entryname, "SS_START", WAD_NAME))
	 {
	 state      = 's';
	 entry_type = "group of sprites";
	 replaces   = 0;
         nitems     = 0;
	 }
      else
	 {
	 mdir = FindMasterDir (MasterDir, entryname);
	 replaces = mdir != NULL;
	 /* if it is a level, do the same thing for the next 10 entries too */
	 if (levelname2levelno (entryname))
	    {
	    state = 11;
	    entry_type = "level";
	    // Add to list of level names
	    {
	    level_list_item_t *new_list;
	    new_list = (level_list_item_t *)
	       realloc (level_list, (nlevels + 1) * sizeof *level_list);
	    if (new_list != 0)
	       {
	       level_list = new_list;
	       strncpy (level_list[nlevels], entryname, sizeof *level_list);
	       nlevels++;
	       }
	    }
	    }
	 else
	    entry_type = "entry";
	 }
      if (n == 0
	  || state_prev      != state
	  || replaces_prev   != replaces
	  || entry_type_prev != entry_type)
         {
	 if (n > 0)
	    verbmsg ("\n");
	 names = 0;
	 verbmsg ("  %s %s", replaces ? "Updating" : "Adding new", entry_type);
         }
      if (names >= 6)
         {
	 verbmsg ("\n  %-*s %-*s",
	     strlen (replaces ? "Updating" : "Adding new"), "",
	     strlen (entry_type), "");
	 names = 0;
         }
      verbmsg  (" %-*s", WAD_NAME, entryname);
      names++;
      if ((*entry_type == 'm' || *entry_type == 'l') && wad->directory[n].size)
	 verbmsg (" warning: non-zero length (%ld)", wad->directory[n].size);
      }
   // Either F_END or FF_END mark the end of a
   // DeuTex-generated group of flats.
   else if (state == 'f')
      {
      if (! strncmp (entryname, "F_END", WAD_NAME)
	  || ! strncmp (entryname, "FF_END", WAD_NAME))
         {
	 state = 0;
         verbmsg ("/%.*s (%d flats)", WAD_NAME, entryname, nitems);
         }
      // Of course, F?_START and F?_END don't count
      // toward the number of flats in the group.
      else if (! (*entryname == 'F'
                  && (! strncmp (entryname + 2, "_START", 6)
                      || ! strcmp (entryname + 2, "_END"))))
         nitems++;
      }
   // PP_END marks the end of a DeuTex-generated group of patches.
   else if (state == 'p')
      {
      if (! strncmp (entryname, "PP_END", WAD_NAME))
         {
         state = 0;
         verbmsg ("/PP_END (%d patches)", nitems);
         }
      // Of course, P?_START and P?_END don't count
      // toward the number of flats in the group.
      else if (! (*entryname == 'P'
                  && (! strncmp (entryname + 2, "_START", 6)
                      || ! strcmp (entryname + 2, "_END"))))
         nitems++;
      }
   // Either S_END or SS_END mark the end of a
   // DeuTex-generated group of sprites.
   else if (state == 's')
      {
      if (! strncmp (entryname, "S_END", WAD_NAME)
	  || ! strncmp (entryname, "SS_END", WAD_NAME))
         {
	 state = 0;
         verbmsg ("/%.*s (%d sprites)", WAD_NAME, entryname, nitems);
         }
      // Of course, S?_START and S?_END don't count
      // toward the number of sprites in the group.
      else if (! (*entryname == 'S'
                  && (! strncmp (entryname + 2, "_START", 6)
                      || ! strcmp (entryname + 2, "_END"))))
         nitems++;
      }
     
   /* if this entry is not in the master directory, then add it */
   if (!replaces)
      {
      mdir = MasterDir;
      while (mdir->next)
	 mdir = mdir->next;
      mdir->next = (MDirPtr) GetMemory (sizeof (struct MasterDirectory));
      mdir = mdir->next;
      mdir->next = NULL;
      }
   /* else, simply replace it */
   mdir->wadfile = wad;
   memcpy (&(mdir->dir), &(wad->directory[n]), sizeof (struct Directory));
   mdir = mdir->next;

   if (state > 0 && state <= 11)
      state--;
   }
verbmsg ("\n");
master_dir_serial.bump ();

// Print list of levels found in this pwad
if (level_list != 0)
   {
   printf ("  Levels: ");
   qsort (level_list, nlevels, sizeof *level_list, level_name_order);
   for (size_t n = 0; n < nlevels; n++)
      {
      int prev = n > 0           ? levelname2rank (level_list[n - 1]) : INT_MIN;
      int cur  =                   levelname2rank (level_list[n    ]);
      int next = n + 1 < nlevels ? levelname2rank (level_list[n + 1]) : INT_MAX;
      if (cur != prev + 1 || cur != next - 1)
         {
         if (cur == prev + 1)
	    putchar ('-');
	 else if (n > 0)
	    putchar (' ');
         printf ("%.*s", (int) sizeof *level_list, level_list[n]);
         }
      }
   putchar ('\n');
   free (level_list);
   }
return 0;
}


/*
 *	level_name_order - -cmp-style comparison of two level names
 */
static int level_name_order (const void *p1, const void *p2)
{
return levelname2rank ((const char *) p1)
     - levelname2rank ((const char *) p2);
}


/*
 *	CloseWadFiles - close all wads
 *
 *	Close all the wad, deallocating the wad file structures.
 */
void CloseWadFiles ()
{
MDirPtr curd, nextd;

// Close the wad files
Wad_file *wf;
wad_list.rewind ();
while (wad_list.get (wf))
   wad_list.del ();

// Delete the master directory
curd = MasterDir;
MasterDir = NULL;
while (curd)
   {
   nextd = curd->next;
   FreeMemory (curd);
   curd = nextd;
   }
master_dir_serial.bump ();
}


/*
 *	CloseUnusedWadFiles - forget unused patch wad files
 */
void CloseUnusedWadFiles ()
{
Wad_file *wf;
wad_list.rewind ();
while (wad_list.get (wf))
   {
   // Check if the wad file is used by a directory entry
   MDirPtr mdir = MasterDir;
   while (mdir && mdir->wadfile != wf)
      mdir = mdir->next;
   if (mdir == 0)
      wad_list.del ();
   }
}


/*
 *	BasicWadOpen - open a wad
 *
 *	Basic opening of wad file and creation of node in Wad
 *	linked list.
 *
 *	Return a null pointer on error.
 */
Wad_file *BasicWadOpen (const char *filename, ygpf_t pic_format)
{
bool fail = false;

/* If this wad is already open, close it first (it's not always
   possible to open the same file twice). Also position the
   wad_list pointer on the old wad (or at the end of the list if
   this is a new wad) so that the reopening a wad doesn't change
   it's relative position in the list.
   
   FIXME if reopening fails, we're left in the cold. I'm not
   sure how to avoid that, though. */
{
   Wad_file *dummy;
   wad_list.rewind ();
   while (wad_list.get (dummy))
      if (strcmp(filename, dummy->filename) == 0)
	 {
	 wad_list.del ();
	 break;
	 }
}

// Create a new Wad_file
Wad_file *wf = new Wad_file;
wf->pic_format_ = pic_format;
wf->directory   = 0;
wf->filename    = (char *) GetMemory (strlen (filename) + 1);
strcpy (wf->filename, filename);

// Open the wad and read its header.
wf->fp = fopen (filename, "rb");
if (wf->fp == 0)
   {
   printf ("%.128s: %s\n", filename, strerror (errno));
   fail = true;
   goto byebye;
   }
{
bool e = file_read_bytes (wf->fp, wf->type, 4);
e     |= file_read_i32   (wf->fp, &wf->dirsize);
e     |= file_read_i32   (wf->fp, &wf->dirstart);
if (e || memcmp (wf->type, "IWAD", 4) != 0 && memcmp (wf->type, "PWAD", 4) != 0)
   {
   printf ("%.128s: not a wad (bad header)\n", filename);
   fail = true;
   goto byebye;
   }
}
verbmsg ("  Type %.4s, directory has %ld entries at offset %08lXh\n",
   wf->type, (long) wf->dirsize, (long) wf->dirstart);

// Load the directory of the wad
wf->directory = (DirPtr) GetMemory ((long) sizeof (struct Directory)
   * wf->dirsize);
if (fseek (wf->fp, wf->dirstart, SEEK_SET) != 0)
   {
   printf ("%.128s: can't seek to directory at %08lXh\n",
      filename, wf->dirstart);
   fail = true;
   goto byebye;
   }
for (i32 n = 0; n < wf->dirsize; n++)
   {
   bool e  = file_read_i32   (wf->fp, &wf->directory[n].start);
   e      |= file_read_i32   (wf->fp, &wf->directory[n].size);
   e      |= file_read_bytes (wf->fp, wf->directory[n].name, WAD_NAME);
   if (e)
      {
      printf ("%.128s: read error on directory entry %ld\n", filename, (long)n);
      fail = true;
      goto byebye;
      }
   }

// Insert the new wad in the list
wad_list.insert (wf);

byebye:
if (fail)
  {
  delete wf;
  return 0;
  }
return wf;
}


/*
 *	ListMasterDirectory - list the master directory
 */
void ListMasterDirectory (FILE *file)
{
char dataname[WAD_NAME + 1];
MDirPtr dir;
char key;
int lines = 3;

dataname[WAD_NAME] = '\0';
fprintf (file, "The Master Directory\n");
fprintf (file, "====================\n\n");
fprintf (file, "NAME____  FILE______________________________________________"
   "  SIZE__  START____\n");
for (dir = MasterDir; dir; dir = dir->next)
   {
   strncpy (dataname, dir->dir.name, WAD_NAME);
   fprintf (file, "%-*s  %-50s  %6ld  x%08lx\n",
    WAD_NAME, dataname, dir->wadfile->pathname (),
    dir->dir.size, dir->dir.start);
   if (file == stdout && lines++ > screen_lines - 4)
      {
      lines = 0;
      printf ("['Q' followed by Return to abort, Return only to continue]");
      key = getchar ();
      printf ("\r%57s\r", "");
      if (key == 'Q' || key == 'q')
	 {
         getchar ();  // Read the '\n'
         break;
         }
      }
   }
}


/*
 *	ListFileDirectory - list the directory of a wad
 */
void ListFileDirectory (FILE *file, const Wad_file *wad)
{
char dataname[WAD_NAME + 1];
char key;
int lines = 5;
long n;

dataname[WAD_NAME] = '\0';
fprintf (file, "Wad File Directory\n");
fprintf (file, "==================\n\n");
fprintf (file, "Wad File: %s\n\n", wad->pathname ());
fprintf (file, "NAME____  SIZE__  START____  END______\n");
for (n = 0; n < wad->dirsize; n++)
   {
   strncpy (dataname, wad->directory[n].name, WAD_NAME);
   fprintf (file, "%-*s  %6ld  x%08lx  x%08lx\n",
     WAD_NAME, dataname,
     wad->directory[n].size,
     wad->directory[n].start,
     wad->directory[n].size + wad->directory[n].start - 1);
   if (file == stdout && lines++ > screen_lines - 4)
      {
      lines = 0;
      printf ("['Q' followed by Return to abort, Return only to continue]");
      key = getchar ();
      printf ("\r%57s\r", "");
      if (key == 'Q' || key == 'q')
         {
         getchar ();  // Read the '\n'
	 break;
         }
      }
   }
}


/*
 *	BuildNewMainWad - build a new iwad (or pwad)
 *
 *	Build a new wad file from master directory.
 */
void BuildNewMainWad (const char *filename, bool patchonly)
{
FILE *file;
long counter = 12;
MDirPtr cur;
long size;
long dirstart;
long dirnum;

/* open the file and store signatures */
if (patchonly)
   printf ("Building a compound Patch Wad file \"%s\".\n", filename);
else
   printf ("Building a new Main Wad file \"%s\" (size approx 10 MB)\n",
      filename);
if (FindMasterDir (MasterDir, "E2M4") == NULL
 && FindMasterDir (MasterDir, "MAP01") == NULL
 && FindMasterDir (MasterDir, "MAP33") == NULL
 && strcmp (Game, "doom02")
 && strcmp (Game, "doom04")
 && strcmp (Game, "doom05")
 && strcmp (Game, "doompr"))
   fatal_error ("You were warned: you are not allowed to do this.");
if ((file = fopen (filename, "wb")) == NULL)
   fatal_error ("unable to open file \"%s\"", filename);
if (patchonly)
   WriteBytes (file, "PWAD", 4);
else
   WriteBytes (file, "IWAD", 4);
file_write_i32 (file, 0xdeadbeef);      /* put true value in later */
file_write_i32 (file, 0xdeadbeef);      /* put true value in later */

/* output the directory data chunks */
const Wad_file *iwad = 0;	// FIXME unreliable way of knowing the iwad
wad_list.rewind ();		// got to look into this
wad_list.get (iwad);
for (cur = MasterDir; cur; cur = cur->next)
   {
   if (patchonly && cur->wadfile == iwad)
      continue;
   size = cur->dir.size;
   counter += size;
   cur->wadfile->seek (cur->dir.start);
   if (cur->wadfile->error ())
      ;  // FIXME
   if (copy_bytes (file, cur->wadfile->fp, size) != 0)
      ;  // FIXME
   printf ("Size: %luK\r", counter / 1024);
   }

/* output the directory */
dirstart = counter;
counter = 12;
dirnum = 0;
for (cur = MasterDir; cur; cur = cur->next)
   {
   if (patchonly && cur->wadfile == iwad)
      continue;
   if (dirnum % 100 == 0)
      printf ("Outputting directory %04ld...\r", dirnum);
   if (cur->dir.start)
      file_write_i32 (file, counter);
   else
      file_write_i32 (file, 0);
   file_write_i32 (file, cur->dir.size);
   file_write_name (file, cur->dir.name);
   counter += cur->dir.size;
   dirnum++;
   }

/* fix up the number of entries and directory start information */
if (fseek (file, 4L, 0))
   fatal_error ("error writing to file");
file_write_i32 (file, dirnum);
file_write_i32 (file, dirstart);

/* close the file */
printf ("                            \r");
fclose (file);
}


/*
 *	DumpDirectoryEntry - hexadecimal dump of a lump
 *
 *	Dump a directory entry in hex
 */
void DumpDirectoryEntry (FILE *file, const char *entryname)
{
char dataname[WAD_NAME + 1];
char key;
int lines = 5;
long n = 0;
unsigned char buf[16];
const int bytes_per_line = 16;

for (MDirPtr entry = MasterDir; entry != 0; entry = entry->next)
   {
   if (y_strnicmp (entry->dir.name, entryname, WAD_NAME) != 0)
      continue;
   strncpy (dataname, entry->dir.name, WAD_NAME);
   dataname[WAD_NAME] = '\0';
   fprintf (file, "Contents of entry %s (size = %ld bytes):\n",
      dataname, entry->dir.size);
   const Wad_file *wf = entry->wadfile;
   wf->seek (entry->dir.start);
   for (n = 0; n < entry->dir.size;)
      {
      int i;
      fprintf (file, "%04lX: ", n);

      // Nb of bytes to read for this line
      long bytes_to_read = entry->dir.size - n;
      if (bytes_to_read > bytes_per_line)
	 bytes_to_read = bytes_per_line;
      long nbytes = wf->read_vbytes (buf, bytes_to_read);
      if (wf->error ())
	 break;
      n += nbytes;

      for (i = 0; i < nbytes; i++)
	 fprintf (file, " %02X", buf[i]);
      for (; i < bytes_per_line; i++)
	 fputs ("   ", file);
      fprintf (file, "   ");

      for (i = 0; i < nbytes; i++)
	 {
	 if (buf[i] >= 0x20
	    && buf[i] != 0x7f
	    && ! (buf[i] >= 0x80 && buf[i] <= 0xa0)  // ISO 8859-1
	    )
	    putc (buf[i], file);
	 else
	    putc ('.', file);
	 }
      putc ('\n', file);

      if (file == stdout && lines++ > screen_lines - 4)
	 {
	 lines = 0;
	 printf ("[%d%% - Q + Return to abort,"
	     " S + Return to skip this entry,"
	     " Return to continue]", (int) (n * 100 / entry->dir.size));
	 key = getchar ();
	 printf ("\r%68s\r", "");
	 if (key == 'S' || key == 's')
	    {
	    getchar ();  // Read the '\n'
	    break;
	    }
	 if (key == 'Q' || key == 'q')
	    {
	    getchar ();  // Read the '\n'
	    return;
	    }
	 }
      }
   }
if (! n)
   {
   printf ("Entry not in master directory.\n");
   return;
   }
}



/*
 *	SaveDirectoryEntry - write the contents of a lump to a new pwad
 *
 *	Save a directory entry to disk
 */
void SaveDirectoryEntry (FILE *file, const char *entryname)
{
MDirPtr entry;

for (entry = MasterDir; entry; entry = entry->next)
   if (!y_strnicmp (entry->dir.name, entryname, WAD_NAME))
      break;
if (entry)
   {
   // Write the header
   WriteBytes (file, "PWAD", 4);	// Type = PWAD
   file_write_i32 (file, 1);		// 1 entry in the directory
   file_write_i32 (file, 12);		// The directory starts at offset 12

   // Write the directory
   file_write_i32 (file, 28);		// First entry starts at offset 28
   file_write_i32 (file, entry->dir.size);	// Size of first entry
   file_write_name (file, entry->dir.name);	// Name of first entry

   // Write the lump data
   entry->wadfile->seek (entry->dir.start);
   if (entry->wadfile->error ())
      {
      err ("%s: seek error", entryname);
      return;
      }
   int r = copy_bytes (file, entry->wadfile->fp, entry->dir.size);
   if (r != 0)
      {
      if (r == 1)
	err ("%s: error reading from source wad", entryname);
      else if (r == 2)
	err ("%s: error writing to destination wad", entryname);
      else
	nf_bug ("%s: copy_bytes() returned %d", entryname, r);
      return;
      }
   }
else
   {
   printf ("Entry not in master directory.\n");
   return;
   }
}


/*
 *	SaveEntryToRawFile - write the contents of a lump to a new file
 *
 *	Save a directory entry to disk, without a pwad header
 */
void SaveEntryToRawFile (FILE *file, const char *entryname)
{
MDirPtr entry;

for (entry = MasterDir; entry; entry = entry->next)
   if (!y_strnicmp (entry->dir.name, entryname, WAD_NAME))
      break;
if (entry)
   {
   verbmsg ("Writing %ld bytes starting from offset %lX...\n",
      (long) entry->dir.size, (unsigned long) entry->dir.start);
   entry->wadfile->seek (entry->dir.start);
   if (entry->wadfile->error ())
      {
      err ("%s: seek error", entryname);
      return;
      }
   int r = copy_bytes (file, entry->wadfile->fp, entry->dir.size);
   if (r != 0)
      {
      if (r == 1)
	err ("%s: error reading from source wad", entryname);
      else if (r == 2)
	err ("%s: error writing to destination file", entryname);
      else
	nf_bug ("%s: copy_bytes() returned %d", entryname, r);
      return;
      }
   }
else
   {
   printf ("[Entry not in master directory]\n");
   return;
   }
}


/*
 *	SaveEntryFromRawFile - encapsulate a raw file in a pwad
 *
 *	Encapsulate a raw file in a pwad file
 */
void SaveEntryFromRawFile (FILE *file, FILE *raw, const char *entryname)
{
long    size;
char    name8[WAD_NAME];

// Write the header
WriteBytes (file, "PWAD", 4);		// Type = PWAD
file_write_i32 (file, 1);		// 1 entry in the directory
file_write_i32 (file, 12);		// The directory starts at offset 12

// Write the directory
file_write_i32 (file, 28);		// First entry starts at offset 28

if (fseek (raw, 0L, SEEK_END) != 0)
   fatal_error ("error reading from raw file");
size = ftell (raw);
if (size < 0)
   fatal_error ("error reading from raw file");
if (fseek (raw, 0L, SEEK_SET) != 0)
   fatal_error ("error reading from raw file");
file_write_i32 (file, size);		// Size of first entry

memset (name8, '\0', WAD_NAME);
strncpy (name8, entryname, WAD_NAME);
file_write_name (file, name8);		// Name of first entry

// Write the lump data
int r = copy_bytes (file, raw, size);
if (r != 0)
   {
   if (r == 1)
     err ("%s: error reading from source file", entryname);
   else if (r == 2)
     err ("%s: error writing to destination wad", entryname);
   else
     nf_bug ("%s: copy_bytes() returned %d", entryname, r);
   return;
   }
}


/*
 *	locate_pwad
 *	Look for a PWAD in the standard directories
 *	and returns its name in a GetMemory'd buffer
 *	(or NULL if not found). It's up to the caller
 *	to free the buffer after use.
 */


/* Directories that are searched for PWADs */
static const char *standard_directories[] =
   {
   "",
   "~/",                            // "~" means "the user's home directory"
   "/usr/local/share/games/%s/",    // %s is replaced by <Game>
   "/usr/share/games/%s/",          // %s is replaced by <Game>
   "/usr/local/share/games/wads/",
   "/usr/share/games/wads/",
   0
   };


static char *locate_pwad (const char *filename)
{
al_fext_t ext;
const char **dirname;
char *real_basename;
char *real_name;

// Get the extension in <ext>
al_fana (filename, NULL, NULL, NULL, ext);

// If it's an absolute name, stop there.
if (is_absolute (filename))
   {
   real_name = (char *) GetMemory (strlen (filename) + 1 + (*ext ? 0 : 4));
   strcpy (real_name, filename);
   if (! *ext)
      strcat (real_name, ".wad");
   bool r = file_exists (real_name);
   if (! r)
      {
      FreeMemory (real_name);
      return 0;
      }
   return real_name;
   }

// It's a relative name. If no extension given, append ".wad"
real_basename = (char *) GetMemory (strlen (filename) + 1 + (*ext ? 0 : 4));
strcpy (real_basename, filename);
if (! *ext)
   strcat (real_basename, ".wad");

// Then search for a file of that name in the standard directories.
real_name = (char *) GetMemory (Y_FILE_NAME + 1);
for (dirname = standard_directories; *dirname; dirname++)
   {
   if (! strcmp (*dirname, "~/"))
      if (getenv ("HOME"))
         {
	 al_scps (real_name, getenv ("HOME"), Y_FILE_NAME);
         al_sapc (real_name, '/', Y_FILE_NAME);
         }
      else
	 continue;
   else
      y_snprintf (real_name, Y_FILE_NAME + 1, *dirname, Game ? Game : "");
   al_saps (real_name, real_basename, Y_FILE_NAME);
   verbmsg ("  Trying \"%s\"... ", real_name);
   if (file_exists (real_name))
      {
      verbmsg ("right on !\n");
      FreeMemory (real_basename);
      return real_name;
      }
   verbmsg ("nuts\n");
   }
FreeMemory (real_name);
FreeMemory (real_basename);
return NULL;
}


/* end of file */

