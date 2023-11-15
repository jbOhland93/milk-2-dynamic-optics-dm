#include "../headers/UserInputHandler.h"

#include <ncurses.h>
#include <sstream>
#include <iterator>

using namespace std;

vector<userCmd> UserInputHandler::cmdList = {
	userCmd::CMD_HELP,
    userCmd::CMD_ARM,
	userCmd::CMD_UNARM,
    userCmd::CMD_RELAX,
	userCmd::CMD_QUIT,
};

map<string,userCmd> UserInputHandler::cmdStrings = {
	{"help", userCmd::CMD_HELP},
    {"arm", userCmd::CMD_ARM},
    {"unarm", userCmd::CMD_UNARM},
    {"relax", userCmd::CMD_RELAX},
	{"quit", userCmd::CMD_QUIT},
};

map<userCmd,string> UserInputHandler::cmdHelp = {
	{userCmd::CMD_HELP, "Shows a list of available commands"},
    {userCmd::CMD_ARM, "Enables DM control via ISIO stream (default)"},
    {userCmd::CMD_UNARM, "Disables DM control via ISIO stream"},
    {userCmd::CMD_RELAX, "Starts the DM relaxation routine"},
	{userCmd::CMD_QUIT, "Quits the programm"},
};

UserInputHandler::UserInputHandler(bool test)
{
}

void UserInputHandler::core()
{
    initscr();
    wmove(stdscr, 0, 0);
    addstr("=== Welcome to the CLI of the milk-2-dynamic-optics-dm application. ===\n");
    addstr("\nEnter any string and hit enter to display the response.\n");
    addstr("Enter \"quit\" to exit the CLI.\n");
    addstr("Enter \"help\" for a list of comand options.\n\n");
    mPrintareaStart = getcury(stdscr);

    mvprintw(LINES-2,0,mPrompt);

    while (mRunning) {
        waitOnNextCmd();

        char str[1024];
        getstr(str);
		handleInput(str);
    }
    endwin();
}

void UserInputHandler::waitOnNextCmd()
{
	wmove(stdscr, LINES-2, 0);
	clrtoeol();
    addstr(mPrompt);
}

void UserInputHandler::handleInput(char* input)
{
    istringstream iss(input);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
    if (tokens.size() == 0)
        return; // Empty command string - perhaps user pressed enter.
    string cmd = tokens.at(0);

	if (cmdStrings.find(cmd) == cmdStrings.end()) {
		handleUnknownCmd(tokens.at(0));
	} else {
		switch(cmdStrings[cmd]) {
		case userCmd::CMD_HELP:
			execCmdHelp();
			break;
        case userCmd::CMD_ARM:
			execCmdArmUnarm(true);
			break;
        case userCmd::CMD_UNARM:
			execCmdArmUnarm(false);
			break;
        case userCmd::CMD_RELAX:
            execCmdRelax();
            break;
		case userCmd::CMD_QUIT:
			mRunning = false;
			break;
		default:
			mvprintw(LINES-1, 0, "Command found but no switch case implemented: ");
			clrtoeol();
			addstr(cmd.c_str());
		}
    }
}

void UserInputHandler::handleUnknownCmd(string cmd)
{
    clearResponseLine();
	addstr(mAnswerUnknown);
	addstr(cmd.c_str());
}

void UserInputHandler::execCmdHelp()
{
    int descrCol = 32;

    clearPrintArea();
    addstr("The following commands are available:\n");

	map<string, userCmd>::iterator it;
	for (it = cmdStrings.begin(); it != cmdStrings.end(); it++)
	{
        addstr(it->first.c_str());
        wmove(stdscr, getcury(stdscr), descrCol);
        addstr(cmdHelp[it->second].c_str());
        addstr("\n");
	}
    clearResponseLine();
}

void UserInputHandler::execCmdArmUnarm(bool start)
{
    clearResponseLine();
    if (start)
    {
        if (true)
            mvprintw(LINES-1, 0, "DM is already armed, nothing to be done.");
        else
        {
            int w,h;
            getmaxyx(stdscr, h, w);
            
            WINDOW* win = newwin(0,0,w,h);
            overwrite(stdscr, win);

            // ARM DM
            overwrite(win, stdscr);
            addstr("DM armed for ISIO input.");
        }
    }
    else
        if (true)
        {
            // UNARM DM
            mvprintw(LINES-1, 0, "DM unarmed and idle.");
        }
        else
        {
            addstr("DM is already unarmed, nothing to be done.");
        }
}

void UserInputHandler::execCmdRelax()
{
    clearResponseLine();
    if (true)
        mvprintw(LINES-1, 0, "DM is armed for ISIO control. Unarm before relaxing.");
    else
    {
        mvprintw(LINES-1, 0, "Relaxation routine running...");
        // RELAX DM
        mvprintw(LINES-1, 0, "Relaxation routine finished!");
    }
}

void UserInputHandler::clearPrintArea()
{
    wmove(stdscr, mPrintareaStart, 0);
    int curY = mPrintareaStart;
    while (curY < LINES-2) {
        clrtoeol();
        curY++;
        wmove(stdscr, curY, 0);
    }
    wmove(stdscr, mPrintareaStart, 0);
}

void UserInputHandler::clearResponseLine()
{
    wmove(stdscr, LINES-1, 0);
    clrtoeol();
}