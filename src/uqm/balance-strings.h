/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This file contains definitions about balance-specific strings.
 */

#ifndef BALANCE_STRINGS_H
#define BALANCE_STRINGS_H

#include "libs/strlib.h"

#define BALANCE_NETMELEE_STRING_COUNT 1

enum
{
	BALANCE_NETMELEE_STRING_BASE = 0
};

#define BALANCE_STRING(i) ((UNICODE *)GetStringAddress (SetAbsStringTableIndex (balance_strings, (i))))

extern STRING balance_strings;

#endif /* BALANCE_STRINGS_H */
