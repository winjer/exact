/* $Id: match.h,v 1.3 2003/01/24 15:32:24 doug Exp $
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

#include <sys/types.h>
#include <regex.h>

#define MATCH_LOGIN_USERNAME_MAX	255
#define MATCH_LOGIN_HOSTNAME_MAX	255

typedef struct {
	char	username[MATCH_LOGIN_USERNAME_MAX];
	char	hostname[MATCH_LOGIN_HOSTNAME_MAX];
} match_login;

int match_init();
match_login *match_line(char *buff);
