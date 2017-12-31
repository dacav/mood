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

#include <libmoodgoph/response.h>

static void check_response_list();

int main(int argc, char** argv)
{
    check_response_list();
    return 0;
}

static void check_response_list()
{
    moodgoph_response_t response = moodgoph_response_new();

    struct moodgoph_response_list_item item = {
        .type = MOODGOPH_RESPONSE_FILE,
        .display_string = "display",
        .selector = "selector",
        .hostname = "localhost",
        .port = 70,
    };

    for (int i = 0; i < 36; i ++) {
        moodgoph_response_list_append(response, &item);
    }

    moodgoph_response_delete(response);
}
