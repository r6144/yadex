/*
 *	highlt.h
 *	AYM 1998-09-20
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


#include "edwidget.h"
#include "objid.h"


class highlight_c : public edwidget_c
   {
   public :
      highlight_c (void);
      void set (Objid& obj) { this->obj = obj; }

      /* Methods declared in edwidget */
      void unset () { obj.nil (); }
      void draw          ();
      void undraw        ();
      int can_undraw () { return 1; }  // I have the ability to undraw myself.
      int need_to_clear () { return 0; }  // I know how to undraw myself.
      void clear () { obj_disp.nil (); }

   private :
      Objid obj;	// The object we should highlight
      Objid obj_disp;	// The object that is really highlighted
   };

