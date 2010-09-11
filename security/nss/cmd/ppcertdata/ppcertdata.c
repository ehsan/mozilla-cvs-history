/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the CertData.txt review helper program.
 *
 * The Initial Developer of the Original Code is
 * Nelson Bolyard
 * Portions created by the Initial Developer are Copyright (C) 2009-2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

unsigned char  binary_line[64 * 1024];

int
main(int argc, const char ** argv)
{
    int            skip_count = 0;
    int            bytes_read;
    char           line[133];

    if (argc > 1) {
    	skip_count = atoi(argv[1]);
    }
    if (argc > 2 || skip_count < 0) {
        printf("Usage: %s [ skip_columns ] \n", argv[0]);
	return 1;
    }

    while (fgets(line, 132, stdin) && (bytes_read = strlen(line)) > 0 ) {
	int    bytes_written;
	char * found;
	char * in          = line       + skip_count; 
	int    left        = bytes_read - skip_count;
	int    is_cert;
	int    is_serial;
	int    is_name;
	int    use_pp      = 0;
	int    out = 0;

	line[bytes_read] = 0;
	if (bytes_read <= skip_count) 
	    continue;
	fwrite(in, 1, left, stdout);
	found = strstr(in, "MULTILINE_OCTAL");
	if (!found) 
	    continue;
	fflush(stdout);

	is_cert   = (NULL != strstr(in, "CKA_VALUE"));
	is_serial = (NULL != strstr(in, "CKA_SERIAL_NUMBER"));
	is_name   = (NULL != strstr(in, "CKA_ISSUER")) ||
		    (NULL != strstr(in, "CKA_SUBJECT"));
	while (fgets(line, 132, stdin) && 
	       (bytes_read = strlen(line)) > 0 ) {
	    in   = line       + skip_count; 
	    left = bytes_read - skip_count;

	    if ((left >= 3) && !strncmp(in, "END", 3))
		break;
	    while (left >= 4) {
		if (in[0] == '\\'  && isdigit(in[1]) && 
		    isdigit(in[2]) && isdigit(in[3])) {
		    left -= 4;
		    binary_line[out++] = ((in[1] - '0') << 6) |
					 ((in[2] - '0') << 3) | 
					  (in[3] - '0');
		    in += 4;
		} else 
		    break;
	    }
	}
	if (out) {
	    FILE * mypipe;
	    int    off = 0;

	    mypipe = fopen("/tmp/deleteme", "wb");
	    if (!mypipe) {
	        fputs("Cannot create /tmp/deleteme\n", stderr);
		break;
	    }
	    do {
		bytes_written = fwrite(binary_line + off, 1, out, mypipe);
	        if (bytes_written <= 0)
		    break;
	        off += bytes_written;
		out -= bytes_written;
	    } while (out > 0);
	    fclose(mypipe);
	}
	if (is_cert)
	    system("pp -t certificate -i /tmp/deleteme");
	else if (is_name)
	    system("pp -t name -i /tmp/deleteme 2>> /dev/null");
	else if (is_serial)
	    system("pp -t serial -i /tmp/deleteme 2>> /dev/null");
	else
	    system("od -t x1 /tmp/deleteme 2>> /dev/null");
    }
    return 0;
}
