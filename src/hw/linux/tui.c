/*
 * BlueParrot XCP
 *
 * (C) 2007-2020 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#include <ctype.h>
#include <string.h>

#include <ncurses.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_tui.h"


/*
 * Local Constants.
 *
 */


/*
 * Local Types.
 *
 */


/*
 * Local Function Prototypes.
 *
 */
static WINDOW *create_newwin(int height, int width, int starty, int startx);
static void destroy_win(WINDOW *local_win);
static void centered_text(WINDOW * win, int row, char const * text, int attrs);
static WINDOW * centered_window(int height, int width);

/*
 * Local Constants.
 *
 */
static const char TITLE[] = "Blueparrot XCP";

/*
 * Local Variables.
 *
 */

static void centered_text(WINDOW * win, int row, char const * text, int attrs)
{
    int maxX, maxY;

    getmaxyx(win, maxY, maxX);
    XCP_UNREFERENCED_PARAMETER(maxY);
    attron(attrs);
    mvprintw(row, (maxX - strlen(text)) / 2, text);
    attroff(attrs);

}


static WINDOW * centered_window(int height, int width)
{
    return create_newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);
}

static WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0); /* 0, 0 gives default characters
                              * for the vertical and horizontal
                              * * lines */
    wrefresh(local_win); /* Show that box */
    return local_win;
}

static void destroy_win(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}

void XcpTui_Init(void)
{
    bool ext;
    char buf[128];
    WINDOW *my_win;
    int startx, starty, width, height;

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    start_color(); /* Start color */
    init_pair(1, COLOR_BLUE, COLOR_BLACK);

#if defined(SOCKET_CAN)
    ext = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER);
    sprintf(buf, "XCPonCAN listening on 0x%04x [%s]\n",
        XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), ext ? "EXT" : "STD"
    );
#elif defined(ETHER)
    sprintf(buf, "XCPonEth -- Listening on port %d / %s [%s]",
            Xcp_Options.port, Xcp_Options.tcp ? "TCP" : "UDP",Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
#endif

    centered_text(stdscr, 2, TITLE, COLOR_PAIR(1) | A_BOLD);
    centered_text(stdscr, 4, buf, A_DIM);



    height = 10;
    width = 30;
    starty = (LINES - height) / 2; /* Calculating for a center placement */
    startx = (COLS - width) / 2; /* of the window */
    refresh();
    my_win = create_newwin(height, width, starty, startx);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    wbkgd(my_win, 2);
    wrefresh(my_win);
}

void XcpTui_Deinit(void)
{
    endwin();
}


void * XcpTui_MainFunction(void * param)
{
    int ch;

    XCP_UNREFERENCED_PARAMETER(param);

    while (XCP_TRUE) {
        ch = getch();
        if (tolower(ch) == 'q') {
            break;
        } else {
#if 0
            mvprintw(20, 10, "The pressed key is ");
            attron(A_BOLD);
            mvprintw(20, 30, "%c", ch);
            attroff(A_BOLD);
#endif
            refresh();
        }
    }
    return NULL;
}

void XcpHw_ErrorMsg(char * const function, int errorCode)
{
    fprintf(stderr, "[%s] failed with: [%d] %s", function, errorCode, strerror(errorCode));
}

