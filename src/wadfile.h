/*
 *	wadfile.h - Wad_file class
 *	AYM 2001-09-18
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Rapha�l Quinet and Brendon Wyber.

The rest of Yadex is Copyright � 1997-2003 Andr� Majorel and others.and others.

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


#ifndef YH_WADFILE  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_WADFILE


/*
 *	Wad_file - wad file open for reading
 *
 *	The Wad_file class is a simple wad file object. It
 *	provides functions to read the usual data formats (byte,
 *	16-bit signed integer, 32-bit signed integer) portably.
 *
 *	Errors are reported not by overloading the return
 *	value but by setting an internal flag that can be read
 *	with the error() function, a bit like stdio's feof() and
 *	ferror(). Calling error() has the side effect of
 *	clearing the error flag and calling clearerr() on the
 *	underlying FILE. This is the only way to reset the error
 *	flag. Thus, you don't have to check the error status
 *	after every operation. You can call seek() and read_*()
 *	any number of times before calling error() : the class
 *	guarantees that if any one of the operations failed,
 *	error() returns true.
 *
 *	Yadex has a policy of reporting I/O errors, mentioning
 *	the name of the file and the offset at which the error
 *	occurred. This policy is enforced by the Wad_file class.
 *	To avoid flooding, no message is printed if the error
 *	flag was already set before the error occurred. This
 *	gives you some control on how verbose the error
 *	reporting is ; if you want a message for every error,
 *	all you have to do is to check error() after every
 *	operation. It's probably best to report only the first
 *	error, though.
 *
 *	If for some reason you need to disable the error
 *	messages, add a bool quiet parameter with a default
 *	value of false to the functions that can fail.
 *
 *	It's probably a good idea to call error() after every
 *	seek(), before attempting to read. Although seek()
 *	rarely fails under Unix, MS-DOS refuses to seek() past
 *	EOF, if I recall correctly.
 *
 *	The interface is somewhat crufty. Some functions take a
 *	pointer, others take a reference. Some take an optional
 *	count parameter, some take a mandatory count parameter,
 *	and some have no count parameter at all. Needs cleanup.
 *
 *	This class has many public data members, and no proper
 *	constructor. That's because it evolved from a C struct.
 *
 *	Another problem is the lack of errno detail. That is
 *	partly stdio's fault.
 *
 *	const: for the moment, const means that you can read
 *	from but not change the attributes, close, reopen, etc.
 */
class Wad_file
{
  /* Ugly but better than making the data members public. FIXME
     Many of these are friend just because they use fp for
     reading. */
  friend char     *GetWadFileName     (const char *);
  friend int       SaveLevelData      (const char *, const char *);
  friend int       OpenMainWad        (const char *);
  friend int       OpenPatchWad       (const char *);
  friend Wad_file *BasicWadOpen       (const char *, ygpf_t);
  friend void      BuildNewMainWad    (const char *, bool);
  friend void      SaveDirectoryEntry (FILE *, const char *);
  friend void      SaveEntryToRawFile (FILE *, const char *);
  friend void      ListFileDirectory  (FILE *, const Wad_file *);
  friend FILE	  *get_decorate       (void);

  public :
    Wad_file () :
      filename (0),
      pic_format_ (YGPF_NORMAL),
      fp (0),
      dirsize (0),
      dirstart (0),
      directory (0),
      error_ (false)
    {
      strcpy (type, "BUG");
      strcpy (where_, "DEADBEEF");
    }
    ~Wad_file ();
    const char *pathname    () const;
    ygpf_t      pic_format  () const;
    bool        error       () const;
    const char *where       () const;
    void        seek        (long offset) const;
    u8          read_u8     () const;
    void        read_u8     (u8& buf) const;
    i16         read_i16    () const;
    void        read_i16    (i16 *buf) const;
    void        read_i32    (i32 *buf, long count = 1) const;
    void        read_bytes  (void *buf, long count) const;
    long        read_vbytes (void *buf, long count) const;
    const char *what        () const;

  private :
    char *filename;			// Name of the wad file
    ygpf_t pic_format_;			// Picture format (usually PF_NORMAL)
    FILE *fp;				// C file stream information
    char type[4];			// Type of wad file ("IWAD" or "PWAD")
    i32  dirsize;			// Entries in directory
    i32  dirstart;			// Offset to start of directory
    DirPtr directory;			// Array of directory information
    mutable bool error_;		// I/O error occur since last error()
    mutable char where_[101];		// Static workspace for where()

    Wad_file (const Wad_file& rhs);		// Deliberately not implemented
    Wad_file& operator= (const Wad_file& rhs);	// Deliberately not implemented
};


/*
 *	Wad_file::pathname - return the pathname of the file
 */
inline const char *Wad_file::pathname () const
{
  return filename;
}


/*
 *	Wad_file::pic_format - return the pic_format of the wad
 */
inline ygpf_t Wad_file::pic_format () const
{
  return pic_format_;
}


/*
 *	Wad_file::error - tell whether any errors occurred
 *
 *	Reset the error indicator and call clearerr() on the
 *	underlying stdio stream. Thus calling Wad_file::error()
 *	again immediately after always returns false. Calling
 *	this function is the only way to clear the error flag of
 *	a Wad_file.
 *
 *	So short that it's a good candidate for inlining.
 *
 *	Return true if an error occurred, false otherwise.
 */
inline bool Wad_file::error () const
{
  if (! error_)
    return false;

  clearerr (fp);
  error_ = false;
  return true;
}


/*
 *	Wad_file::seek - move the file pointer
 *
 *	If an error occurs, set the error flag.
 */
inline void Wad_file::seek (long offset) const
{
  if (fseek (fp, offset, 0) != 0)
  {
    if (! error_)
      err ("%s: can't seek to %lXh", filename, offset);
    error_ = true;
  }
}


/*
 *	Wad_file::read_u8 - read a byte
 *
 *	If an error occurs, set the error flag and the return
 *	value is undefined.
 */
inline u8 Wad_file::read_u8 () const
{
  u8 v = u8 (getc (fp));

  if (feof (fp) || ferror (fp))
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
  }
  return v;
}


/*
 *	Wad_file::read_u8 - read a byte
 *
 *	If an error occurs, set the error flag and the contents
 *	of buf is undefined.
 */
inline void Wad_file::read_u8 (u8& buf) const
{
  buf = getc (fp);

  if (feof (fp) || ferror (fp))
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
  }
}


/*
 *	Wad_file::read_i16 - read a little-endian 16-bit signed integer
 *
 *	If an error occurs, set the error flag and the return
 *	value is undefined.
 */
inline i16 Wad_file::read_i16 () const
{
  const size_t nbytes = 2;
  u8 buf[nbytes];

  if (fread (buf, 1, nbytes, fp) != nbytes)
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
    return EOF;  // Whatever
  }
  return buf[0] | buf[1] << 8;
}


/*
 *	Wad_file::read_i16 - read a little-endian 16-bit signed integer
 *
 *	The value read is stored in *buf. If an error occurs,
 *	set the error flag and the contents of *buf is undefined.
 */
inline void Wad_file::read_i16 (i16 *buf) const
{
  *buf = getc (fp) | (getc (fp) << 8);

  if (feof (fp) || ferror (fp))
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
  }
}


/*
 *	Wad_file::read_i32 - read little-endian 32-bit signed integers
 *
 *	Read <count> little-endian 32-bit signed integers from
 *	wad file <wadfile> into *buf. If an error occurs, set
 *	error_ and the contents of *buf is undefined.
 */
inline void Wad_file::read_i32 (i32 *buf, long count) const
{
  while (count-- > 0)
  {
    *buf++ =    getc (fp)
       | (      getc (fp) << 8)
       | ((i32) getc (fp) << 16)
       | ((i32) getc (fp) << 24);
  }

  if (feof (fp) || ferror (fp))
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
  }
}


/*
 *	Wad_file::read_bytes - read bytes from a wad file
 *
 *	Read <count> bytes and store them into buffer <buf>.
 *	<count> is _not_ limited to size_t. If an I/O error
 *	occurs or EOF is reached before the requested number of
 *	bytes is read, set the error flag.
 */
inline void Wad_file::read_bytes (void *buf, long count) const
{
  long bytes_read;

  bytes_read = read_vbytes (buf, count);
  if (bytes_read != count)
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
  }
}


/*
 *	Wad_file::what - what a wad contains
 *
 *	Written for the sake of the "w" command. Return the
 *	name of the first lump in the wad, which gives an idea
 *	of what it contains. The string is *not* NUL-terminated.
 */
inline const char *Wad_file::what () const
{
  if (directory == 0)
    return "(nodir)";
  if (dirsize < 1)
    return "(empty)";
  return directory[0].name;
}


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
