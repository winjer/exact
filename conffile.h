/* $Id: conffile.h,v 1.6 2003/02/19 20:27:15 doug Exp $
 * 
 * This file is part of EXACT.
 *
 * EXACT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * EXACT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with EXACT; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

void conffile_setname(char *name);
char *conffile_name();
void conffile_read();
char *conffile_param(char *name);
int conffile_param_int(char *name);
void conffile_check();
void conffile_reload(int);
