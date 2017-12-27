/*  This file is part of Mood.
 *
 *  Mood is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  Mood is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mood.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libmoodgoph/request.h>
#include <libmoodgoph/node_filesystem.h>

#include <stdio.h>

int main(int argc, char** argv)
{
    char buffer[] = "/goat/race";
    moodgoph_request_t request = moodgoph_request_new(buffer, sizeof(buffer));

    moodgoph_node_t node = moodgoph_node_filesystem_new("tmp", "/tmp/");
    moodgoph_node_serve(node, request);
    moodgoph_node_delete(node);

    moodgoph_request_delete(request);
}
